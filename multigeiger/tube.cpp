// code related to the geiger-mueller tube hw interface
// - high voltage generation
// - GM pulse counting

#include <Arduino.h>

#include "log.h"
#include "timers.h"
#include "tube.h"

// The test pin, if enabled, is high while isr_GMC_count is active.
// -1 == disabled, otherwise pin 13 might be an option.
#define PIN_TEST_OUTPUT -1

#define PIN_HV_FET_OUTPUT 23
#define PIN_HV_CAP_FULL_INPUT 22  // !! has to be capable of "interrupt on change"
#define PIN_GMC_COUNT_INPUT 2     // !! has to be capable of "interrupt on change"

// Dead Time of the Geiger Counter. [usec]
// Has to be longer than the complete pulse generated on the Pin PIN_GMC_COUNT_INPUT.
#define GMC_DEAD_TIME 190

TUBETYPE tubes[] = {
  // use 0.0 conversion factor for unknown tubes, so it computes an "obviously-wrong" 0.0 uSv/h value rather than a confusing one.
  {"Radiation unknown", 0, 0.0},
  // The conversion factors for SBM-20 and SBM-19 are taken from the datasheets (according to Jürgen)
  {"Radiation SBM-20", 20, 1 / 2.47},
  {"Radiation SBM-19", 19, 1 / 9.81888},
  // The Si22G conversion factor was determined by Juergen Boehringer like this:
  // Set up a Si22G based MultiGeiger close to the official odlinfo.bfs.de measurement unit in Sindelfingen.
  // Determine how many counts the Si22G gives within the same time the odlinfo unit needs for 1uSv.
  // Result: 44205 counts on the Si22G for 1 uSv.
  // So, to convert from cps to uSv/h, the calculation is: uSvh = cps * 3600 / 44205 = cps / 12.2792
  {"Radiation Si22G", 22, 1 / 12.2792}
};

volatile bool isr_GMC_cap_full;

volatile unsigned long isr_hv_pulses;
volatile bool isr_hv_charge_error;

volatile unsigned int isr_GMC_counts;
volatile unsigned long isr_count_timestamp;
volatile unsigned long isr_count_time_between;

// MUX (mutexes used for mutual exclusive access to isr variables)
portMUX_TYPE mux_cap_full = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE mux_GMC_count = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE mux_hv = portMUX_INITIALIZER_UNLOCKED;

// Maximum amount of HV capacitor charge pulses to generate in one charge cycle.
#define MAX_CHARGE_PULSES 3333

// hw timer period and microseconds -> periods conversion
#define PERIOD_DURATION_US 100
#define PERIODS(us) ((us) / PERIOD_DURATION_US)

void IRAM_ATTR isr_recharge() {
  // this code is periodically called by a timer hw interrupt, always same period.
  // we need to decide internally whether we actually want to do something.
  //
  // note: this is implemented like it is because dynamically reprogramming the hw timer
  // to a different period would require us to call library functions like timerAlarmWrite
  // which are **not** in IRAM (but in flash) and doing that can lead to spurious fatal
  // exceptions like "Cache disabled but cached memory region accessed".
  static unsigned int current = 0;  // current period counter
  static unsigned int next_state = 0;  // periods to next state machine execution
  static unsigned int next_charge = PERIODS(1000000);  // periods between recharges, initially 1s
  if (current++ < next_state)
    return;  // nothing to do yet

  // we reached "next_state", so we execute the state machine:
  current = 0;

  enum State {init, pulse_h, pulse_l, check_full, is_full, charge_fail};
  static State state = init;
  static int charge_pulses;
  if (state == init) {
    charge_pulses = 0;
    portENTER_CRITICAL_ISR(&mux_cap_full);
    isr_GMC_cap_full = 0;
    portEXIT_CRITICAL_ISR(&mux_cap_full);
    state = pulse_h;
    // fall through
  }
  while (state < is_full) {
    if (state == pulse_h) {
      digitalWrite(PIN_HV_FET_OUTPUT, HIGH);  // turn the HV FET on
      state = pulse_l;
      next_state = PERIODS(1500);  // 1500us (5000us gives 1.3 times more charge, 500us gives 1/20th of charge)
      return;
    }
    if (state == pulse_l) {
      digitalWrite(PIN_HV_FET_OUTPUT, LOW);   // turn the HV FET off
      state = check_full;
      next_state = PERIODS(1000);  // 1000us
      return;
    }
    if (state == check_full) {
      charge_pulses++;
      if (isr_GMC_cap_full)
        state = is_full;
      else if (charge_pulses < MAX_CHARGE_PULSES)
        state = pulse_h;
      else
        state = charge_fail;
      // fall through
    }
  }
  if (state == is_full) {
    // capacitor full
    portENTER_CRITICAL_ISR(&mux_hv);
    isr_hv_charge_error = false;
    isr_hv_pulses += charge_pulses;
    portEXIT_CRITICAL_ISR(&mux_hv);
    state = init;
    // depending on a lot of circumstances (e.g. level of radiation, humidity,
    // leak currents (diode leak current depends on temperature), tube type, ...),
    // we might need to charge the HV capacitor more or less often.
    // we target charging with 2 pulses here because if we only needed 1 charge
    // pulse, this does not imply the HV capacitor actually needed charging. if
    // we needed 2 pulses, we are sure the HV capacitor needed a little charge.
    if (charge_pulses <= 1) {
      // one charge pulse was enough, so maybe we charge too often
      next_charge = next_charge * 5 / 4;
    } else {
      // 2 charge pulses: no change
      // > 2: the more charge pulses we needed, the more frequently we want to recharge
      next_charge = next_charge * 2 / charge_pulses;
    }
    // never go below 1ms or above 10s
    if (next_charge < PERIODS(1000))
      next_charge = PERIODS(1000);
    else if (next_charge > PERIODS(10000000))
      next_charge = PERIODS(10000000);
    next_state = next_charge;
    return;
  }
  if (state == charge_fail) {
    // capacitor does not charge!
    portENTER_CRITICAL_ISR(&mux_hv);
    isr_hv_charge_error = true;
    isr_hv_pulses += charge_pulses;
    portEXIT_CRITICAL_ISR(&mux_hv);
    // let's retry charging later
    state = init;
    next_charge = PERIODS(1000000);  // reset to default 1s charge interval
    next_state = PERIODS(10 * 60 * 1000000);  // wait for 10 minutes before retrying
    return;
  }
}

