// low level log() call - outputs to serial/usb

#include <Arduino.h>
#include "string.h"
#include "log.h"
#include "clock.h"
#include "utils.h"

// the GEIGER: prefix is is to easily differentiate our output from other esp32 output (e.g. wifi messages)
#define LOG_PREFIX_FORMAT "GEIGER: %s "
#define LOG_PREFIX_LEN (7+1+19+1)  // chars, without the terminating \0

int log_level = NOLOG;  // messages at level >= log_level will be output

void log(int level, const char *format, ...) {
  if (level > log_level)
    return;

  va_list args;
  va_start(args, format);
  char buf[vsnprintf(NULL, 0, format, args) + 1 + LOG_PREFIX_LEN];
  sprintf(buf, LOG_PREFIX_FORMAT, utctime());
  vsprintf(buf + LOG_PREFIX_LEN, format, args);
  va_end(args);
  Debug.println(buf);
}

void setup_log(int level) {
  Debug.begin(115200);		// Output to Serial at 115200 baud
  while (!Debug) {};
  log(NOLOG, "Logging initialized at level %d.", level);  // this will always be output
  log_level = level;
}
int getloglevel(void){
  return log_level;
}
void setloglevel(int level){
  log_level=level;
}
