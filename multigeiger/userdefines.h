//====================================================================================================================================
// ***************************************
// User changable parameters:
// ***************************************
//====================================================================================================================================

// ** Please uncomment one and only one of following CPU-Defines
#define CPU WIFI
//#define CPU LORA
//#define CPU STICK

// ** select (uncomment) one counter tube
#define ROHRNAME SBM20
//#define ROHRNAME SBM19
//#define ROHRNAME Si22G

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
#define SPEAKER_TICK 0
// white led on board blinks with ervery tick
#define LED_TICK  1
// Display
#define SHOW_DISPLAY 0
// Start sound
#define PLAY_SOUND 0

// Send to servers:
// Madavi to see values in real time
#define SEND2MADAVI 1
// Luftdaten should always be 1 -> standard server (for not LoRa devices)
#define SEND2LUFTDATEN 0
// For LoRa-Devices, send to TTN (if this is set to 1, sending to
// Madavi and Luftdaten should be deactivated !!)
#define SEND2LORA 0

// Print debug-Info while sending to servers
#define DEBUG_SERVER_SEND 0


// *********************************************************************************
// END of user changable parameters.  Do not edit beyond this point! 
// *********************************************************************************
//====================================================================================================================================
