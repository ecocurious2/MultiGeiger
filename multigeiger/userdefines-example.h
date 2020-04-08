// User / hardware specific definitions - do NOT edit THIS (example) file.
//
// To use them please copy this file to userdefines.h (do not add that file to version control)
// and then edit userdefines.h as appropriate for your use case / hardware.


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
// if set to true, print debug info on serial (USB) interface while sending to servers (madavi or sensor.community)
#define DEBUG_SERVER_SEND true

// Time to try to connect to saved WiFi [sec]
#define CONNECT_TIMEOUT 30

// Time for configuration via local access point [sec]
#define WAIT_4_CONFIG 180

// Speaker Ticks with every pulse?
#define SPEAKER_TICK true

// White LED on uC board flashing with every pulse?
#define LED_TICK true

// Enable display?
#define SHOW_DISPLAY true

// Play a start sound at boot/reboot time?
#define PLAY_SOUND true

// Send to servers:
// Send data to Madavi server?
// Madavi should be used to see values in real time.
#define SEND2MADAVI true

// Send data to sensor.community server?
// Should always be true so that the data is archived there. Standard server for devices without LoRa.
#define SEND2SENSORCOMMUNITY true

// Send data via LoRa to TTN?
// Only for devices with LoRa, automatically deactivated for devices without LoRa.
// If this is set to true, sending to Madavi and sensor.community should be deactivated!
// Note: The TTN configuration needs to be done in lorawan.cpp (starting at line 65).
#define SEND2LORA false

