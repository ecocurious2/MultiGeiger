// temperature, humidity, pressure sensor (usually a BME280) related code

#ifndef _THP_SENSOR_H_
#define _THP_SENSOR_H_

#define BME680_BSEC_LP_READOUT_INTERVAL 3000  // ms

bool setup_thp_sensor(void);
bool read_thp_sensor(float *temperature, float *humidity, float *pressure, int *iaq);

#endif // _THP_SENSOR_H_
