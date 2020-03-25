// code related to the geiger-mueller tube hw interface

#ifndef _TUBE_H_
#define _TUBE_H_

typedef struct {
  const char *type;          // type string for sensor.community
  const char nbr;            // number to be sent by LoRa
  const float cps_to_uSvph;  // factor to convert counts per second to µSievert per hour
} TUBETYPE;

extern TUBETYPE tubes[];
extern volatile bool isr_GMC_cap_full;
extern volatile bool isr_gotGMCpulse;
extern volatile unsigned int isr_GMC_counts;
extern volatile unsigned int isr_GMC_counts_2send;
extern volatile unsigned long isr_count_timestamp;
extern volatile unsigned long isr_count_timestamp_2send;
extern volatile unsigned long isr_count_time_between;

extern portMUX_TYPE mux_cap_full;
extern portMUX_TYPE mux_GMC_count;

int charge_hv(bool forced, unsigned long current_ms);
int gen_charge_pulses(bool setup);
void setup_tube(void);

#endif // _TUBE_H_
