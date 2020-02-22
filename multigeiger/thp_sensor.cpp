// temperature, humidity, pressure sensor (usually a BME280) related code

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "log.h"
#include "thp_sensor.h"

static bool have_thp = false;

Adafruit_BME280 bme;

bool setup_thp_sensor(void) {
  have_thp = (bool) bme.begin(BME280_ADDRESS);
  if (!have_thp)
    have_thp = (bool) bme.begin(BME280_ADDRESS_ALTERNATE);
  if (have_thp)
    log(INFO, "BME_Status: %d  ID:%0X", (int) have_thp, bme.sensorID());
  else
    log(INFO, "BME_Status: %d  ID:None", (int) have_thp);
  return have_thp;
}

bool read_thp_sensor(float *temperature, float *humidity, float *pressure) {
  *temperature = have_thp ? bme.readTemperature() : 0.0;
  *humidity = have_thp ? bme.readHumidity() : 0.0;
  *pressure = have_thp ? bme.readPressure() : 0.0;
  return have_thp;
}

