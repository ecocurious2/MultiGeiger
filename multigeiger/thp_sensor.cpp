// temperature, humidity, pressure sensor (usually a BME280) related code

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include "log.h"
#include "thp_sensor.h"

#include <Adafruit_BME280.h>
/*
 Bosch BSEC Lib, https://github.com/BoschSensortec/BSEC-Arduino-library
 The BSEC software is only available for download or use after accepting the software license agreement.
 By using this library, you have agreed to the terms of the license agreement:
 https://ae-bst.resource.bosch.com/media/_tech/media/bsec/2017-07-17_ClickThrough_License_Terms_Environmentalib_SW_CLEAN.pdf */
#include <bsec.h>

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config\generic_33v_3s_4d\bsec_iaq.txt"
};

// Helper functions declarations
bool checkIaqSensorStatus(void);

Bsec iaqSensor;     // Create an object of the class Bsec
#define BME_TEMP_CORRECTION 1

static int type_thp = 0;

Adafruit_BME280 bme280;

bool setup_thp_sensor(void) {


  // BME280
  if (bme280.begin(BME280_ADDRESS))
    type_thp = 280;
  else if (bme280.begin(BME280_ADDRESS_ALTERNATE))
    type_thp = 280;

  // BME680
  if (type_thp == 0) {
    iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
    if (checkIaqSensorStatus())
      type_thp = 680;
    else {
      iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
      if (checkIaqSensorStatus())
        type_thp = 680;
    }
  }

  switch (type_thp) {
  case 680: {
    iaqSensor.setConfig(bsec_config_iaq);
    if (!checkIaqSensorStatus()) {
      type_thp = 0;
      log(INFO, "BME_Status: BME680 config error");
      break;
    }
    bsec_virtual_sensor_t sensorList[10] = {
      BSEC_OUTPUT_RAW_TEMPERATURE,
      BSEC_OUTPUT_RAW_PRESSURE,
      BSEC_OUTPUT_RAW_HUMIDITY,
      BSEC_OUTPUT_RAW_GAS,
      BSEC_OUTPUT_IAQ,
      BSEC_OUTPUT_STATIC_IAQ,
      BSEC_OUTPUT_CO2_EQUIVALENT,
      BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    };
    iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
    if (!checkIaqSensorStatus()) {
      type_thp = 0;
      log(INFO, "BME_Status: BME680 sensor subscription error");
      break;
    }
    iaqSensor.setTemperatureOffset(BME_TEMP_CORRECTION);
    if (!iaqSensor.run()) {
      type_thp = 0;
      log(INFO, "BME_Status: BME680 sensor readout error");
      break;
    }
    log(INFO, "BME_Status: ok,  ID: BME680_BSEC");
    break;
  }
  case 280:
    log(INFO, "BME_Status: ok,  ID: BME280");
    break;
  default:
    log(INFO, "BME_Status: not found");
    break;
  }
  return (type_thp > 0);
}

bool read_thp_sensor(float *temperature, float *humidity, float *pressure, int *iaq) {
  static int bsec_failcount = 0;
  if (type_thp == 280) {
    *temperature = bme280.readTemperature();
    *humidity = bme280.readHumidity();
    *pressure = bme280.readPressure();
    *iaq = 0;
  } else if (type_thp == 680) {
    if (!iaqSensor.run())
      bsec_failcount++;
    else
      bsec_failcount = 0;
    if (bsec_failcount > 10) {
      log(INFO, "BME680_BSEC: Multiple readout errors");
      return false;
    }
    *temperature = iaqSensor.temperature;
    *humidity = iaqSensor.humidity;
    *pressure = iaqSensor.pressure;
    *iaq = iaqSensor.iaq;
  } else {
    *temperature = 0.0;
    *humidity = 0.0;
    *pressure = 0.0;
    *iaq = 0;
  }
  return (type_thp > 0);
}

bool checkIaqSensorStatus(void) {
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK)
      Serial.print("BSEC error code : ");
    else
      Serial.print("BSEC warning code : ");
    Serial.println(iaqSensor.status);
    return false;
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK)
      Serial.print("BME680 error code : ");
    else
      Serial.print("BME680 warning code : ");
    Serial.println(iaqSensor.status);
    return false;
  }

  return true;
}
