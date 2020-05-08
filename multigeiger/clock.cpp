#include <Arduino.h>
#include <sys/time.h>

#include "clock.h"


void config_time(time_t timestamp) {
  if (timestamp > 0) {
    // a specific timestamp was given, e.g. for testing purposes, set time:
    struct timeval tv = {timestamp, 0};
    settimeofday(&tv, nullptr);
  } else {
    // timestamp == 0 means we shall use NTP to get the time:
    configTime(TZ_OFFSET, DST_OFFSET, NTP_SRV_1, NTP_SRV_2);
    // please note that this just sets up NTP. the actual time sync might take some minutes...
  }
}


bool time_valid(void) {
  // does the current time look valid (not 1970)?
  time_t now;
  now = time(nullptr);
  return (now >= TIME_MIN_VALID);
}


bool wait_ntp(void) {
  // wait for ntp time sync (until the time looks valid)
  int retry = 0;
  while (!time_valid()) {
    if (retry++ >= NTP_MAX_RETRY)
      break;
    delay(NTP_SLEEP);
  }
  return (retry < NTP_MAX_RETRY);  // success: true, failure: false
}


char *utctime(void) {
  // return a pointer to a timestamp string like 2019-12-31T23:59:59
  time_t t;
  struct tm *ti;
  static char buffer[20];

  time(&t);
  ti = gmtime(&t);
  strftime(buffer, 20, "%Y-%m-%dT%H:%M:%S", ti);
  return buffer;
}


void setup_clock(time_t timestamp) {
  config_time(timestamp);
}

