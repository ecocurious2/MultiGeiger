#ifndef CLOCK_H
#define CLOCK_H

#define NTP_SRV_1 "pool.ntp.org"
#define NTP_SRV_2 "time.nist.gov"

#define TZ_OFFSET 0  // timezone offset from UTC [s]
#define DST_OFFSET 0  // DST offset from NONDST [s]

#define NTP_MAX_RETRY 300  // how many NTP_SLEEPs to wait before giving up
#define NTP_SLEEP 1000  // ms

#define TIME_MIN_VALID ((2019 - 1970) * 365 * 24 * 3600)  // ~ Dec 2018

// timestamp == 0 -> NTP wanted, otherwise just set the clock.
void setup_clock(time_t timestamp);

// wait until ntp has synced (returned true) or timeout (returned false).
bool wait_ntp(void);

// check if current time looks ok (year >= 2020)
bool time_valid(void);

// return a iso-8601-like utc timestamp
char *utctime(void);

#endif