void IRAM_ATTR isr_GMC_capacitor_full() {
  portENTER_CRITICAL_ISR(&mux_cap_full);
  isr_GMC_cap_full = 1;
  portEXIT_CRITICAL_ISR(&mux_cap_full);
}

void read_hv(bool *hv_error, unsigned long *pulses) {
  portENTER_CRITICAL(&mux_hv);
  *pulses = isr_hv_pulses;
  *hv_error = isr_hv_charge_error;
  portEXIT_CRITICAL(&mux_hv);
}

void IRAM_ATTR isr_GMC_count() {
  unsigned long now;
  static unsigned long last;
  portENTER_CRITICAL_ISR(&mux_GMC_count);
  #if PIN_TEST_OUTPUT >= 0
  digitalWrite(PIN_TEST_OUTPUT, HIGH);
  #endif
  now = micros();
  if (now > (last + GMC_DEAD_TIME)) {
    // We only consider a pulse valid if it happens more than GMC_DEAD_TIME after the last valid pulse.
    // Reason: Pulses occurring short after a valid pulse are false pulses generated by the rising edge on the PIN_GMC_COUNT_INPUT.
    //         This happens because we don't have a Schmitt trigger on this controller pin.
    isr_count_timestamp = millis();        // remember (system) time of the pulse
    isr_count_time_between = now - last;   // save for statistics debuging
    isr_GMC_counts++;                      // count the pulse
    last = now;                            // remember timestamp of last **valid** pulse
  }
  #if PIN_TEST_OUTPUT >= 0
  digitalWrite(PIN_TEST_OUTPUT, LOW);
  #endif
  portEXIT_CRITICAL_ISR(&mux_GMC_count);
}

void read_GMC(unsigned long *counts, unsigned long *timestamp, unsigned int *between) {
  portENTER_CRITICAL(&mux_GMC_count);
  *counts += isr_GMC_counts;
  isr_GMC_counts = 0;
  *timestamp = isr_count_timestamp;
  *between = isr_count_time_between;
  portEXIT_CRITICAL(&mux_GMC_count);
}

void setup_tube(void) {
  pinMode(PIN_TEST_OUTPUT, OUTPUT);
  pinMode(PIN_HV_FET_OUTPUT, OUTPUT);
  pinMode(PIN_HV_CAP_FULL_INPUT, INPUT);
  pinMode(PIN_GMC_COUNT_INPUT, INPUT);

  digitalWrite(PIN_TEST_OUTPUT, LOW);
  digitalWrite(PIN_HV_FET_OUTPUT, LOW);

  unsigned long now_ms = millis();

  // note: we do not need to get the portMUX here as we did not yet enable interrupts.
  isr_count_timestamp = now_ms;
  isr_count_time_between = 0;
  isr_GMC_cap_full = 0;
  isr_GMC_counts = 0;
  isr_hv_pulses = 0;
  isr_hv_charge_error = false;

  attachInterrupt(digitalPinToInterrupt(PIN_HV_CAP_FULL_INPUT), isr_GMC_capacitor_full, RISING);  // capacitor full
  attachInterrupt(digitalPinToInterrupt(PIN_GMC_COUNT_INPUT), isr_GMC_count, FALLING);            // GMC pulse detected

  setup_recharge_timer(isr_recharge, PERIOD_DURATION_US);
}
