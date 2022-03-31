// low level log() call - outputs to serial/usb

#ifndef _LOG_H_
#define _LOG_H_

// log levels
// reverted the numbers to meet logic in log_data.h (see #define Serial_None ...)
// DEBUG is max info, reducing step by step till NOLOG
#define DEBUG 5
#define INFO 4
#define WARNING 3
#define ERROR 2
#define CRITICAL 1
#define NOLOG 0  // only to set log_level, so log() never creates output

void log(int level, const char *format, ...);
void setup_log(int level);
int getloglevel(void);
void setloglevel(int level);
#endif // _LOG_H_
