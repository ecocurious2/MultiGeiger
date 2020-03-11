// measurements data transmission related code
// - via WiFi to internet servers
// - via LoRa to TTN (to internet servers)

#include <Arduino.h>
#include <HTTPClient.h>

#include "log.h"
#include "display.h"
#include "userdefines.h"
#include "webconf.h"

#if CPU==STICK
#include "loraWan.h"
#endif

#include "transmission.h"

// Hosts for data delivery
#define MADAVI "http://api-rrd.madavi.de/data.php"
#define SENSORCOMMUNITY "http://api.sensor.community/v1/push-sensor-data/"
#define TOILET "http://ptsv2.com/t/rk9pr-1582220446/post"

static String http_software_version;
#if CPU==STICK
static unsigned int lora_software_version;
#endif
static String chipID;

void setup_transmission(const char *version, char *ssid) {
  chipID = String(ssid);
  chipID.replace("ESP32", "esp32");

  http_software_version = String(version);

  #if CPU==STICK
  int major, minor, patch;
  sscanf(version, "V%d.%d.%d", &major, &minor, &patch);
  lora_software_version = (major << 12) + (minor << 4) + patch;
  setup_lorawan();
  #endif
}

void prepare_http(HTTPClient *http, const char *host) {
  http->begin(host);
  http->addHeader("Content-Type", "application/json; charset=UTF-8");
  http->addHeader("Connection", "close");
  http->addHeader("X-Sensor", chipID);
}

void send_http(HTTPClient *http, String body) {
  if (DEBUG_SERVER_SEND == 1)
    log(DEBUG, "http request body: %s", body.c_str());

  int httpResponseCode = http->POST(body);
  if (httpResponseCode > 0) {
    String response = http->getString();
    if (DEBUG_SERVER_SEND == 1) {
      log(DEBUG, "http code: %d", httpResponseCode);
      log(DEBUG, "http response: %s", response.c_str());
    }
  } else {
    log(ERROR, "Error on sending POST: %d", httpResponseCode);
  }
  http->end();
}

void send_http_geiger(const char *host, unsigned int timediff, unsigned int hv_pulses,
                      unsigned int gm_counts, unsigned int cpm, int xpin) {
  char body[1000];
  HTTPClient http;
  prepare_http(&http, host);
  if (xpin != XPIN_NO_XPIN) {
    http.addHeader("X-PIN", String(xpin));
  }
  const char *json_format = R"=====(
{
 "software_version": "%s",
 "sensordatavalues": [
  {"value_type": "counts_per_minute", "value": "%d"},
  {"value_type": "hv_pulses", "value": "%d"},
  {"value_type": "counts", "value": "%d"},
  {"value_type": "sample_time_ms", "value": "%d"}
 ]
}
)=====";
  snprintf(body, 1000, json_format,
           http_software_version.c_str(),
           cpm,
           hv_pulses,
           gm_counts,
           timediff);
  send_http(&http, body);
}

void send_http_thp(const char *host, float temperature, float humidity, float pressure, int xpin) {
  char body[1000];
  HTTPClient http;
  prepare_http(&http, host);
  if(xpin != XPIN_NO_XPIN) {
    http.addHeader("X-PIN", String(xpin));
  }
  const char *json_format = R"=====(
{
 "software_version": "%s",
 "sensordatavalues": [
  {"value_type": "temperature", "value": "%.2f"},
  {"value_type": "humidity", "value": "%.2f"},
  {"value_type": "pressure", "value": "%.2f"}
 ]
}
)=====";
  snprintf(body, 1000, json_format,
           http_software_version.c_str(),
           temperature,
           humidity,
           pressure);
  send_http(&http, body);
}

// two extra functions for MADAVI, because MADAVI needs the sensorname in value_type to recognize the sensors
void send_http_geiger_2_madavi(String tube_type, unsigned int timediff, unsigned int hv_pulses,
                      unsigned int gm_counts, unsigned int cpm) {
  char body[1000];
  HTTPClient http;
  prepare_http(&http, MADAVI);
  tube_type = tube_type.substring(10);
  const char *json_format = R"=====(
{
 "software_version": "%s",
 "sensordatavalues": [
  {"value_type": "%s_counts_per_minute", "value": "%d"},
  {"value_type": "%s_hv_pulses", "value": "%d"},
  {"value_type": "%s_counts", "value": "%d"},
  {"value_type": "%s_sample_time_ms", "value": "%d"}
 ]
}
)=====";
  snprintf(body, 1000, json_format,
           http_software_version.c_str(),
           tube_type.c_str(), cpm,
           tube_type.c_str(), hv_pulses,
           tube_type.c_str(), gm_counts,
           tube_type.c_str(), timediff);
  send_http(&http, body);
}

