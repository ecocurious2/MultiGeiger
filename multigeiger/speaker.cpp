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

void cycle(int t1, int t2, int volume) {
  // output one full cycle of the sound wave
  // volume: 0: low, 1: high
  digitalWrite(PIN_SPEAKER_OUTPUT_P, LOW);
  digitalWrite(PIN_SPEAKER_OUTPUT_N, HIGH);
  delayMicroseconds(t1);
  digitalWrite(PIN_SPEAKER_OUTPUT_P, (volume == 1));
  digitalWrite(PIN_SPEAKER_OUTPUT_N, LOW);
  delayMicroseconds(t2);
}

void tick(int use_led, int use_speaker) {
  // make LED flicker and speaker tick
  if (use_led) {
    digitalWrite(LED_BUILTIN, HIGH);    // switch on LED
  }
  for (int t = 0; t < 4; t++) {
    if (use_speaker)
      cycle(500, 500, 1);  // takes 1ms
    else if (use_led)
      delay(1);            // also takes 1ms
  }
  if (use_led) {
    digitalWrite(LED_BUILTIN, LOW);     // switch off LED
  }
}

void tone(int frequency_mHz, int time_ms, int volume) {
  int cycle_time_us, cycle_1_time_us, cycle_2_time_us;
  unsigned long end_ms;

  cycle_time_us = 1000000000 / frequency_mHz;
  cycle_1_time_us = cycle_time_us / 2;
  cycle_2_time_us = cycle_time_us - cycle_1_time_us;

  end_ms = millis() + time_ms;
  do {
    cycle(cycle_1_time_us, cycle_2_time_us, volume);
  } while (millis() < end_ms);
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

