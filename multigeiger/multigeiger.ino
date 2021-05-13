// Project: Simple Multi-Geiger
// (c) 2019,2020 by the authors, see AUTHORS file in toplevel directory.
// Licensed under the GPL v3 (or later), see LICENSE file in toplevel directory.

#include <Arduino.h>

#include "version.h"
#include "log.h"
#include "log_data.h"
#include "userdefines.h"
#include "thp_sensor.h"
#include "tube.h"
#include "switches.h"
#include "speaker.h"
#include "webconf.h"
#include "display.h"
#include "transmission.h"
#include "ble.h"
#include "chkhardware.h"
#include "clock.h"
#include "status_led.h"

// Measurement interval (default 2.5min) [sec]
#define MEASUREMENT_INTERVAL 150

// Max time the greeting display will be on. [msec]
#define AFTERSTART 5000

// In which intervals the OLED display is updated. [msec]
#define DISPLAYREFRESH 10000

// Minimum amount of GM pulses required to early-update the display.
#define MINCOUNTS 100

// Target loop duration [ms]
// slow down the arduino main loop so it spins about once per LOOP_DURATION -
#define LOOP_DURATION 1000

// DIP switches
static Switches switches;


void setup() {
  bool isLoraBoard = init_hwtest();
  setup_log(DEFAULT_LOG_LEVEL);
  setup_status_LED();
  setup_display(isLoraBoard);
  setup_switches(isLoraBoard);
  switches = read_switches();  // only read DIP switches once at boot time
  setup_thp_sensor();
  setup_webconf(isLoraBoard);
  setup_speaker(playSound, ledTick && switches.led_on, speakerTick && switches.speaker_on);
  setup_transmission(VERSION_STR, ssid, isLoraBoard);
  setup_ble(ssid, sendToBle && switches.ble_on);
  setup_log_data(SERIAL_DEBUG);
  setup_tube();
}

void setup_ntp(int wifi_status) {
  static bool clock_configured = false;
  if (clock_configured)
    return;

  // this is called from loop() because we do not want to block in setup().
  // it might either take a while until WiFi is ready or it might even never
  // be ready, e.g. in LoRa-based deployments or on the road using BLE.

  // a bug in arduino-esp32 1.0.4 crashes the esp32 if the wifi is not
  // configured yet and one tries to configure for NTP:
  if (wifi_status != ST_WIFI_CONNECTED)
    return;

  setup_clock(0);  // 0 == do NTP!
  // please note that time is not necessarily NTP-correct below here.
  // it might take some minutes until we have the correct time.
  // if we do not have a connection to NTP servers, it will just count up from 1970.

  clock_configured = true;
}

int update_wifi_status(void) {
  int st;
  switch (iotWebConf.getState()) {
  case IOTWEBCONF_STATE_CONNECTING:
    st = ST_WIFI_CONNECTING;
    break;
  case IOTWEBCONF_STATE_ONLINE:
    st = ST_WIFI_CONNECTED;
    break;
  case IOTWEBCONF_STATE_AP_MODE:
    st = ST_WIFI_AP;
    break;
  default:
    st = ST_WIFI_OFF;
    break;
  }
  set_status(STATUS_WIFI, st);
  return st;
}

int update_ble_status(void) {  // currently no error detection
  int st;
  if (sendToBle && switches.ble_on)
    st = is_ble_connected() ? ST_BLE_CONNECTED : ST_BLE_CONNECTABLE;
  else
    st = ST_BLE_OFF;
  set_status(STATUS_BLE, st);
  return st;
}

