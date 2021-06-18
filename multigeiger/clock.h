#ifndef CLOCK_H
#define CLOCK_H

#define NTP_SRV_1 "pool.ntp.org"
#define NTP_SRV_2 "time.nist.gov"

#define TZ_OFFSET 0  // timezone offset from UTC [s]
#define DST_OFFSET 0  // DST offset from NONDST [s]

// timestamp == 0 -> NTP wanted, otherwise just set the clock.
void setup_clock(time_t timestamp);

// return a iso-8601-like utc timestamp
char *utctime(void);

#endif

