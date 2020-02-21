// DIP switch related code

#ifndef _SWITCHES_H_
#define _SWITCHES_H_

// What are the switches good for?
enum {SPEAKER_ON, DISPLAY_ON, LED_ON, UNUSED};

void setup_switches(void);
char *read_switches(void);

#endif // _SWITCHES_H_
