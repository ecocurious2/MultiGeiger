// Code related to the transmission of the measured data via Bluetooth Low Energy
// Uses GATT Heart Rate Measurement Service for notifications with CPM update

#ifndef _BLE_H_
#define _BLE_H_

void setup_ble(char *device_name, bool ble_enabled);
void update_bledata(unsigned int cpm);
bool is_ble_connected();

#endif // _BLE_H_