void send_http_thp_2_madavi(float temperature, float humidity, float pressure) {
  char body[1000];
  HTTPClient http;
  prepare_http(&http, MADAVI);
  const char *json_format = R"=====(
{
 "software_version": "%s",
 "sensordatavalues": [
  {"value_type": "BME280_temperature", "value": "%.2f"},
  {"value_type": "BME280_humidity", "value": "%.2f"},
  {"value_type": "BME280_pressure", "value": "%.2f"}
 ]
}
)=====";
  snprintf(body, 1000, json_format,
           http_software_version.c_str(),
           temperature,
           humidity,
           pressure);
  send_http(&http, body);
}

#if CPU==STICK
// LoRa payload:
// To minimise airtime and follow the 'TTN Fair Access Policy', we only send necessary bytes.
// We do NOT use Cayenne LPP.
// The payload will be translated via http integration and a small program to be compatible with sensor.community.
// For byte definitions see ttn2luft.pdf in docs directory.
void send_ttn_geiger(int tube_nbr, unsigned int dt, unsigned int gm_counts) {
  unsigned char ttnData[10];
  // first the number of GM counts
  ttnData[0] = (gm_counts >> 24) & 0xFF;
  ttnData[1] = (gm_counts >> 16) & 0xFF;
  ttnData[2] = (gm_counts >> 8) & 0xFF;
  ttnData[3] = gm_counts & 0xFF;
  // now 3 bytes for the measurement interval [in ms] (max ca. 4 hours)
  ttnData[4] = (dt >> 16) & 0xFF;
  ttnData[5] = (dt >> 8) & 0xFF;
  ttnData[6] = dt & 0xFF;
  // next two bytes are software version
  ttnData[7] = (lora_software_version >> 8) & 0xFF;
  ttnData[8] = lora_software_version & 0xFF;
  // next byte is the tube number
  ttnData[9] = tube_nbr;
  lorawan_send(1, ttnData, 10, false, NULL, NULL, NULL);
}

void send_ttn_thp(float temperature, float humidity, float pressure) {
  unsigned char ttnData[5];
  ttnData[0] = ((int)(temperature * 10)) >> 8;
  ttnData[1] = ((int)(temperature * 10)) & 0xFF;
  ttnData[2] = (int)(humidity * 2);
  ttnData[3] = ((int)(pressure / 10)) >> 8;
  ttnData[4] = ((int)(pressure / 10)) & 0xFF;
  lorawan_send(2, ttnData, 5, false, NULL, NULL, NULL);
}
#endif

void transmit_data(String tube_type, int tube_nbr, unsigned int dt, unsigned int hv_pulses, unsigned int gm_counts, unsigned int cpm,
                   int have_thp, float temperature, float humidity, float pressure) {

  #if SEND2DUMMY
  displayStatusLine("Toilet");
  log(INFO, "SENDING TO TOILET ...");
  send_http_geiger(TOILET, dt, hv_pulses, gm_counts, cpm, XPIN_NO_XPIN);
  if (have_thp) {
    send_http_thp(TOILET, temperature, humidity, pressure, XPIN_NO_XPIN);
  }
  delay(300);
  #endif

  if(sendToMadavi) {
    log(INFO, "Sending to Madavi ...");
    displayStatusLine("Madavi");
    send_http_geiger_2_madavi(tube_type, dt, hv_pulses, gm_counts, cpm);
    if (have_thp) {
      send_http_thp_2_madavi(temperature, humidity, pressure);
    }
    delay(300);
  }

  if(sendToCommunity) {
    log(INFO, "Sending to sensor.community ...");
    displayStatusLine("sensor.community");
    send_http_geiger(SENSORCOMMUNITY, dt, hv_pulses, gm_counts, cpm, XPIN_RADIATION)  ;
    if (have_thp) {
      send_http_thp(SENSORCOMMUNITY, temperature, humidity, pressure, XPIN_BME280);
    }
    delay(300);
  }

  #if CPU==STICK
  if(sendToLora && (strcmp(appeui,"") != 0)) { // send only, if we have LoRa credentials
    log(INFO, "Sending to TTN ...");
    displayStatusLine("TTN");
    send_ttn_geiger(tube_nbr, dt, gm_counts);
    if (have_thp)
      send_ttn_thp(temperature, humidity, pressure);
  }
  #endif

  displayStatusLine(" ");
}

