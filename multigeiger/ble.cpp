// Code related to the transmission of the measured data via Bluetooth Low Energy,
// uses GATT Heart Rate Measurement Service for notifications with CPM update.
//
// Heart Rate Measurement = Radiation CPM, Energy Expense = Rolling Packet Counter
//
// Based on Neil Kolban's example file: https://github.com/nkolban/ESP32_BLE_Arduino
// Based on Andreas Spiess' example file: https://github.com/SensorsIot/Bluetooth-BLE-on-Arduino-IDE/blob/master/Polar_H7_Sensor/Polar_H7_Sensor.ino

#include <Arduino.h>

#include "log.h"
#include "userdefines.h"
#include "utils.h"
#include "ble.h"
#include "display.h"

#include <NimBLEDevice.h>

static NimBLEServer *bleServer;

#define BLE_SERVICE_HEART_RATE    BLEUUID((uint16_t)0x180D)  // 16 bit UUID of Heart Rate Service
#define BLE_CHAR_HR_MEASUREMENT   BLEUUID((uint16_t)0x2A37)  // 16 bit UUID of Heart Rate Measurement Characteristic
#define BLE_CHAR_HR_POSITION      BLEUUID((uint16_t)0x2A38)  // 16 bit UUID of Heart Rate Sensor Position Characteristic
#define BLE_CHAR_HR_CONTROLPOINT  BLEUUID((uint16_t)0x2A39)  // 16 bit UUID of Heart Rate Control Point Characteristic

#define BLE_SERVICE_ENVIRONMENTAL BLEUUID((uint16_t)0x181A)  // 16 bit UUID of EnvironmentaL Service
#define BLE_CHAR_ENV_TEMPERATURE  BLEUUID((uint16_t)0x2A6E)  // 16 bit UUID of EnvironmentaL Temperature Characteristic (yields 16 bit little endian, 0.01 degC)
#define BLE_CHAR_ENV_HUMIDITY     BLEUUID((uint16_t)0x2A6F)  // 16 bit UUID of EnvironmentaL Humidity Characteristic (yields 16 bit little endian, 0.01 %)
#define BLE_CHAR_ENV_PRESSURE     BLEUUID((uint16_t)0x2A6D)  // 16 bit UUID of EnvironmentaL Air Pressure Characteristic (yields 32 bit little endian, 0.1 Pa)
#define BLE_CHAR_ENV_IAQ          BLEUUID(0x422302f1, 0x2342, 0x2342, 0x2342234223422342)  // 128 bit UUID of Environmental Indoor Air Quality (yields 16 bit little endian IAQ)
// Characteristic for IAQ from 2019 Chaos Communication Camp card10 https://firmware.card10.badge.events.ccc.de/bluetooth/ess.html

#define BLE_DESCR_UUID            BLEUUID((uint16_t)0x2901)  // 16 bit UUID of BLE Descriptor

static bool ble_enabled = false;
static bool device_connected = false;

uint8_t flags_HRS = 0b00001001; // bit 0 --> 1 = HR (cpm) as UINT16, bit 3 --> add UINT16 Energy Expended (rolling update counter)
uint8_t txBuffer_HRM[5]; // transmit buffer, byte[0] = flags, [1, 2] = cpm, [3, 4] = rolling update counter
uint8_t txBuffer_HRPOS[1] = {TUBE_TYPE};
unsigned int status_HRCP = 0;
unsigned int cpm_update_counter = 0;

bool is_ble_connected(void) {
  return ble_enabled && device_connected;
}

class MyServerCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) {
    log(INFO, "BLE device connected, remote MAC: %s", NimBLEAddress(desc->peer_ota_addr).toString().c_str());
    device_connected = true;
  };

  void onDisconnect(NimBLEServer *pServer) {
    device_connected = false;
    log(INFO, "BLE device disconnected");
  }
};

// Callback allowing for Control Point Characteristic to reset "energy expenditure" (packet counter)
class MyCharacteristicCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue().c_str();
    status_HRCP = (rxValue.length() > 0) ? (unsigned int)rxValue[0] : 0;
  }
};

void update_bledata(unsigned int cpm, float temperature, float humidity, float pressure, int iaq) {
  if (!ble_enabled)
    return;
  cpm_update_counter++;
  cpm_update_counter = cpm_update_counter & 0xFFFF;
  if (status_HRCP > 0) {
    log(DEBUG, "HR Control Point Value received: %d, resetting packet counter", status_HRCP);
    cpm_update_counter = 0;
    status_HRCP = 0;
  }
  txBuffer_HRM[0] = flags_HRS;
  txBuffer_HRM[1] = cpm & 0xFF;
  txBuffer_HRM[2] = (cpm >> 8) & 0xFF;
  txBuffer_HRM[3] = cpm_update_counter & 0xFF;
  txBuffer_HRM[4] = (cpm_update_counter >> 8) & 0xFF;
  if (bleServer->getConnectedCount()) {
    NimBLEService *bleSvc = bleServer->getServiceByUUID(BLE_SERVICE_HEART_RATE);
    if (bleSvc) {
      NimBLECharacteristic *bleChr = bleSvc->getCharacteristic(BLE_CHAR_HR_MEASUREMENT);
      if (bleChr) {
        bleChr->setValue(txBuffer_HRM, 5);
        bleChr->notify();
      }
    }
    NimBLEService *bleEnvSvc = bleServer->getServiceByUUID(BLE_SERVICE_ENVIRONMENTAL);
    if (bleEnvSvc) {
      uint8_t txBufferEnv[4];
      NimBLECharacteristic *bleEnvTChr = bleEnvSvc->getCharacteristic(BLE_CHAR_ENV_TEMPERATURE);
      if (bleEnvTChr) {
        bleEnvTChr->setValue<int>(int(temperature * 100));
        bleEnvTChr->notify();
      }
      NimBLECharacteristic *bleEnvHChr = bleEnvSvc->getCharacteristic(BLE_CHAR_ENV_HUMIDITY);
      if (bleEnvHChr) {
        bleEnvHChr->setValue<int>(int(humidity * 100));
        bleEnvHChr->notify();
      }
      NimBLECharacteristic *bleEnvPChr = bleEnvSvc->getCharacteristic(BLE_CHAR_ENV_PRESSURE);
      if (bleEnvPChr) {
        bleEnvPChr->setValue<int>(int(pressure * 10));
        bleEnvPChr->notify();
      }
      NimBLECharacteristic *bleEnvIAQChr = bleEnvSvc->getCharacteristic(BLE_CHAR_ENV_IAQ);
      if (bleEnvIAQChr) {
        txBufferEnv[0] = iaq & 0xFF;
        txBufferEnv[1] = (iaq >> 8) & 0xFF;
        bleEnvIAQChr->setValue(txBufferEnv, 2);
        bleEnvIAQChr->notify();
      }
    }
  }

}