void publish(unsigned long current_ms, unsigned long current_counts, unsigned long gm_count_timestamp, unsigned long current_hv_pulses) {
  static unsigned long last_timestamp = millis();
  static unsigned long last_counts = 0;
  static unsigned long last_hv_pulses = 0;
  static unsigned long last_count_timestamp = 0;
  static unsigned int accumulated_GMC_counts = 0;
  static unsigned long accumulated_time = 0;
  static float accumulated_Count_Rate = 0.0, accumulated_Dose_Rate = 0.0;

  if (((current_counts - last_counts) >= MINCOUNTS) || ((current_ms - last_timestamp) >= DISPLAYREFRESH)) {
    if ((gm_count_timestamp == 0) && (last_count_timestamp == 0)) {
      // seems like there was no GM pulse yet and everything is still in initial state.
      // get out of here, we can't do anything useful now.
      return;
    }
    last_timestamp = current_ms;
    int hv_pulses = current_hv_pulses - last_hv_pulses;
    last_hv_pulses = current_hv_pulses;
    int counts = current_counts - last_counts;
    last_counts = current_counts;
    int dt = gm_count_timestamp - last_count_timestamp;
    last_count_timestamp = gm_count_timestamp;

    accumulated_time += dt;
    accumulated_GMC_counts = current_counts;

    float GMC_factor_uSvph = tubes[TUBE_TYPE].cps_to_uSvph;

    // calculate the current count rate and dose rate
    float Count_Rate = (dt != 0) ? (float)counts * 1000.0 / (float)dt : 0.0;
    float Dose_Rate = Count_Rate * GMC_factor_uSvph;

    // indicate status on RGB LED (if any)
    indicate(Dose_Rate, I_TEST);

    // calculate the count rate and dose rate over the complete time from start
    accumulated_Count_Rate = (accumulated_time != 0) ? (float)accumulated_GMC_counts * 1000.0 / (float)accumulated_time : 0.0;
    accumulated_Dose_Rate = accumulated_Count_Rate * GMC_factor_uSvph;

    // ... and update the data on display, notify via BLE
    update_bledata((unsigned int)(Count_Rate * 60));
    display_GMC(((int)accumulated_time / 1000), (int)(accumulated_Dose_Rate * 1000), (int)(Count_Rate * 60),
                (showDisplay && switches.display_on));

    if (Serial_Print_Mode == Serial_Logging) {
      log_data(counts, dt, Count_Rate, Dose_Rate, hv_pulses,
               accumulated_GMC_counts, accumulated_time, accumulated_Count_Rate, accumulated_Dose_Rate);
    }
  } else {
    // If there were no pulses after AFTERSTART msecs after boot, clear display anyway and show 0 counts.
    static unsigned long boot_timestamp = millis();
    static unsigned long afterStartTime = AFTERSTART;
    if (afterStartTime && ((current_ms - boot_timestamp) >= afterStartTime)) {
      afterStartTime = 0;
      update_bledata(0);
      display_GMC(0, 0, 0, (showDisplay && switches.display_on));
    }
  }
}

void one_minute_log(unsigned long current_ms, unsigned long current_counts) {
  static unsigned int last_counts = 0;
  static unsigned long last_timestamp = millis();
  int dt = current_ms - last_timestamp;
  if (dt >= 60000) {
    unsigned long counts = current_counts - last_counts;
    unsigned int count_rate = (counts * 60000) / dt;
    if (((((counts * 60000) % dt) * 2) / dt) >= 1) {
      count_rate++;  // Rounding + 0.5
    }
    log_data_one_minute((current_ms / 1000), count_rate, counts);
    last_timestamp = current_ms;
    last_counts = current_counts;
  }
}

void statistics_log(unsigned long current_counts, unsigned int time_between) {
  static unsigned long last_counts = 0;
  if (current_counts != last_counts) {
    log_data_statistics(time_between);
    last_counts = current_counts;
  }
}

void read_THP(unsigned long current_ms,
              bool *have_thp, float *temperature, float *humidity, float *pressure) {
  static unsigned long last_timestamp = 0;
  if ((current_ms - last_timestamp) >= (MEASUREMENT_INTERVAL * 1000)) {
    last_timestamp = current_ms;
    *have_thp = read_thp_sensor(temperature, humidity, pressure);
    if (*have_thp)
      log(INFO, "Measured THP: T=%.2f H=%.f P=%.f", *temperature, *humidity, *pressure);
  }
}

