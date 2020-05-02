// Project: Simple Multi-Geiger
// (c) 2019,2020 by the authors, see AUTHORS file in toplevel directory.
// Licensed under the GPL v3 (or later), see LICENSE file in toplevel directory.
//
// Description: With minimal external components you are able to build a Geiger Counter that:
//   - is precise
//   - cheap
//   - makes the typical tick sounds
//   - produces a listing via RS232 (via USB)
//   - is adaptable to your application
//
// Information about the new Heltec board ESP32 WIFI OLED:
// - how to get the device up and running:
//   - https://robotzero.one/heltec-wifi-kit-32/
// - driver for the USB=>UART-Chip CP2102:
//   - http://esp32.net/usb-uart/#SiLabs
// - infos from the Heltec, the board manufacturer:
//   - http://www.heltec.cn/project/wifi-kit-32/?lang=en
// - it is sold on ebay e.g. under the following names:
//   - "1X(ESP32 WIFI Bluetooth Entwicklungsboard OLED 0.96 "Display IOT Kit Modul GY"
// - there is also a variant with LoRaWAN:
//   - http://fambach.net/esp32-wifi-lora-433/
//   - https://www.hackerspace-ffm.de/wiki/index.php?title=Heltec_Wifi_LoRa_32
//

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
#include "chkhardware.h"

// Measurement interval (default 2.5min) [sec]
#define MEASUREMENT_INTERVAL 150

// MAX time to wait until connected. [msec]
// If there is still no connection after that time,
// measurements will start, but won't be sent to servers.
#define MAX_WAIT_TIME 300000

// Max time the greeting display will be on. [msec]
#define AFTERSTART 5000

// In which intervals the OLED display is updated. [msec]
#define DISPLAYREFRESH 10000

// Minimum amount of GM pulses required to early-update the display.
#define MINCOUNTS 100

// Target loop duration [ms]
// slow down the arduino main loop so it spins about once per LOOP_DURATION -
// this is more than enough, but still less than once per 1-2ms (without delay).
// it should not be too slow because the speaker/LED tick is done in main loop.
#define LOOP_DURATION 5

// DIP switches
static Switches switches;


void setup() {
  bool isLoraBoard = init_hwtest();
  setup_log(DEFAULT_LOG_LEVEL);
  setup_display(isLoraBoard);
  setup_speaker();
  setup_switches(isLoraBoard);
  switches = read_switches();  // only read DIP switches once at boot time
  setup_thp_sensor();
  setup_webconf(isLoraBoard);
  setup_transmission(VERSION_STR, ssid, isLoraBoard);

  if (playSound)
    play_start_sound();

  setup_log_data(SERIAL_DEBUG);
  setup_tube();
}

