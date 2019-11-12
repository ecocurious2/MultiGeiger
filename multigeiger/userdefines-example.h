//====================================================================================================================================
// User / hardware specific definitions - do NOT edit THIS (example) file.
//
// To use them please copy this file to userdefines.h (do not add that file to version control)
// and then edit userdefines.h as appropriate for your use case / hardware.
//====================================================================================================================================

// ** Please uncomment one and only one of following CPU-Defines
#define CPU WIFI
//#define CPU STICK

// ** select (uncomment) one counter tube
//#define TUBE_TYPE SBM20
#define TUBE_TYPE SBM19
//#define TUBE_TYPE Si22G

// ** select (uncomment) debug level output to serial io (USB)
//#define   SERIAL_DEBUG Serial_None            // No Serial Printout
//#define   SERIAL_DEBUG Serial_Debug           // Only debug and error output will be printed via RS232(USB)
#define   SERIAL_DEBUG Serial_Logging         // Log measurement as table via RS232(USB)
//#define   SERIAL_DEBUG Serial_One_Minute_Log  // One Minute logging will be printed via RS232(USB)
//#define   SERIAL_DEBUG Serial_Statistics_Log  // Lists time between two events in us via RS232(USB)

// Time to try to connect to  saved WLAN [sec]
#define CONNECT_TIMEOUT 30
// Time for configuration via local access point [sec]
#define WAIT_4_CONFIG 180

// Speaker Ticks  1-> on,  0-> off
#define SPEAKER_TICK 1
// white led on board blinks with ervery tick
#define LED_TICK  1
// Display
#define SHOW_DISPLAY 1
// Start sound
#define PLAY_SOUND 0

// Send to servers:
// Madavi to see values in real time
#define SEND2MADAVI 1
// Luftdaten should always be 1 -> standard server (for not LoRa devices)
#define SEND2LUFTDATEN 1
// For LoRa-Devices, send to TTN (if this is set to 1, sending to
// Madavi and Luftdaten should be deactivated !!)
#define SEND2LORA 1

// Print debug-Info while sending to servers
#define DEBUG_SERVER_SEND 1


// *********************************************************************************
// END of user changable parameters.  Do not edit beyond this point!
// *********************************************************************************
//====================================================================================================================================
