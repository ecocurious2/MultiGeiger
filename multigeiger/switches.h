// DIP switch related code

#ifndef _SWITCHES_H_
#define _SWITCHES_H_

// What are the switches good for?
typedef struct switches {
  unsigned int speaker_on: 1;  // SW0
  unsigned int display_on: 1;  // SW1
  unsigned int led_on: 1;  // SW2
  unsigned int unused: 1;  // SW3
} Switches;

void setup_switches(void);
Switches read_switches(void);

#endif // _SWITCHES_H_
