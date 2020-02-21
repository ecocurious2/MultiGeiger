// temperature, humidity, pressure sensor (usually a BME280) related code

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "log.h"
#include "thp_sensor.h"

int have_thp = 0;

float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

Adafruit_BME280 bme;

void setup_thp_sensor() {
  have_thp = bme.begin(BME280_ADDRESS);
  if (!have_thp)
    have_thp = bme.begin(BME280_ADDRESS_ALTERNATE);
  log(INFO, "BME_Status: %d  ID:%0X", have_thp, bme.sensorID());
}

void read_thp_sensor() {
  if (have_thp) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure();
  }
}

