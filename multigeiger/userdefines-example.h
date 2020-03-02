// User / hardware specific definitions - do NOT edit THIS (example) file.
//
// To use them please copy this file to userdefines.h (do not add that file to version control)
// and then edit userdefines.h as appropriate for your use case / hardware.


// CPU values (board types, DO NOT CHANGE):
#define WIFI 0  // Heltec Wifi Kit 32
#define LORA 1  // Heltec Wifi Lora 32 (V2)
#define STICK 2  // Heltec Wireless Stick (has LoRa on board)

// your CPU (board type)
// if you use platformio, CPU is defined in platformio.ini and you don't need to change the definition of CPU.
// Arduino IDE users *must* define CPU here.
#ifndef CPU
#define CPU WIFI
#endif

// TUBE_TYPE values (predefined at sensor.community, DO NOT CHANGE):
#define TUBE_UNKNOWN 0 // this can be used for experimenting with other GM tubes and has a 0 CPM to uSv/h conversion factor.
#define SBM20 1
#define SBM19 2
#define Si22G 3

// your Geiger-Mueller counter tube:
#define TUBE_TYPE Si22G

// DEFAULT_LOG_LEVEL values (DO NOT CHANGE)
#include "log.h"

// your log level:
#define DEFAULT_LOG_LEVEL INFO

// SERIAL_DEBUG values (DO NOT CHANGE)
#include "log_data.h"

// your serial logging style:
#define SERIAL_DEBUG Serial_Logging

// Server transmission debugging:
// if set to 1, print debug info on serial (USB) interface while sending to servers (madavi or sensor.community)
#define DEBUG_SERVER_SEND 0

// Time to try to connect to saved WiFi [sec]
#define CONNECT_TIMEOUT 30

// Time for configuration via local access point [sec]
#define WAIT_4_CONFIG 180
