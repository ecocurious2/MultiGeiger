#include <Arduino.h>

#define RECHARGE_TIMER 0
#define AUDIO_TIMER 1

hw_timer_t *recharge_timer = NULL;
hw_timer_t *audio_timer = NULL;

hw_timer_t *setup_timer(int timer_no, void (*isr)(), int period_us) {
  hw_timer_t *timer = timerBegin(timer_no, 80, true);  // prescaler: 80MHz / 80 == 1MHz
  timerAttachInterrupt(timer, isr, true);  // set ISR
  timerAlarmWrite(timer, period_us, true);  // set alarm after period, do repeat
  timerWrite(timer, 0);
  timerAlarmEnable(timer);
  return timer;
}

void setup_recharge_timer(void (*isr_recharge)(), int period_us) {
  recharge_timer = setup_timer(RECHARGE_TIMER, isr_recharge, period_us);
}

void setup_audio_timer(void (*isr_audio)(), int period_us) {
  audio_timer = setup_timer(AUDIO_TIMER, isr_audio, period_us);
}

