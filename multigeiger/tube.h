// code related to the geiger-mueller tube hw interface

#ifndef _TUBE_H_
#define _TUBE_H_

typedef struct {
  const char *type;          // type string for sensor.community
  const char nbr;            // number to be sent by LoRa
  const float cps_to_uSvph;  // factor to convert counts per second to µSievert per hour
} TUBETYPE;

extern TUBETYPE tubes[];

void setup_tube(void);
void read_GMC(unsigned long *counts, unsigned long *timestamp, unsigned int *between);
void read_hv(bool *hv_error, unsigned long *pulses);

#endif // _TUBE_H_
