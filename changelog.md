# Change log  
This is the change log for the multigeiger project.

### Version 1.8  2019-11-04  
 * merged pull requests from TW:  
    indentation/spacing, refactor OLED functions, fix conversion factor
 * MEASUREMENT_INTERVAL 150sec
 * changed LoRa payload

### Version 1.7 2019-10-21
 * PINs rearranged, so we can use new Wifi-Kit-32 and
 * WiFi Stick Light
 * Hardware-Layout V1.4 and up
 * use switch for speaker tick and display off

### Version 1.61 2019-09-30
 * default measuring interval is now 2.5min 
  
### Version 1.6 2019-09-13 
 * some rearrangement of files
 * userdefine.h for user changable #defines
 * test with dip-switch (needs pullups !)
 
### Version 1.5 2019-09-11 
* added BME280 ( uses I2C of display )
* Support for display on Wireless Stick
* For LoRa-Devices added LoRa functionality      

### Version 1.4 2019-09-03
 * default configuration with measurement interval of 10min

### Version 1.3 2019-09-03
 * building of ESP-ID out of MAC address is now identical to 'Feinstaubsensor'

### Version 1.2 2019-09-02
 * error in sendig to madavi corrected

### Version 1.1 2019-09-01
 * Library IoTWebConfig changed -> function 'setThingName' added
 * Move this (IoTWebConfig) library to source path
