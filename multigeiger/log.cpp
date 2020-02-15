// low level log() call - outputs to serial/usb

#include "log.h"

#include <Arduino.h>

// this is to differentiate our output from other esp32 output (e.g. wifi messages)
#define LOG_PREFIX "GEIGER: "
#define LOG_PREFIX_LEN 8  // 8 chars, without the terminating \0

static int log_level = NOLOG;  // messages at level >= log_level will be output

void log(int level, const char *format, ...) {
  if (level < log_level)
    return;

  va_list args;
  va_start(args, format);
  char buf[vsnprintf(NULL, 0, format, args) + 1 + LOG_PREFIX_LEN];
  strcpy(buf, LOG_PREFIX);
  vsprintf(buf + LOG_PREFIX_LEN, format, args);
  va_end(args);
  Serial.println(buf);
}

void setup_log(int level) {
  Serial.begin(115200);
  while (!Serial) {};
  log(NOLOG, "Logging initialized at level %d.", level);  // this will always be output
  log_level = level;
}
