// speaker / sound related code
// also handles the onboard LED, which lights up while speaker ticks.

#include <Arduino.h>
#include <driver/mcpwm.h>

#include "speaker.h"
#include "timers.h"

#define PIN_SPEAKER_OUTPUT_P 12
#define PIN_SPEAKER_OUTPUT_N 0

// MUX (mutexes used for mutual exclusive access to isr variables)
portMUX_TYPE mux_audio = portMUX_INITIALIZER_UNLOCKED;

volatile int *isr_audio_sequence = NULL;
volatile int *isr_tick_sequence = NULL;
volatile int *isr_sequence = NULL;  // currently played sequence

static int tick_sequence[8], tock_sequence[8];

// hw timer period and microseconds -> periods conversion
#define PERIOD_DURATION_US 1000
#define PERIODS(us) ((us) / PERIOD_DURATION_US)

void IRAM_ATTR isr_audio() {
  // this code is periodically called by a timer hw interrupt, always same period.
  // we need to decide internally whether we actually want to do something.
  //
  // note: this is implemented like it is because dynamically reprogramming the hw timer
  // to a different period would require us to call library functions like timerAlarmWrite
  // which are **not** in IRAM (but in flash) and doing that can lead to spurious fatal
  // exceptions like "Cache disabled but cached memory region accessed".
  static unsigned int current = 0;  // current period counter
  static unsigned int next = PERIODS(1000);  // periods to next sequencer execution
  if (current++ < next)
    return;  // nothing to do yet

  // we reached "next", so we execute the sequencer:
  current = 0;

  // tone and tick generation, also led blinking
  int frequency_mHz, volume, led, duration_ms;
  static bool playing_audio = false, playing_tick = false;

  // fetch next tone / next led state
  portENTER_CRITICAL_ISR(&mux_audio);
  if (!isr_sequence) {
    if (isr_audio_sequence) {
      isr_sequence = isr_audio_sequence;
      playing_audio = true;
    } else if (isr_tick_sequence) {
      isr_sequence = isr_tick_sequence;
      playing_tick = true;
    }
  }
  volatile int *p = isr_sequence;
  if (p) {
    frequency_mHz = *p++;
    volume = *p++;
    led = *p++;
    duration_ms = *p++;
    isr_sequence = p;
  }
  portEXIT_CRITICAL_ISR(&mux_audio);

  if (!p)
    return;  // nothing to do

  // note: by all means, **AVOID** mcpwm_set_duty() in ISR, causes floating point coprocessor troubles!
  //       when just calling mcpwm_set_duty_**type**(), it will reuse a previously set duty cycle.
  if (frequency_mHz > 0) {
    // enable sound output
    if (volume >= 1) {
      // high volume - MCPWM A/B outputs generate inverted signals
      mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
      mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_1);
    } else {
      // low volume - do MCPWM on A, keep B permanently low
      mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
      mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    }
    // set frequency
    mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, frequency_mHz / 1000);
    // start outputting PWM signal(s)
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
  } else {
    // frequency_mHz == 0 -> disable sound output
    // stop any PWM signals
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    // keep A high and B low (we have a piezo, no current flowing)
    mcpwm_set_signal_high(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
  }

  if (led >= 0)  // led == -1 can be used as "don't touch LED"
    digitalWrite(LED_BUILTIN, led ? HIGH : LOW);

  if (duration_ms > 0) {
    next = PERIODS(duration_ms * 1000);
  } else {
    // duration == 0 marks the end of the sequence to play
    portENTER_CRITICAL_ISR(&mux_audio);
    isr_sequence = NULL;
    if (playing_tick)
      isr_tick_sequence = NULL;
    else if (playing_audio)
      isr_audio_sequence = NULL;
    portEXIT_CRITICAL_ISR(&mux_audio);
    next = PERIODS(1000);
  }
}

void IRAM_ATTR tick(bool tick) {
  // make speaker tick and LED blink (or tock, no LED), called from ISR!
  portENTER_CRITICAL_ISR(&mux_audio);
  isr_tick_sequence = tick ? tick_sequence : tock_sequence;
  portEXIT_CRITICAL_ISR(&mux_audio);
}

void play(int *sequence) {
  // play a tone sequence, called from normal code (not ISR)
  portENTER_CRITICAL(&mux_audio);
  isr_audio_sequence = sequence;
  portEXIT_CRITICAL(&mux_audio);
}

void init_tick_sequence(int *sequence, bool tick, bool use_led, bool use_speaker) {
  // tick: 5000Hz, 4ms, led
  // tock: 1000Hz, 4ms, no led
  sequence[0] = use_speaker ? (tick ? 5000000 : 1000000) : 0;  // frequency_mHz
  sequence[1] = 1;  // volume
  sequence[2] = use_led ? (tick ? 1 : -1) : -1;  // led
  sequence[3] = 4;  // duration_ms

  sequence[4] = 0;  // silence
  sequence[5] = 0;
  sequence[6] = use_led ? (tick ? 0 : -1) : -1;  // led
  sequence[7] = 0;  // END
}

#define TONE(f, v, led, t) {int(f * 0.75), v, led, int(t * 85)}

void setup_speaker(bool playSound, bool use_led, bool use_speaker) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // LED off

  init_tick_sequence(tick_sequence, true, use_led, use_speaker);
  init_tick_sequence(tock_sequence, false, use_led, use_speaker);

  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_SPEAKER_OUTPUT_P);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PIN_SPEAKER_OUTPUT_N);

  mcpwm_config_t pwm_config;
  pwm_config.frequency = 1000;
  // set duty cycles to 50% (and never modify them again!)
  pwm_config.cmpr_a = 50.0;
  pwm_config.cmpr_b = 50.0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

  setup_audio_timer(isr_audio, PERIOD_DURATION_US);

  if (playSound) {
    static int sequence[][4] = {
      TONE(1174659, 1, -1, 2),  // D
      TONE(0, 0, 0, 2),         // ---
      TONE(1318510, 1, -1, 2),  // E
      TONE(0, 0, 0, 2),         // ---
      TONE(1479978, 1, -1, 2),  // Fis
      TONE(0, 0, 0, 2),         // ---
      TONE(1567982, 1, -1, 4),  // G
      TONE(1174659, 1, -1, 2),  // D
      TONE(1318510, 1, -1, 2),  // E
      TONE(1174659, 1, -1, 4),  // D
      TONE(987767, 1, -1, 2),   // H
      TONE(1046502, 1, -1, 2),  // C
      TONE(987767, 1, -1, 4),   // H
      TONE(987767, 0, -1, 4),   // H
      TONE(0, 0, -1, 0),        // speaker off, end
    };
    play((int *)sequence);
  }
}
