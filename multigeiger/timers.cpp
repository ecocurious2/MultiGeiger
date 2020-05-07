#include <Arduino.h>

#define RECHARGE_TIMER 0
hw_timer_t *recharge_timer = NULL;

void setup_recharge_timer(void (*isr_recharge)(), int period_us) {
  recharge_timer = timerBegin(RECHARGE_TIMER, 80, true);  // prescaler: 80MHz / 80 == 1MHz
  timerAttachInterrupt(recharge_timer, isr_recharge, true);  // set ISR
  timerAlarmWrite(recharge_timer, period_us, true);  // set alarm after period, do repeat
  timerWrite(recharge_timer, 0);
  timerAlarmEnable(recharge_timer);
}
