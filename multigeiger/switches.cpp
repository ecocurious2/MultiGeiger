// DIP switch related code

#include <Arduino.h>

#include "log.h"
#include "switches.h"

// Inputs for the switches
static unsigned int PIN_SWI_0, PIN_SWI_1, PIN_SWI_2, PIN_SWI_3;

void setup_switches(bool isLoraBoard) {
  if (isLoraBoard) {
    PIN_SWI_0 = 36;
    PIN_SWI_1 = 37;
    PIN_SWI_2 = 38;
    PIN_SWI_3 = 39;
  } else {
    PIN_SWI_0 = 39;
    PIN_SWI_1 = 38;
    PIN_SWI_2 = 37;
    PIN_SWI_3 = 36;
  };
  pinMode(PIN_SWI_0, INPUT);  // These pins DON'T HAVE PULLUPS!
  pinMode(PIN_SWI_1, INPUT);
  pinMode(PIN_SWI_2, INPUT);
  pinMode(PIN_SWI_3, INPUT);

  Switches s = read_switches();
  log(DEBUG, "Switches: SW0 speaker_on %d,  SW1 display_on %d,  SW2 led_on: %d,  SW3 ble_on: %d",
      s.speaker_on, s.display_on, s.led_on, s.ble_on);
}

Switches read_switches() {
  Switches s;

  // Read Switches (active LOW!)
  s.speaker_on = !digitalRead(PIN_SWI_0);
  s.display_on = !digitalRead(PIN_SWI_1);
  s.led_on = !digitalRead(PIN_SWI_2);
  s.ble_on = !digitalRead(PIN_SWI_3);

  return s;
}
