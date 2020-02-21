// DIP switch related code

#include <Arduino.h>

#include "log.h"
#include "switches.h"

// Inputs for the switches
#if CPU == STICK
#define PIN_SWI_0 36
#define PIN_SWI_1 37
#define PIN_SWI_2 38
#define PIN_SWI_3 39
#else
#define PIN_SWI_0 39
#define PIN_SWI_1 38
#define PIN_SWI_2 37
#define PIN_SWI_3 36
#endif

void setup_switches() {
  pinMode(PIN_SWI_0, INPUT);  // These pins DON'T HAVE PULLUPS!
  pinMode(PIN_SWI_1, INPUT);
  pinMode(PIN_SWI_2, INPUT);
  pinMode(PIN_SWI_3, INPUT);

  Switches s = read_switches();
  log(DEBUG, "Switches: SW0 speaker_on %d,  SW1 display_on %d,  SW2 led_on: %d,  SW3 unused: %d",
      s.speaker_on, s.display_on, s.led_on, s.unused);
}

Switches read_switches() {
  Switches s;

  // Read Switches (active LOW!)
  s.speaker_on = !digitalRead(PIN_SWI_0);
  s.display_on = !digitalRead(PIN_SWI_1);
  s.led_on = !digitalRead(PIN_SWI_2);
  s.unused = !digitalRead(PIN_SWI_3);

  return s;
}
