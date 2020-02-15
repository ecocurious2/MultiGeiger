// temperature, humidity, pressure sensor (usually a BME280) related code

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "log.h"
#include "thp_sensor.h"

int haveBME280 = 0;

float bme_temperature = 0.0;
float bme_humidity = 0.0;
float bme_pressure = 0.0;

Adafruit_BME280 bme;

void setup_thp_sensor() {
  haveBME280 = bme.begin(BME280_ADDRESS);
  if (!haveBME280)
    haveBME280 = bme.begin(BME280_ADDRESS_ALTERNATE);
  log(INFO, "BME_Status: %d  ID:%0X", haveBME280, bme.sensorID());
}

void read_thp_sensor() {
  if (haveBME280) {
    bme_temperature = bme.readTemperature();
    bme_humidity = bme.readHumidity();
    bme_pressure = bme.readPressure();
  }
}