void transmit(unsigned long current_ms, unsigned long current_counts, unsigned long gm_count_timestamp, unsigned long current_hv_pulses,
              bool have_thp, float temperature, float humidity, float pressure, int wifi_status) {
  static unsigned long last_counts = 0;
  static unsigned long last_hv_pulses = 0;
  static unsigned long last_timestamp = millis();
  static unsigned long last_count_timestamp = 0;
  if ((current_ms - last_timestamp) >= (MEASUREMENT_INTERVAL * 1000)) {
    if ((gm_count_timestamp == 0) && (last_count_timestamp == 0)) {
      // seems like there was no GM pulse yet and everything is still in initial state.
      // get out of here, we can't do anything useful now.
      return;
    }
    last_timestamp = current_ms;
    unsigned long counts = current_counts - last_counts;
    last_counts = current_counts;
    int dt = gm_count_timestamp - last_count_timestamp;
    last_count_timestamp = gm_count_timestamp;
    unsigned int current_cpm;
    current_cpm = (dt != 0) ? (int)(counts * 60000 / dt) : 0;

    int hv_pulses = current_hv_pulses - last_hv_pulses;
    last_hv_pulses = current_hv_pulses;

    log(DEBUG, "current time: %s", utctime());
    log(DEBUG, "Measured GM: cpm= %d HV=%d", current_cpm, hv_pulses);

    transmit_data(tubes[TUBE_TYPE].type, tubes[TUBE_TYPE].nbr, dt, hv_pulses, counts, current_cpm,
                  have_thp, temperature, humidity, pressure, wifi_status);
  }
}

void loop() {
  static bool hv_error = false;  // true means a HV capacitor charging issue

  static bool have_thp = false;
  static float temperature = 0.0, humidity = 0.0, pressure = 0.0;

  unsigned long current_ms = millis();  // to save multiple calls to millis()

  // this is the always increasing HV pulse master counter.
  // main program: all other hv pulse counter values shall be derived from it.
  static unsigned long hv_pulses = 0;

  // this is the always increasing geiger mueller master counter.
  // main program: all other counter values for misc. purposes shall be derived from it.
  // ISR code: counters and other values there should be only short-lived and only be
  //           used to update values in main program.
  static unsigned long gm_counts = 0;

  // this is the master timestamp of the last geiger mueller event [ms]
  // main program: all other timestamp bookkeeping values shall be derived from it.
  // ISR code: only one timestamp shall be kept/updated there with the only purpose
  //           of being used to update the master timestamp.
  static unsigned long gm_count_timestamp;

  // time between last 2 geiger mueller events [us]
  unsigned int gm_count_time_between;

  read_GMC(&gm_counts, &gm_count_timestamp, &gm_count_time_between);

  read_THP(current_ms, &have_thp, &temperature, &humidity, &pressure);

  read_hv(&hv_error, &hv_pulses);
  set_status(STATUS_HV, hv_error ? ST_HV_ERROR : ST_HV_OK);

  int wifi_status = update_wifi_status();
  setup_ntp(wifi_status);

  update_ble_status();

  // do any other periodic updates for uplinks
  poll_transmission();

  publish(current_ms, gm_counts, gm_count_timestamp, hv_pulses);

  if (Serial_Print_Mode == Serial_One_Minute_Log)
    one_minute_log(current_ms, gm_counts);

  if (Serial_Print_Mode == Serial_Statistics_Log)
    statistics_log(gm_counts, gm_count_time_between);

  transmit(current_ms, gm_counts, gm_count_timestamp, hv_pulses, have_thp, temperature, humidity, pressure, wifi_status);

  long loop_duration;
  loop_duration = millis() - current_ms;
  iotWebConf.delay((loop_duration < LOOP_DURATION) ? (LOOP_DURATION - loop_duration) : 0);
}
