// low level log() call - outputs to serial/usb

#ifndef _LOG_H_
#define _LOG_H_

// log levels
// reverted the numbers to meet logic in log_data.h (see #define Serial_None 0 ...)
// DEBUG is max info, reducing output step by step till NOLOG
/* TR, 20.04.2022 :
1) there's no CRITICAL and no WARNING . remove both and use numbers for Serial_Print_Mode?
and NOLOG really logs nothing then ...
2) Serial_Print_Mode should be included in this sequence (--> settable through the loglevel!).
   It's not clear to me, why Serial_Print_Mode is set fix at compilation time ?!?
   Org:
#define DEBUG    5
#define INFO     4
#define WARNING  3  // there is nothing with WARNING ...
#define ERROR    2
#define CRITICAL 1  // there is nothing with CRITICAL ...
#define NOLOG    0  // minimum info, only display alarms
*/

#define DEBUG     5
#define INFO      4
#define MED_INFO  3
#define MIN_INFO  2
#define ERROR     1
#define NOLOG     0


void log(int level, const char *format, ...);
void setup_log(int level);
int getloglevel(void);
void setloglevel(int level);
#endif // _LOG_H_
