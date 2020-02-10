.. _changelog:

Changelog
=========

V1.13.0 (not released yet)
--------------------------

* ...


V1.12.0 2020-01-18
------------------

* simple OTA (Over-The-Air) updates via web browser based upload, #120
* use less charge pulses in loop() for timing, more in setup() for initial charging, #134
* output error msg on Serial if HV charging fails
* tag log output with "GEIGER: ", #85
* add TUBE_UNKNOWN 0 to have a specific value for experimenting
* adapted platformio.ini to pull all dependencies
* send CR and LF on serial
* changed default tube from sbm-20 to si22g
* semantic versioning, version numbers now like x.y.z
* changed building of revString and lora_version
* docs updated / improved
* explain SBM-19/SBM-20 conversion factor
* removed IotWebconf bundled&patched code, used as a lib now.

V1.11.1 2019-12-16 rxf
----------------------

* change luftdaten.info to sensor.community

V1.11.0 2019-12-16 rxf
----------------------

* defaults in userdefines-example.h changed
* Software version for LoRa now 2 Bytes
* Display start screen for Wireless stick fixed
* changed to semantic versioning

V1.10 2019-12-13
----------------

* conversion factor for Si22G tube fixed
* char variables changed to int
* isr routines shielded with portMUX
* debug serial out formatting improved
* sequence of counting and dispaying and hv charging improved
* speaker and led tick fixed
* many calls to millis() consolidated

V1.9 2019-11-12
---------------

* structure for different counter tubes
* LoRa payload changed again
* hv pulse every second
* calculate and display cpm value every 10 seconds
* fixed div by 0 if there's no tube
* Readme corrected

V1.8 2019-11-04
---------------

* indentation/spacing, refactor OLED functions, fix conversion factor
* MEASUREMENT_INTERVAL 150sec
* changed LoRa payload

V1.7 2019-10-21
---------------

* PINs rearranged, so we can use new Wifi-Kit-32 and WiFi Stick Light
* Hardware-Layout V1.4 and up
* use switch for speaker tick and display off

V1.61 2019-09-30
----------------

* default measuring interval is now 2.5min

V1.6 2019-09-13
---------------

* some rearrangement of files
* userdefine.h for user changable #defines
* test with dip-switch (needs pullup resistors!)
* Hardware layout V1.3 and lower - OLD Wifi-Kit-32!

V1.5 2019-09-11
---------------

* added BME280 (uses same I2C as display)
* Support for display on Wireless Stick
* For LoRa-Devices added LoRa functionality

V1.4 2019-09-03
---------------

* default configuration with measurement interval of 10min

V1.3 2019-09-03
---------------

* building of ESP-ID out of MAC address is now identical to 'Feinstaubsensor'

V1.2 2019-09-02
---------------

* sending to madavi corrected

V1.1 2019-09-01
---------------

* Library IoTWebConfig changed -> function 'setThingName' added
* Move this (IoTWebConfig) library to source path
* building the SSID from the MAC corrected: first 3 Bytes of MAC build SSID
* LoRa autodetection removed

V1.0 2019-08-19 rxf
-------------------

* added detection of LoRa device
* WiFiManager to enter WLAN data and other configs
* send to luftdaten.info every 2.5 min

V0.3 2019-05-12 jb
------------------

* added bug fix for the "Double-Trigger-Problem". This was caused
  by the rising edge falsly triggering an other pulse recording.
  The Problem is that there is no Schmitt-Trigger available in the controller.
* simplified serial printing modes
* made seconds in Display as inverse to be able to separate it from minutes
* cleaned up the code
* Fixed overflow bug in Minute-Count+

V0.2 2019-04-26 jb
------------------

* added 1 Minute RS232 (USB) logging mode

V0.1 2019-03-25 jb
------------------

* first version for ESP32 board
