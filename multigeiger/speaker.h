// speaker / sound related code
// also handles the onboard LED, which lights up while speaker ticks.

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

void setup_speaker(bool playSound, bool use_led, bool use_speaker);
void tick(bool tick);

#endif // _SPEAKER_H_
