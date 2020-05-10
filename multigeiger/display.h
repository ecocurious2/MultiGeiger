// OLED display related code

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void setup_display(bool loraHardware);
void DisplayGMC(int TimeSec, int RadNSvph, int CPM, bool use_display);
void clearDisplayLine(int line);
void displayStatusLine(String txt);

// supported status indexes and values:

// these are used for all subsystems
#define ST_NODISPLAY 0
#define ST_OFF 0
#define ST_OK 1
#define ST_ERROR 2

#define STATUS_WIFI 0
#define ST_WIFI_OFF 0
#define ST_WIFI_CONNECTED 1
#define ST_WIFI_ERROR 2
#define ST_WIFI_CONNECTING 3
#define ST_WIFI_AP 4

#define STATUS_SCOMM 1
#define ST_SCOMM_OFF 0
#define ST_SCOMM_IDLE 1
#define ST_SCOMM_ERROR 2
#define ST_SCOMM_SENDING 3

#define STATUS_MADAVI 2
#define ST_MADAVI_OFF 0
#define ST_MADAVI_IDLE 1
#define ST_MADAVI_ERROR 2
#define ST_MADAVI_SENDING 3

#define STATUS_TTN 3
#define ST_TTN_OFF 0
#define ST_TTN_IDLE 1
#define ST_TTN_ERROR 2
#define ST_TTN_SENDING 3

#define STATUS_BT 4
#define ST_BT_OFF 0
#define ST_BT_CONNECTED 1
#define ST_BT_ERROR 2
#define ST_BT_CONNECTABLE 3

// status index 5 is still free

// status index 6 is still free

#define STATUS_HV 7
#define ST_HV_OK 1
#define ST_HV_ERROR 2

#define STATUS_MAX 8

void set_status(int index, int value);
void displayStatus(void);

#endif // _DISPLAY_H_
