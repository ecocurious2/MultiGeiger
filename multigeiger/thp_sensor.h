// temperature, humidity, pressure sensor (usually a BME280) related code

#ifndef _THP_SENSOR_H_
#define _THP_SENSOR_H_

bool setup_thp_sensor(void);
bool read_thp_sensor(float *temperature, float *humidity, float *pressure);

#endif // _THP_SENSOR_H_
