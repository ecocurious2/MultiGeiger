// temperature, humidity, pressure sensor (usually a BME280) related code

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BME680.h>

#include "log.h"
#include "thp_sensor.h"

static int type_thp = 0;

Adafruit_BME280 bme280;
Adafruit_BME680 bme680;

bool setup_thp_sensor(void) {
  // BME280
  if (bme280.begin(BME280_ADDRESS))
    type_thp = 280;
  else if (bme280.begin(BME280_ADDRESS_ALTERNATE))
    type_thp = 280;

  // BME680
  if (type_thp == 0) {
    if (bme680.begin(BME680_I2C_ADDR_PRIMARY))
      type_thp = 680;
    else if (bme680.begin(BME680_I2C_ADDR_SECONDARY))
      type_thp = 680;
  }

  switch (type_thp) {
  case 680:
    // Set up oversampling and filter initialization
    bme680.setTemperatureOversampling(BME680_OS_8X);
    bme680.setHumidityOversampling(BME680_OS_2X);
    bme680.setPressureOversampling(BME680_OS_4X);
    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme680.setGasHeater(300, 150); // 300*C for 150 ms
    log(INFO, "BME_Status: ok,  ID: BME680");
    break;
  case 280:
    log(INFO, "BME_Status: ok,  ID: BME280");
    break;
  default:
    log(INFO, "BME_Status: not found");
    break;
  }
  return (type_thp > 0);
}

bool read_thp_sensor(float *temperature, float *humidity, float *pressure) {
  if (type_thp == 280) {
    *temperature = bme280.readTemperature();
    *humidity = bme280.readHumidity();
    *pressure = bme280.readPressure();
  } else if (type_thp == 680) {
    if (!bme680.performReading()) {
      log(INFO, "BME680: Failed to perform reading");
      return false;
    }
    *temperature = bme680.temperature;
    *humidity = bme680.humidity;
    *pressure = bme680.pressure;
  } else {
    *temperature = 0.0;
    *humidity = 0.0;
    *pressure = 0.0;
  }
  return (type_thp > 0);
}

