// low level log() call - outputs to serial/usb

#ifndef _LOG_H_
#define _LOG_H_

// log levels
#define DEBUG 0
#define INFO 1
#define WARNING 2
#define ERROR 3
#define CRITICAL 4
#define NOLOG 999  // only to set log_level, so log() never creates output

void log(int level, const char *format, ...);
void setup_log(int level);

#endif // _LOG_H_
