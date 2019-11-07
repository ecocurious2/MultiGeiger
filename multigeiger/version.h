// Revision history:

const char* revString = "V1.8_2019-11-04";

/*
* Version date        author
*
* V1.8    2019-11-04  rxf 
*   - merged pull requests from TW:
*     indentation/spacing, refactor OLED functions, fix conversion factor
*   - MEASUREMENT_INTERVAL 150sec
*   - changed LoRa payload
*
* V1.7  2019-10.-21   rxf
*   - PINs rearranged, so we can use new Wifi-Kit-32 and
*   - WiFi Stick Light
*   - Hardware-Layout V1.4 and up
*   - use switch for speaker tick and display off
*
* V1.6  2019-09-13  rxf
*   - rearrangement of files
*   - test dip-switch
*   - Hardware-Layout V1.3 and lower - OLD Wifi-Kit-32 !
*
* V1.5  2019-09-11  rxf               
*   - added BME280 via I2C
*   - Display adapted for Wireless Stick
*   - added Lora
*
* V1.4  2019-09-03  rxf              
*   - default config, measurment interval 10min
*
* V1.3  2019_09_03  rxf             
*   - Building MAC changed again. Now its identical to 'Feinstaubsensor'
*
* V1.2  2019_09_02  rxf             
*   - sending to madavi corrected
*
* V1.1  2019_09_01  rxf              
*   - build SSID out of MAC corrected: first 3 Byte of MAC build SSID
*   - IoTWebConfig: setter for thingName added; lib moved into local source path
*   - LoRa autodetection removed
*
* V1.0  2019_08_19  rxf             
*   - added detection of LoRa-Device
*   - WiFiManager to enter WLAN data and other configs
*   - send to luftdaten.info every 2.5 min
*
* V0.3  2019_05_12  Juergen Boehringer      
*   - Added Bugfix for the "Double-Trigger-Problem". This was caused
*     by the rising edge falsly triggering an other pulse recording.
*     The Problem is that there is no Schmitt-Trigger available in the Controller
*   - simplified serial printing modes
*   - made seconds in Display as inverse to be able to seperate it from minutes
*   - cleaned up the code
*   - Fixed Overflow-Bug in Minute-Count+
*
* V0.2  2019_04_26  Juergen Boehringer      
*   - added 1 Minute RS232-Logging-Mode
*
* V0.1  2019_03_25" ^Juergen Boehringer      
*   - first versionvfor ESP32-Board
*/