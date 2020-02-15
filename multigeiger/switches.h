// DIP switch related code

// What are the switches good for?
enum {SPEAKER_ON, DISPLAY_ON, LED_ON, UNUSED};

void setup_switches(void);
char *read_switches(void);
