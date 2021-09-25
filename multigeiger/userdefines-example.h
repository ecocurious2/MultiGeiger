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

// Send data via BLE?
// Device provides "Heart Rate Service" (0x180D) and these characteristics.
// 0x2A37: Heart Rate Measurement
// --> sends current CPM as shown on display + rolling packet counter as energy expenditure (roll-over @0xFF).
// 0x2A38: Heart Rate Sensor Position --> sends TUBE_TYPE
// 0x2A39: Heart Rate Control Point --> allows to reset "energy expenditure", as required by service definition
#define SEND2BLE false

// Play an alarm sound when radiation level is too high?
// Activates when either accumulated dose rate reaches the set threshold (see below)
// or when the current dose rate is higher than the accumulated dose rate by the set factor (see below).
// ! Requires a valid tube type to be set in order to calculate dose rate.
#define LOCAL_ALARM_SOUND false

// Accumulated dose rate threshold to trigger the local alarm
// The accumulated dose rate is the overall average since the last start of MultiGeiger.
// Default value: 0.500 µSv/h
// ! Requires a valid tube type to be set in order to calculate dose rate.
#define LOCAL_ALARM_THRESHOLD 0.500  // µSv/h

// Factor of current dose rate vs. accumulated dose rate to trigger the local alarm
// Default value: 3
// Example: accumulated dose rate from days of operation is at 0.1 µSv/h.
// If current dose rate rises to (default) 3 times the accumulated dose rate, i.e. 0.3 µSv/h, trigger the local alarm.
// ! Requires a valid tube type to be set in order to calculate dose rate.
#define LOCAL_ALARM_FACTOR 3  // current / accumulated dose rate

// Send MultiGeiger info and alerts to Telegram Messenger chat
// REQUIRES WIFI CONNECTION!
// Update via Telegram Messenger every N sensor.community messages (default 150 s / 2.5 min)
// Set to 0 to disable normal data transfer
// 24 = 1/hour, 576 = 1/day, 4032 = 1/week
#define TELEGRAM_TICKER_EVERY 0

// To communicate with the Telegram Messenger on your phone you need to create a bot
// Starting point: https://core.telegram.org/bots
// You will get a Bot token, please insert this here within ""
// Form: "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
// Default: Empty string ""
#define TELEGRAM_BOT_TOKEN ""  // "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

// Important: Start your bot channel with the command "/start" and receive the Chat ID
// In order to get MultiGeiger messages to your specific chat, please provide this Chat ID here, again within ""
// Form: "1234567890"
// Default: Empty string ""
#define TELEGRAM_CHAT_ID ""  // "1234567890"

// Send local alerts via Telegram Messenger
// See above for more info on local alerts
#define TELEGRAM_ALERT true
