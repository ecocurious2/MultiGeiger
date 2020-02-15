// speaker / sound related code
// also handles the onboard LED, which lights up while speaker ticks.

void setup_speaker(void);
void tick(int use_led, int use_speaker);
void tone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume);
void play_start_sound();
