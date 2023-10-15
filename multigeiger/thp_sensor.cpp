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
  // #include "config\generic_33v_3s_4d\bsec_iaq.txt"
  0, 8, 4, 1, 61, 0, 0, 0, 0, 0, 0, 0, 174, 1, 0, 0, 48, 0, 1, 0, 0, 192, 168, 71, 64, 49, 119, 76, 0, 0, 225, 68, 137, 65, 0, 191, 205, 204, 204, 190, 0, 0, 64, 191, 225, 122, 148, 190, 0, 0, 0, 0, 216, 85, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 28, 0, 2, 0, 0, 244, 1, 225, 0, 25, 0, 0, 128, 64, 0, 0, 32, 65, 144, 1, 0, 0, 112, 65, 0, 0, 0, 63, 16, 0, 3, 0, 10, 215, 163, 60, 10, 215, 35, 59, 10, 215, 35, 59, 9, 0, 5, 0, 0, 0, 0, 0, 1, 88, 0, 9, 0, 229, 208, 34, 62, 0, 0, 0, 0, 0, 0, 0, 0, 218, 27, 156, 62, 225, 11, 67, 64, 0, 0, 160, 64, 0, 0, 0, 0, 0, 0, 0, 0, 94, 75, 72, 189, 93, 254, 159, 64, 66, 62, 160, 191, 0, 0, 0, 0, 0, 0, 0, 0, 33, 31, 180, 190, 138, 176, 97, 64, 65, 241, 99, 190, 0, 0, 0, 0, 0, 0, 0, 0, 167, 121, 71, 61, 165, 189, 41, 192, 184, 30, 189, 64, 12, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 229, 0, 254, 0, 2, 1, 5, 48, 117, 100, 0, 44, 1, 112, 23, 151, 7, 132, 3, 197, 0, 92, 4, 144, 1, 64, 1, 64, 1, 144, 1, 48, 117, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 48, 117, 100, 0, 100, 0, 100, 0, 48, 117, 48, 117, 100, 0, 100, 0, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 44, 1, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 8, 7, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 112, 23, 255, 255, 255, 255, 255, 255, 255, 255, 220, 5, 220, 5, 220, 5, 255, 255, 255, 255, 255, 255, 220, 5, 220, 5, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 44, 1, 0, 0, 0, 0, 237, 52, 0, 0
};

Adafruit_BME280 bme280;
Bsec iaqSensor;  // Create an object of the class Bsec, based on BME680
bsec_virtual_sensor_t sensorList[10] = {  // Active virtual sensors for BSEC
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


#define BME_TEMP_CORRECTION 1  // only 1 degree as on separate PCB

static int type_thp = 0;

// Helper functions declarations
bool checkIaqSensorStatus(void);

const char *get_thp_name() {
  switch (type_thp) {
  case 0:
    return "no THP sensor";
  case 280:
    return "BME280";
  case 680:
    return "BME680_BSEC";
  default:
    return "no defined THP sensor";
  }
}

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

  // BME680 initialization
  if (type_thp == 680) {
    iaqSensor.setConfig(bsec_config_iaq);
    if (!checkIaqSensorStatus()) {
      type_thp = 0;
      log(INFO, "THP_Status: BME680 config error");
    }
  }
  if (type_thp == 680) {
    iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
    if (!checkIaqSensorStatus()) {
      type_thp = 0;
      log(INFO, "THP_Status: BME680 sensor subscription error");
    }
  }
  if (type_thp == 680) {
    iaqSensor.setTemperatureOffset(BME_TEMP_CORRECTION);
    if (!iaqSensor.run()) {
      type_thp = 0;
      log(INFO, "THP_Status: BME680 sensor readout error");
    }
  }

  log(INFO, "THP_Status: %s initialized", get_thp_name());
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
      log(INFO, "THP_Status: BME680_BSEC multiple readout errors");
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
      log(INFO, "THP_Status: BSEC error - code %d", (int)iaqSensor.status);
    else
      log(INFO, "THP_Status: BSEC warning - code %d", (int)iaqSensor.status);
    return false;
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK)
      log(INFO, "THP_Status: BME680 error - code %d", (int)iaqSensor.status);
    else
      log(INFO, "THP_Status: BME680 warning - code %d", (int)iaqSensor.status);
    Serial.println(iaqSensor.status);
    return false;
  }

  return true;
}
