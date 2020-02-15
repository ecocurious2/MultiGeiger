// speaker / sound related code
// also handles the onboard LED, which lights up while speaker ticks.

#include <Arduino.h>

#include "speaker.h"

#define PIN_SPEAKER_OUTPUT_P 12
#define PIN_SPEAKER_OUTPUT_N 0

void setup_speaker() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_SPEAKER_OUTPUT_P, OUTPUT);
  pinMode(PIN_SPEAKER_OUTPUT_N, OUTPUT);

  // LED off
  digitalWrite(LED_BUILTIN, LOW);
  // note: piezo beeper, thus no current is flowing after capacitive charge up:
  digitalWrite(PIN_SPEAKER_OUTPUT_P, HIGH);
  digitalWrite(PIN_SPEAKER_OUTPUT_N, LOW);
}

void tick(int use_led, int use_speaker) {
  // make LED flicker and speaker tick
  if (use_led) {
    digitalWrite(LED_BUILTIN, HIGH);    // switch on LED
  }
  if (use_speaker) {  // make "Tick" sound
    for (int speaker_count = 0; speaker_count <= 3; speaker_count++) {
      digitalWrite(PIN_SPEAKER_OUTPUT_P, LOW);
      digitalWrite(PIN_SPEAKER_OUTPUT_N, HIGH);
      delayMicroseconds(500);
      digitalWrite(PIN_SPEAKER_OUTPUT_P, HIGH);
      digitalWrite(PIN_SPEAKER_OUTPUT_N, LOW);
      delayMicroseconds(500);
    }
  } else {
    if (use_led) {
      delay(4);
    }
  }
  if (use_led) {
    digitalWrite(LED_BUILTIN, LOW);     // switch off LED
  }
}

void tone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume) {
  unsigned int cycle_time_us, cycle_1_time_us, cycle_2_time_us;
  unsigned long count_timestamp_end;

  cycle_time_us = 1000000000 / frequency_mHz;
  cycle_1_time_us = cycle_time_us / 2;
  cycle_2_time_us = cycle_time_us - cycle_1_time_us;
  count_timestamp_end = millis() + time_ms;

  do {
    digitalWrite(PIN_SPEAKER_OUTPUT_P, (volume == 1));
    digitalWrite(PIN_SPEAKER_OUTPUT_N, LOW);
    delayMicroseconds(cycle_1_time_us);
    digitalWrite(PIN_SPEAKER_OUTPUT_P, LOW);
    digitalWrite(PIN_SPEAKER_OUTPUT_N, HIGH);
    delayMicroseconds(cycle_2_time_us);
  } while (millis() < count_timestamp_end);
}

void play_start_sound() {
  float freq_factor = 0.75;
  int time_factor = 85;

  tone(1174659 * freq_factor, 2 * time_factor, 1); // D
  delay(2 * time_factor);                          // ---
  tone(1318510 * freq_factor, 2 * time_factor, 1); // E
  delay(2 * time_factor);                          // ---
  tone(1479978 * freq_factor, 2 * time_factor, 1); // Fis
  delay(2 * time_factor);                          // ---

  tone(1567982 * freq_factor, 4 * time_factor, 1); // G
  tone(1174659 * freq_factor, 2 * time_factor, 1); // D
  tone(1318510 * freq_factor, 2 * time_factor, 1); // E
  tone(1174659 * freq_factor, 4 * time_factor, 1); // D
  tone(987767 * freq_factor, 2 * time_factor, 1);  // H
  tone(1046502 * freq_factor, 2 * time_factor, 1); // C
  tone(987767 * freq_factor, 4 * time_factor, 1);  // H
  tone(987767 * freq_factor, 4 * time_factor, 0);  // H
}

