// temperature, humidity, pressure sensor (usually a BME280) related code

extern int haveBME280;

extern float bme_temperature;
extern float bme_humidity;
extern float bme_pressure;

void setup_thp_sensor(void);
void read_thp_sensor(void);
