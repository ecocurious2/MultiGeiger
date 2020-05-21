// Code related to the transmission of the measured data via Bluetooth Low Energy,
// uses GATT Heart Rate Measurement Service for notifications with CPM update
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

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BLE_SERVICE_HEART_RATE    BLEUUID((uint16_t)0x180D)  // 16 bit UUID of Heart Rate Service
#define BLE_CHAR_HR_MEASUREMENT   BLEUUID((uint16_t)0x2A37)  // 16 bit UUID of Heart Rate Measurement Characteristic
#define BLE_CHAR_HR_POSITION      BLEUUID((uint16_t)0x2A38)  // 16 bit UUID of Heart Rate Sensor Position Characteristic
#define BLE_CHAR_HR_CONTROLPOINT  BLEUUID((uint16_t)0x2A39)  // 16 bit UUID of Heart Rate Control Point Characteristic
#define BLE_DESCR_UUID            BLEUUID((uint16_t)0x2901)  // 16 bit UUID of BLE Descriptor

BLECharacteristic bleCharHRM(BLE_CHAR_HR_MEASUREMENT, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bleDescriptorHRM(BLE_DESCR_UUID);
BLECharacteristic bleCharHRCP(BLE_CHAR_HR_CONTROLPOINT, BLECharacteristic::PROPERTY_WRITE);
BLEDescriptor bleDescriptorHRCP(BLE_DESCR_UUID);
BLECharacteristic bleCharHRPOS(BLE_CHAR_HR_POSITION, BLECharacteristic::PROPERTY_READ);
BLEDescriptor bleDescriptorHRPOS(BLE_DESCR_UUID);

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

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) {
    char remoteAddress[18];
    sprintf(
      remoteAddress,
      "%02X:%02X:%02X:%02X:%02X:%02X",
      param->connect.remote_bda[0],
      param->connect.remote_bda[1],
      param->connect.remote_bda[2],
      param->connect.remote_bda[3],
      param->connect.remote_bda[4],
      param->connect.remote_bda[5]
    );
    log(INFO, "BLE device connected, remote MAC: %s", remoteAddress);
    device_connected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    device_connected = false;
    log(INFO, "BLE device disconnected");
  }
};

// Callback allowing for Control Point Characteristic to reset "energy expenditure" (packet counter)
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
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
  bleCharHRM.setValue(txBuffer_HRM, 5);
  bleCharHRM.notify();
}

void setup_ble(char *device_name, bool ble_on) {
  if (!ble_on) {
    set_status(STATUS_BT, ST_BT_OFF);
    ble_enabled = false;
    return;
  }
  ble_enabled = true;

  set_status(STATUS_BT, ST_BT_INIT);
  BLEDevice::init(device_name);

  BLEServer *bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new MyServerCallbacks());

  BLEService *bleService = bleServer->createService(BLE_SERVICE_HEART_RATE);

  bleService->addCharacteristic(&bleCharHRM);
  bleDescriptorHRM.setValue("Radiation rate CPM");
  bleCharHRM.addDescriptor(&bleDescriptorHRM);
  bleCharHRM.addDescriptor(new BLE2902());  // required for notification management of the service

  bleService->addCharacteristic(&bleCharHRCP);
  bleDescriptorHRCP.setValue("0x01 for Energy Exp. (packet counter) reset");
  bleCharHRCP.addDescriptor(&bleDescriptorHRCP);
  bleCharHRCP.setCallbacks(new MyCharacteristicCallbacks());

  bleService->addCharacteristic(&bleCharHRPOS);
  bleDescriptorHRPOS.setValue("Geiger Mueller Tube Type");
  bleCharHRPOS.addDescriptor(&bleDescriptorHRPOS);
  bleCharHRPOS.setValue(txBuffer_HRPOS, 1);

  bleServer->getAdvertising()->addServiceUUID(BLE_SERVICE_HEART_RATE);

  bleService->start();
  bleServer->getAdvertising()->start();

  set_status(STATUS_BT, ST_BT_CONNECTABLE);
  log(INFO, "BLE service advertising started, device name: %s, MAC: %s", device_name, BLEDevice::getAddress().toString().c_str());
}

void disable_ble(void) {
  ble_enabled = false;
  set_status(STATUS_BT, ST_BT_OFF);
}