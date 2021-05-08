.. _changelog:

Changelog
=========

V1.16.0-dev (not released yet)
------------------------------

New features:

* None

Fixes:

* initialize sound and LED to "off", #398

Other changes:

* explicitly turn ticking off before melody/init
* cosmetic: don't touch LED in pauses between melody notes
* upgrade to Adafruit BME680 Library >=2.0.0
* add root cert "Amazon CA 1"
* docs:

  - fix typo in sensor.community domain
  - fixed switch numbering, #349
  - add supported counter tubes
  - create multigeiger-bill-of-material.txt
  - developer docs: bump master branch version to -dev after release
  - update translations

V1.15.0 2021-03-21
------------------

New features:

* add bluetooth (BLE) support, #78

Fixes:

* improve LoRaWAN stability (work around LMIC bug #677, add LMIC polling
  from loop()), #373
* do async NTP/clock setup, #316
* speaker: init "duty_mode" member in MCPWM config
* avoid using IotWebConf 3.0.0 for now, #357, PR #370

Other changes:

* patch: restore partition scheme menu for arduino-esp32 1.0.5
* move CI from travis CI to github workflow
* start screen cleanups, #335
* code / naming style fixes
* remove dates in file names, commit relevant versions to git
* add drill files, #354
* docs:

  - use transifex / sphinx / readthedocs.org for translatons (en/de for now)
  - document docs/translation workflow in development docs
  - added assembly and deployment guide
  - document esp32 board buttons, #129
  - document dip switch usage, #128
  - move README-{de,en}.* contents into the .rst docs
  - BLE usage documentation update with some images, #338
  - added links to map, ecocurious, assembly room
  - markup, rendering, spelling fixes, cleanups
  - fix unclear version / date in Aufbauanleitung, #110
  - moved links to docs -> resources, #223
  - add xkcd about radiation doses to FAQ, #310

V1.14.0 2020-05-16
------------------

New features:

* implement status line on OLED display (see docs), #257
* also support BME680 sensor for temperature, humidity, pressure
* display time up to 60s / 60m / 24h / 99d, then roll over
* speaker/LED: timer-driven sequencer, hw PWM sound, #35
* TLS support

  - add clock module, use NTP to set the clock
  - use persistent per-server HTTPClient instances
  - use connection: keep-alive for web requests
  - add https capability (can be used for sending data)
  - note: transmission to sensor.community and madavi is still using http!

Fixes:

* fixed GM pulse debouncing, #248
* pulse counting: deal with microseconds uint32 overflow, #273
* check WiFi status before trying to transmit
* fix race condition, #286

Other changes:

* dip switches: only read once at boot time, #207
* new font (u8x8 uses 8px width anyway)
* slow down main loop
* toilet -> custom server, add comments about toilet usage, #214
* refactor/simplify pulse counting ISR, bookkeeping in main loop, #220
* refactor big main loop into smaller functions with local bookkeeping.
* misc. other code cleanups
* loraWan: removed unused/not needed code, #212, #234
* removed meeting notes, #294
* docs:

  - README improvements (board name, flash size, partition scheme, passwords,
    LoRa)
  - update development/release docs (create/test binaries, IDE settings, ...)

V1.13.0 2020-04-14
------------------

* auto-detect hardware (STICK vs. WIFI) by hardware pin
* use config web page for more values (userdefines.h has the defaults), #140
* try both adresses of BME280
* LoRa payload changes, e.g. to fulfill 'TTN Fair Access Policy'
* send additional data to servers
* send to MADAVI in one single request both geiger and thp data
* new logging with DEFAULT_LOG_LEVEL configuration
* integrated travis-ci:

  - for compile checks (platformio, wifi and stick build)
  - for style checks (using the "astyle" CPP checker)
* source: modularization, cleanups, less globals, ...
  (quite huge internal changes, please help testing!)
* building:

  - platformio-based build: suppress lmic_project_config.h usage
  - arduino-ide-based build: you still need to edit that file
* use bump2version tool for project version bumps, #169
* docs:

  - added upgrade hints for 1.13 in README on github
  - https://multigeiger.readthedocs.io/ == the beginning of
    new (sphinx / reST-markup based) online docs, #163
  - add a basic, short README in English (also for online docs)
  - include infos about project name, #121
  - moved changelog.md to docs/source/changes.rst
  - updated/fixed development docs, #46
  - update docs about new 5V power supply / cabling, #122
  - description of LoRa Payload updated
  - other docs improvements / fixes

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

