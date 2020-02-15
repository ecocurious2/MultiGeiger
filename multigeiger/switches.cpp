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

  char *sw = read_switches();
  log(DEBUG, "SW0: %d  SW1: %d  SW2: %d  SW3: %d", sw[0], sw[1], sw[2], sw[3]);
}

char *read_switches() {
  static char sw[4];

  // Read Switches (active LOW!)
  sw[0] = !digitalRead(PIN_SWI_0);
  sw[1] = !digitalRead(PIN_SWI_1);
  sw[2] = !digitalRead(PIN_SWI_2);
  sw[3] = !digitalRead(PIN_SWI_3);

  return sw;
}
