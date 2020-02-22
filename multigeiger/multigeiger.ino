// Project: Simple Multi-Geiger
// (c) 2019,2020 by the authors, see AUTHORS file in toplevel directory.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// (see LICENSE file in toplevel directory)
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

// Measurement interval (default 2.5min) [sec]
#define MEASUREMENT_INTERVAL 150

// MAX time to wait until connected. [msec]
// If there is still no connection after that time,
// measurements will start, but won't be sent to servers.
#define MAX_WAIT_TIME 300000

// Max time the greeting display will be on. [msec]
#define AFTERSTART 5000

void setup() {
  setup_log(DEFAULT_LOG_LEVEL);
  setup_display();
  setup_speaker();
  setup_switches();
  setup_thp_sensor();
  setup_webconf();
  setup_transmission(VERSION_STR, ssid);

  if (PLAY_SOUND)
    play_start_sound();

  setup_log_data(SERIAL_DEBUG);
  setup_tube();
}

#define DISPLAYREFRESH 10000
#define MAXCOUNTS 100
#define HVPULSE_MS 1000

void loop() {
  unsigned long time_difference;
  unsigned int HV_pulse_count;
  Switches switches;
  unsigned long current_ms = millis();  // to save multiple calls to millis()
  bool update_display;
  bool wifi_connected = false;
  
  unsigned int GMC_counts;
  unsigned long count_timestamp;
  static unsigned long last_count_timestamp = millis();

  static unsigned int accumulated_GMC_counts = 0;
  static unsigned long accumulated_time = 0;

  unsigned int GMC_counts_2send;
  unsigned long count_timestamp_2send;
  static unsigned long last_count_timestamp_2send = millis();
  static unsigned long transmission_timestamp = millis();

  static unsigned int hvpulsecnt2send = 0;

  static float Count_Rate = 0.0, Dose_Rate = 0.0;
  static float accumulated_Count_Rate = 0.0, accumulated_Dose_Rate = 0.0;

  static unsigned int lastMinuteLogCounts = 0;
  static unsigned long minute_log_timestamp = millis();

  static unsigned long display_timestamp = millis();

  switches = read_switches();

  // copy values from ISR
  portENTER_CRITICAL(&mux_GMC_count);                            // enter critical section
  GMC_counts = isr_GMC_counts;
  // Check if there are enough pulses detected or if enough time has elapsed.
  // If yes, then it is time to calculate the pulse rate, update the display and recharge the HV capacitor.
  update_display = (GMC_counts >= MAXCOUNTS) || ((current_ms - display_timestamp) >= DISPLAYREFRESH);
  if (update_display) isr_GMC_counts = 0;
  count_timestamp = isr_count_timestamp;
  portEXIT_CRITICAL(&mux_GMC_count);                             // leave critical section

  // Pulse the high voltage if we got enough GMC pulses to update the display or at least every 1000ms.
  if (update_display || (current_ms - hvpulse_timestamp) >= HVPULSE_MS) {
    HV_pulse_count = gen_charge_pulses(false);               // charge HV capacitor - sets hvpulse_timestamp!
    hvpulsecnt2send += HV_pulse_count;                       // count for sending
  }

  if (update_display) {
    display_timestamp = current_ms;
    time_difference = count_timestamp - last_count_timestamp;  // calculate all derived values
    last_count_timestamp = count_timestamp;                    // notice the old timestamp
    accumulated_time += time_difference;                       // accumulate all the time
    accumulated_GMC_counts += GMC_counts;                      // accumulate all the pulses
    lastMinuteLogCounts += GMC_counts;

    float GMC_factor_uSvph = tubes[TUBE_TYPE].cps_to_uSvph;

    // calculate the current count rate and dose rate
    if (time_difference != 0)
      Count_Rate = (float)GMC_counts * 1000.0 / (float)time_difference;
    else
      Count_Rate = 0.0;  // avoid division by zero

    Dose_Rate = Count_Rate * GMC_factor_uSvph;                          // ... and dose rate

    // calculate the count rate and dose rate over the complete time from start
    if (accumulated_time != 0)
      accumulated_Count_Rate = (float)accumulated_GMC_counts * 1000.0 / (float)accumulated_time;
    else
      accumulated_Count_Rate = 0.0;  // avoid division by zero

    accumulated_Dose_Rate = accumulated_Count_Rate * GMC_factor_uSvph;

    // ... and display it.
    DisplayGMC(((int)accumulated_time / 1000), (int)(accumulated_Dose_Rate * 1000), (int)(Count_Rate * 60),
               (SHOW_DISPLAY && switches.display_on), wifi_connected);

    if (Serial_Print_Mode == Serial_Logging) {                       // Report all
      log_data(GMC_counts, time_difference, Count_Rate, Dose_Rate, HV_pulse_count,
               accumulated_GMC_counts, accumulated_time, accumulated_Count_Rate, accumulated_Dose_Rate);
    }

    if (Serial_Print_Mode == Serial_One_Minute_Log) {                // 1 Minute Log active?
      if (current_ms - minute_log_timestamp > 60000) {               // Time reached for next 1-Minute log?
        unsigned int lastMinuteLogCountRate = ((lastMinuteLogCounts * 60000) / (current_ms - minute_log_timestamp));   // = * 60 / 1000
        if (((((lastMinuteLogCounts * 60000) % (current_ms - minute_log_timestamp)) * 2) / (current_ms - minute_log_timestamp)) >= 1) {
          lastMinuteLogCountRate++;                                  // Rounding + 0.5
        }
        log_data_one_minute((current_ms / 1000), lastMinuteLogCountRate, lastMinuteLogCounts);
        lastMinuteLogCounts = 0;
        minute_log_timestamp = current_ms;
      }
    }
  }

  if ((Serial_Print_Mode == Serial_Statistics_Log) && isr_gotGMCpulse) {  // statistics log active?
    unsigned int count_time_between;
    portENTER_CRITICAL(&mux_GMC_count);
    count_time_between = isr_count_time_between;
    isr_gotGMCpulse = 0;
    portEXIT_CRITICAL(&mux_GMC_count);
    log_data_statistics(count_time_between);
  }

  // If there were no pulses after 3 secs after start,
  // clear display anyway and show 0 counts.
  static unsigned long afterStartTime = AFTERSTART;
  if (afterStartTime && ((current_ms - transmission_timestamp) >= afterStartTime)) {
    afterStartTime = 0;
    DisplayGMC(((int)accumulated_time / 1000), (int)(accumulated_Dose_Rate * 1000), (int)(Count_Rate * 60),
               (SHOW_DISPLAY && switches.display_on), wifi_connected);
  }

  // Check, if we have to send to sensor.community etc.
  if ((current_ms - transmission_timestamp) >= (MEASUREMENT_INTERVAL * 1000)) {
    transmission_timestamp = current_ms;
    portENTER_CRITICAL(&mux_GMC_count);
    GMC_counts_2send = isr_GMC_counts_2send;                    // copy values from ISR
    count_timestamp_2send = isr_count_timestamp_2send;
    isr_GMC_counts_2send = 0;
    portEXIT_CRITICAL(&mux_GMC_count);
    unsigned int hvp = hvpulsecnt2send;
    hvpulsecnt2send = 0;
    time_difference = count_timestamp_2send - last_count_timestamp_2send;
    last_count_timestamp_2send = count_timestamp_2send;

    unsigned int current_cpm;
    if (time_difference != 0)
      current_cpm = (int)(GMC_counts_2send * 60000 / time_difference);
    else
      current_cpm = 0;  // avoid division by zero

    if (have_thp) {  // temperature / humidity / pressure
      read_thp_sensor();
      log(DEBUG, "Measured: cpm= %d HV=%d T=%.2f H=%.f P=%.f", current_cpm, hvp, temperature, humidity, pressure);
    } else {
      log(DEBUG, "Measured: cpm= %d HV=%d", current_cpm, hvp);
    }

    transmit_data(tubes[TUBE_TYPE].type, tubes[TUBE_TYPE].nbr, time_difference, hvp, GMC_counts_2send, current_cpm,
                  have_thp, temperature, humidity, pressure);
  }

  static unsigned int last_GMC_counts = 0;
  if (GMC_counts != last_GMC_counts) {
    tick(LED_TICK && switches.led_on, SPEAKER_TICK && switches.speaker_on);
    last_GMC_counts = GMC_counts;
  }

  iotWebConf.doLoop();  // see webconf.cpp
  wifi_connected = (iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE);
}

