// speaker / sound related code
// also handles the onboard LED, which lights up while speaker ticks.

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

void setup_speaker(void);
void tick(int use_led, int use_speaker);
void tone(int frequency_mHz, int time_ms, int volume);
void play_start_sound(void);

#endif // _SPEAKER_H_
