// temperature, humidity, pressure sensor (usually a BME280) related code

#ifndef _THP_SENSOR_H_
#define _THP_SENSOR_H_

extern int have_thp;

extern float temperature;
extern float humidity;
extern float pressure;

void setup_thp_sensor(void);
void read_thp_sensor(void);

#endif // _THP_SENSOR_H_