void loop() {
  static bool wifi_connected = false;
  unsigned long current_ms = millis();  // to save multiple calls to millis()
  static unsigned long boot_timestamp = millis();

  // this is the always increasing HV pulse master counter.
  // main program: all other hv pulse counter values shall be derived from it.
  static unsigned long HV_pulses_master = 0;

  // this is the always increasing geiger mueller master counter.
  // main program: all other counter values for misc. purposes shall be derived from it.
  // ISR code: counters and other values there should be only short-lived and only be
  //           used to update values in main program.
  static unsigned long GMC_counts_master = 0;

  // this is the master timestamp of the last geiger mueller event [ms]
  // main program: all other timestamp bookkeeping values shall be derived from it.
  // ISR code: only one timestamp shall be kept/updated there with the only purpose
  //           of being used to update the master timestamp.
  static unsigned long GMC_count_timestamp_master = 0;
  
  // time between last 2 geiger mueller events [us]
  unsigned int GMC_count_time_between;

  static unsigned int accumulated_GMC_counts = 0;
  static unsigned long accumulated_time = 0;

  static float Count_Rate = 0.0, Dose_Rate = 0.0;
  static float accumulated_Count_Rate = 0.0, accumulated_Dose_Rate = 0.0;

  bool update_display;

  read_GMC(&GMC_counts_master, &GMC_count_timestamp_master, &GMC_count_time_between);

  static unsigned long last_display_timestamp = millis();
  static unsigned long last_display_counts = 0;
  static unsigned long last_display_hv_pulses = 0;
  static unsigned long last_count_timestamp = millis();

  // Check if there are enough pulses detected or if enough time has elapsed.
  // If yes, then it is time to calculate the pulse rate, update the display and recharge the HV capacitor.
  update_display = ((GMC_counts_master - last_display_counts) >= MINCOUNTS) || ((current_ms - last_display_timestamp) >= DISPLAYREFRESH);

  // TODO: give GMC_counts_master instead of update_display to charge_hv (so it can compare against inner state and decide when to recharge)
  HV_pulses_master += charge_hv(update_display, current_ms);

  if (update_display) {
    last_display_timestamp = current_ms;
    int hv_pulses = HV_pulses_master - last_display_hv_pulses;
    last_display_hv_pulses = HV_pulses_master;
    int counts = GMC_counts_master - last_display_counts;
    last_display_counts = GMC_counts_master;
    int dt = GMC_count_timestamp_master - last_count_timestamp;
    last_count_timestamp = GMC_count_timestamp_master;

    accumulated_time += dt;
    accumulated_GMC_counts = GMC_counts_master;

    float GMC_factor_uSvph = tubes[TUBE_TYPE].cps_to_uSvph;

    // calculate the current count rate and dose rate
    Count_Rate = (dt != 0) ? (float)counts * 1000.0 / (float)dt : 0.0;
    Dose_Rate = Count_Rate * GMC_factor_uSvph;

    // calculate the count rate and dose rate over the complete time from start
    accumulated_Count_Rate = (accumulated_time != 0) ? (float)accumulated_GMC_counts * 1000.0 / (float)accumulated_time : 0.0;
    accumulated_Dose_Rate = accumulated_Count_Rate * GMC_factor_uSvph;

    // ... and display them.
    DisplayGMC(((int)accumulated_time / 1000), (int)(accumulated_Dose_Rate * 1000), (int)(Count_Rate * 60),
               (showDisplay && switches.display_on), wifi_connected);

    if (Serial_Print_Mode == Serial_Logging) {
      log_data(counts, dt, Count_Rate, Dose_Rate, hv_pulses,
               accumulated_GMC_counts, accumulated_time, accumulated_Count_Rate, accumulated_Dose_Rate);
    }
  }

  if (Serial_Print_Mode == Serial_One_Minute_Log) {
    static unsigned int last_minute_log_counts = 0;
    static unsigned long last_minute_log_timestamp = millis();
    int dt = current_ms - last_minute_log_timestamp;
    if (dt >= 60000) {
      unsigned long counts = GMC_counts_master - last_minute_log_counts;
      unsigned int count_rate = (counts * 60000) / dt;
      if (((((counts * 60000) % dt) * 2) / dt) >= 1) {
        count_rate++;  // Rounding + 0.5
      }
      log_data_one_minute((current_ms / 1000), count_rate, counts);
      last_minute_log_timestamp = current_ms;
      last_minute_log_counts = GMC_counts_master;
    }
  }

  if (Serial_Print_Mode == Serial_Statistics_Log) {
    static unsigned long last_statistics_log_counts = 0;
    if (GMC_counts_master != last_statistics_log_counts) {
      last_statistics_log_counts = GMC_counts_master;
      log_data_statistics(GMC_count_time_between);
    }
  }

  // If there were no pulses after AFTERSTART msecs after boot, clear display anyway and show 0 counts.
  static unsigned long afterStartTime = AFTERSTART;
  if (afterStartTime && ((current_ms - boot_timestamp) >= afterStartTime)) {
    afterStartTime = 0;
    DisplayGMC(((int)accumulated_time / 1000), (int)(accumulated_Dose_Rate * 1000), (int)(Count_Rate * 60),
               (showDisplay && switches.display_on), wifi_connected);
  }

  // Check, if we have to send to sensor.community etc.
  static unsigned long last_transmission_counts = 0;
  static unsigned long last_transmission_hv_pulses = 0;
  static unsigned long last_transmission_timestamp = millis();
  static unsigned long last_count_timestamp_2send = millis();
  if ((current_ms - last_transmission_timestamp) >= (MEASUREMENT_INTERVAL * 1000)) {
    last_transmission_timestamp = current_ms;
    unsigned long counts = GMC_counts_master - last_transmission_counts;
    last_transmission_counts = GMC_counts_master;

    int dt = GMC_count_timestamp_master - last_count_timestamp_2send;
    last_count_timestamp_2send = GMC_count_timestamp_master;

    unsigned int current_cpm;
    current_cpm = (dt != 0) ? (int)(counts * 60000 / dt) : 0;

    int hv_pulses = HV_pulses_master - last_transmission_hv_pulses;
    last_transmission_hv_pulses = HV_pulses_master;

    bool have_thp;
    float temperature, humidity, pressure;
    have_thp = read_thp_sensor(&temperature, &humidity, &pressure);
    if (have_thp)
      log(DEBUG, "Measured: cpm= %d HV=%d T=%.2f H=%.f P=%.f", current_cpm, hv_pulses, temperature, humidity, pressure);
    else
      log(DEBUG, "Measured: cpm= %d HV=%d", current_cpm, hv_pulses);

    transmit_data(tubes[TUBE_TYPE].type, tubes[TUBE_TYPE].nbr, dt, hv_pulses, counts, current_cpm,
                  have_thp, temperature, humidity, pressure);
  }

  static unsigned int last_tick_counts = 0;
  if (GMC_counts_master != last_tick_counts) {
    tick(ledTick && switches.led_on, speakerTick && switches.speaker_on);
    last_tick_counts = GMC_counts_master;
  }

  long loop_duration;
  loop_duration = millis() - current_ms;
  iotWebConf.delay((loop_duration < LOOP_DURATION) ? (LOOP_DURATION - loop_duration) : 0);

  wifi_connected = (iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE);
}

