// To fit in 16bit for lora version number we have
// some limits here: max. VERSION_MAJOR = 15, max. VERSION_PATCH = 15.
#define VERSION_MAJOR 1
#define VERSION_MINOR 11
#define VERSION_PATCH 0
// Date is in format "YYY-MM-DD".
#define VERSION_DATE "2019-12-16"

/**** Revision history:
* Version date        author
*
* V1.11 2019-12-16  rxf
*   - defaults in userdefines-example.h changed
*   - Software version for LoRa now 2 Bytes
*   - Display start screen for Wireless stick fixed
*   - changed to semantic versioning
*
* V1.10 2019-12-13  rxf/tw
*   - conversion factor for Si22G tube fixed
*   - char variables changed to int
*   - isr routines shielded with portMUX
*   - debug serial out formatting improved
*   - sequence of counting and dispaying and hv charging improved
*   - speaker and led tick fixed
*   - many calls to millis() consolidated
* 
* V1.9 2019-11-05 rxf
*   - structure for different counter tubes
*   - LoRa payload changed again
*   - hv pulse at least every second
*   - calculate and display cpm value every 10 seconds
*   - fixed div by 0 if there's no tube
*
* V1.8 2019-11-04 rxf
*   - merged pull requests from TW:
*     indentation/spacing, refactor OLED functions, fix conversion factor
*   - MEASUREMENT_INTERVAL 150sec
*   - changed LoRa payload
*
* V1.7 2019-10-21 rxf
*   - PINs rearranged, so we can use new Wifi-Kit-32 and WiFi Stick Light
*   - Hardware layout V1.4 and up
*   - use switch for speaker tick and display off
*
* V1.6 2019-09-13 rxf
*   - rearrangement of files
*   - test dip switch
*   - Hardware layout V1.3 and lower - OLD Wifi-Kit-32 !
*
* V1.5 2019-09-11 rxf
*   - added BME280 via I2C
*   - Display adapted for Wireless Stick
*   - added LoRa
*
* V1.4 2019-09-03 rxf
*   - default config, measurement interval 10min
*
* V1.3 2019_09_03 rxf
*   - Building MAC changed again. Now it's identical to 'Feinstaubsensor'.
*
* V1.2 2019_09_02 rxf
*   - sending to madavi corrected
*
* V1.1 2019_09_01 rxf
*   - building the SSID from the MAC corrected: first 3 Bytes of MAC build SSID
*   - IoTWebConfig: setter for thingName added; lib moved into local source path
*   - LoRa autodetection removed
*
* V1.0 2019_08_19 rxf
*   - added detection of LoRa device
*   - WiFiManager to enter WLAN data and other configs
*   - send to luftdaten.info every 2.5 min
*
* V0.3 2019_05_12 Juergen Boehringer
*   - added bug fix for the "Double-Trigger-Problem". This was caused
*     by the rising edge falsly triggering an other pulse recording.
*     The Problem is that there is no Schmitt-Trigger available in the controller.
*   - simplified serial printing modes
*   - made seconds in Display as inverse to be able to separate it from minutes
*   - cleaned up the code
*   - Fixed overflow bug in Minute-Count+
*
* V0.2 2019_04_26 Juergen Boehringer
*   - added 1 Minute RS232 (USB) logging mode
*
* V0.1 2019_03_25 Juergen Boehringer
*   - first version for ESP32 board
*/
