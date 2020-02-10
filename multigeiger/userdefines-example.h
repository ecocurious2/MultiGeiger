//====================================================================================================================================
// User / hardware specific definitions - do NOT edit THIS (example) file.
//
// To use them please copy this file to userdefines.h (do not add that file to version control)
// and then edit userdefines.h as appropriate for your use case / hardware.
//====================================================================================================================================

// If you use platformio, CPU is defined in platformio.ini and
// you don't need to change the definition of CPU.
// Arduino IDE users *must* define CPU here and uncomment one and only one of following CPU-Defines.
#ifndef CPU
#define CPU WIFI    // is used for WiFi Kit 32
//#define CPU STICK     // is used for Wireless stick
#endif

// ** select (uncomment) exactly one Geiger-Mueller counter tube:
// #define TUBE_TYPE TUBE_UNKNOWN  // this can be used for experimenting with other GM tubes and has a 0 CPM to uSv/h conversion factor.
// #define TUBE_TYPE SBM20
// #define TUBE_TYPE SBM19
#define TUBE_TYPE Si22G

// ** select (uncomment) exactly one kind of debug level output to serial io (USB)
// #define   SERIAL_DEBUG Serial_None            // No Serial Printout
// #define   SERIAL_DEBUG Serial_Debug           // Only debug and error output
#define   SERIAL_DEBUG Serial_Logging           // Log measurements as table
// #define   SERIAL_DEBUG Serial_One_Minute_Log  // One Minute logging
// #define   SERIAL_DEBUG Serial_Statistics_Log  // Lists time [us] between two events

// ** Special debug info:
// if set to 1, print debug info on serial (USB) interface while sending to servers (madavi or sensor.community)
#define DEBUG_SERVER_SEND 0

// Time to try to connect to saved WiFi [sec]
#define CONNECT_TIMEOUT 30
// Time for configuration via local access point [sec]
#define WAIT_4_CONFIG 180

// Speaker Ticks with every pulse?  1-> on,  0-> off
#define SPEAKER_TICK 1
// White LED on uC board flashing with every pulse?
#define LED_TICK  1
// Enable display?
#define SHOW_DISPLAY 1
// Play a start sound at boot/reboot time?
#define PLAY_SOUND 1

// Send to servers:
// Send data to Madavi server?
// Madavi should be used to see values in real time.
#define SEND2MADAVI 1
// Send data to sensor.community server?
// Should always be 1 so that the data is archived there. Standard server for devices without LoRa.
#define SEND2SENSORCOMMUNITY 1
// Send data via LoRa to TTN?
// Only for devices with LoRa, automatically deactivated for devices without LoRa.
// If this is set to 1, sending to Madavi and sensor.community should be deactivated!
// Note: The TTN configuration needs to be done in lorawan.cpp (starting at line 65).
#define SEND2LORA 0

// *********************************************************************************
// END of user changeable parameters.  Do not edit beyond this point!
// *********************************************************************************
//====================================================================================================================================
