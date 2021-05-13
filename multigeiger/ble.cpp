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

void update_bledata(unsigned int cpm) {
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

  NimBLEService *bleService = bleServer->createService(BLE_SERVICE_HEART_RATE);

// NimBLECharacteristic bleCharHRM(BLE_CHAR_HR_MEASUREMENT, NIMBLE_PROPERTY::NOTIFY);
  NimBLECharacteristic *bleCharHRM = bleService->createCharacteristic(BLE_CHAR_HR_MEASUREMENT, NIMBLE_PROPERTY::NOTIFY);
// NimBLEDescriptor bleDescriptorHRM(BLE_DESCR_UUID);
  NimBLEDescriptor *bleDescriptorHRM = bleCharHRM->createDescriptor("Radiation rate CPM", NIMBLE_PROPERTY::READ);
//  bleCharHRM.addDescriptor(new BLE2902());  // required for notification management of the service

// NimBLECharacteristic bleCharHRCP(BLE_CHAR_HR_CONTROLPOINT, NIMBLE_PROPERTY::WRITE);
  NimBLECharacteristic *bleCharHRCP = bleService->createCharacteristic(BLE_CHAR_HR_CONTROLPOINT, NIMBLE_PROPERTY::WRITE);
// NimBLEDescriptor bleDescriptorHRCP(BLE_DESCR_UUID);
  NimBLEDescriptor *bleDescriptorHRCP = bleCharHRM->createDescriptor("0x01 for Energy Exp. (packet counter) reset", NIMBLE_PROPERTY::READ);
// NimBLECharacteristic bleCharHRPOS(BLE_CHAR_HR_POSITION, NIMBLE_PROPERTY::READ);
  NimBLECharacteristic *bleCharHRPOS = bleService->createCharacteristic(BLE_CHAR_HR_POSITION, NIMBLE_PROPERTY::READ);
// NimBLEDescriptor bleDescriptorHRPOS(BLE_DESCR_UUID);
  NimBLEDescriptor *bleDescriptorHRPOS = bleCharHRM->createDescriptor("Geiger Mueller Tube Type", NIMBLE_PROPERTY::READ);
  bleCharHRCP->setCallbacks(new MyCharacteristicCallbacks());

  bleCharHRPOS->setValue(txBuffer_HRPOS, 1);

  bleServer->getAdvertising()->addServiceUUID(BLE_SERVICE_HEART_RATE);
  bleServer->getAdvertising()->setScanResponse(true);
  bleServer->getAdvertising()->setMinPreferred(0x06);
  bleServer->getAdvertising()->setMinPreferred(0x12);

  bleService->start();
  bleServer->getAdvertising()->start();

  set_status(STATUS_BLE, ST_BLE_CONNECTABLE);
  log(INFO, "BLE service advertising started, device name: %s, MAC: %s", device_name, BLEDevice::getAddress().toString().c_str());
}

void disable_ble(void) {
  ble_enabled = false;
  set_status(STATUS_BLE, ST_BLE_OFF);
}