void setup_ble(char *device_name, bool ble_on) {
  if (!ble_on) {
    set_status(STATUS_BLE, ST_BLE_OFF);
    ble_enabled = false;
    return;
  }
  ble_enabled = true;

  set_status(STATUS_BLE, ST_BLE_INIT);
  NimBLEDevice::init(device_name);

  bleServer = NimBLEDevice::createServer();
  bleServer->setCallbacks(new MyServerCallbacks());

  // Heart Rate Service for CPM
  NimBLEService *bleHRService = bleServer->createService(BLE_SERVICE_HEART_RATE);

  NimBLECharacteristic *bleCharHRM = bleHRService->createCharacteristic(BLE_CHAR_HR_MEASUREMENT, NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor *bleDescriptorHRM = bleCharHRM->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 20);
  bleDescriptorHRM->setValue("Radiation rate CPM");

  NimBLECharacteristic *bleCharHRCP = bleHRService->createCharacteristic(BLE_CHAR_HR_CONTROLPOINT, NIMBLE_PROPERTY::WRITE);
  NimBLEDescriptor *bleDescriptorHRCP = bleCharHRCP->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 50);
  bleDescriptorHRCP->setValue("0x01 for Energy Exp. (packet counter) reset");
  NimBLECharacteristic *bleCharHRPOS = bleHRService->createCharacteristic(BLE_CHAR_HR_POSITION, NIMBLE_PROPERTY::READ);
  NimBLEDescriptor *bleDescriptorHRPOS = bleCharHRPOS->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 30);
  bleDescriptorHRPOS->setValue("Geiger Mueller Tube Type");
  bleCharHRCP->setCallbacks(new MyCharacteristicCallbacks());

  bleCharHRPOS->setValue(txBuffer_HRPOS, 1);

  // Environmental Service for temperature, humidity, air pressure, IAQ indoor air quality
  NimBLEService *bleEnvService = bleServer->createService(BLE_SERVICE_ENVIRONMENTAL);

  NimBLECharacteristic *bleCharEnvT = bleEnvService->createCharacteristic(BLE_CHAR_ENV_TEMPERATURE, NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor *bleDescriptorEnvT = bleCharEnvT->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 30);
  bleDescriptorEnvT->setValue("Temperature (.01 Celsius)");
  NimBLECharacteristic *bleCharEnvH = bleEnvService->createCharacteristic(BLE_CHAR_ENV_HUMIDITY, NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor *bleDescriptorEnvH = bleCharEnvH->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 30);
  bleDescriptorEnvH->setValue("Rel. humidity (.01 %)");
  NimBLECharacteristic *bleCharEnvP = bleEnvService->createCharacteristic(BLE_CHAR_ENV_PRESSURE, NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor *bleDescriptorEnvP = bleCharEnvP->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 30);
  bleDescriptorEnvP->setValue("Air pressure (.1 Pa)");
  NimBLECharacteristic *bleCharEnvIAQ = bleEnvService->createCharacteristic(BLE_CHAR_ENV_IAQ, NIMBLE_PROPERTY::NOTIFY);
  NimBLEDescriptor *bleDescriptorEnvIAQ = bleCharEnvIAQ->createDescriptor(BLE_DESCR_UUID, NIMBLE_PROPERTY::READ, 50);
  bleDescriptorEnvIAQ->setValue("Indoor air quality (25 good .. 500 bad)");


  bleServer->getAdvertising()->addServiceUUID(BLE_SERVICE_HEART_RATE);
  bleServer->getAdvertising()->addServiceUUID(BLE_SERVICE_ENVIRONMENTAL);
  bleServer->getAdvertising()->setScanResponse(true);
  bleServer->getAdvertising()->setMinPreferred(0x06);
  bleServer->getAdvertising()->setMinPreferred(0x12);

  bleHRService->start();
  bleEnvService->start();
  bleServer->getAdvertising()->start();

  set_status(STATUS_BLE, ST_BLE_CONNECTABLE);
  log(INFO, "BLE service advertising started, device name: %s, MAC: %s", device_name, BLEDevice::getAddress().toString().c_str());
}

void disable_ble(void) {
  ble_enabled = false;
  set_status(STATUS_BLE, ST_BLE_OFF);
}
