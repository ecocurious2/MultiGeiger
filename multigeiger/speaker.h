// speaker / sound related code
// also handles the onboard LED, which lights up while speaker ticks.

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

void setup_speaker(bool playSound, bool led_tick, bool speaker_tick);
void tick(bool high);
void tick_enable(bool enable);

#endif // _SPEAKER_H_
