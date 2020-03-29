// Check if we have LoRa hardware

// Test pin 26 (that is LORA_DIO0) to be low.
#include <Arduino.h>

#define HWTESTPIN 26

// Test pin is LoRa_DIO0. If LoRa chip is present, this pin is low, otherwise
// it is an open pin with pullup
bool init_hwtest(void) {
  pinMode(HWTESTPIN, INPUT_PULLUP);
  delay(200);
  if (!digitalRead(HWTESTPIN)) {      // low => LoRa chip detected
    return true;
  }
  return false;
}
