// Web Configuration related code
// also: OTA updates

#include "log.h"

#include "IotWebConf.h"
#include <HTTPClient.h>
#include "userdefines.h"

bool speakerTick = SPEAKER_TICK;
bool playSound = PLAY_SOUND;
bool ledTick = LED_TICK;
bool showDisplay = SHOW_DISPLAY;
bool sendToCommunity = SEND2SENSORCOMMUNITY;
bool sendToMadavi = SEND2MADAVI;
bool sendToLora = SEND2LORA;

char speakerTick_c[2];
char playSound_c[2];
char ledTick_c[2];
char showDisplay_c[2];
char sendToCommunity_c[2];
char sendToMadavi_c[2];
char sendToLora_c[2];

#if CPU==STICK
char appeui[17] = "";
char deveui[17] = "";
char appkey[IOTWEBCONF_WORD_LEN] = "";
#endif

#define BOOL_PARAM(label, id, var) IotWebConfParameter(label " (1 == true, 0 == false)", id, var, 2, "number", "0/1", NULL, "min='0' max='1' step='1'")

IotWebConfSeparator sep0 = IotWebConfSeparator("Misc. settings");
IotWebConfParameter startSoundParam = BOOL_PARAM("Start sound", "startSound", playSound_c);
IotWebConfParameter speakerTickParam = BOOL_PARAM("Speaker tick", "speakerTick", speakerTick_c);
IotWebConfParameter ledTickParam = BOOL_PARAM("LED tick", "ledTick", ledTick_c);
IotWebConfParameter showDisplayParam = BOOL_PARAM("Show display", "showDisplay", showDisplay_c);

IotWebConfSeparator sep1 = IotWebConfSeparator("Transmission settings");
IotWebConfParameter sendToCommunityParam = BOOL_PARAM("Send to sensors.community", "send2Community", sendToCommunity_c);
IotWebConfParameter sendToMadaviParam = BOOL_PARAM("Send to madavi.de", "send2Madavi", sendToMadavi_c);

#if CPU==STICK
IotWebConfSeparator sep2 = IotWebConfSeparator("LoRa settings");
IotWebConfParameter sendToLoraParam = BOOL_PARAM("Send to LoRa (=>TTN)", "send2lora", sendToLora_c);
IotWebConfParameter deveuiParam = IotWebConfParameter("DEVEUI", "deveui", deveui, 17);
IotWebConfParameter appeuiParam = IotWebConfParameter("APPEUI", "appeui", appeui, 17);
IotWebConfParameter appkeyParam = IotWebConfParameter("APPKEY", "appkey", appkey, 33);
#endif

bool parse_bool(char *text, bool *value) {
  if (!strcmp(text, "0")) {
    *value = false;
    return true;
  }
  if (!strcmp(text, "1")) {
    *value = true;
    return true;
  }
  return false;  // invalid
}

void format_bool(bool *value, char *text) {
  strcpy(text, *value ? "1" : "0");
}

#define CONFIG_VERSION "013"  // for IoTWebConfig

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;

char *buildSSID(void);

// SSID == thingName
const char *theName = buildSSID();
char ssid[IOTWEBCONF_WORD_LEN];  // LEN == 33 (2020-01-13)

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "ESP32Geiger";

IotWebConf iotWebConf(theName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

unsigned long getESPchipID() {
  uint64_t espid = ESP.getEfuseMac();
  uint8_t *pespid = (uint8_t *)&espid;
  uint32_t id = 0;
  uint8_t *pid = (uint8_t *)&id;
  pid[0] = (uint8_t)pespid[5];
  pid[1] = (uint8_t)pespid[4];
  pid[2] = (uint8_t)pespid[3];
  log(INFO, "ID: %08X", id);
  log(INFO, "MAC: %04X%08X", (uint16_t)(espid >> 32), (uint32_t)espid);
  return id;
}

char *buildSSID() {
  // build SSID from ESP chip id
  uint32_t id = getESPchipID();
  sprintf(ssid, "ESP32-%d", id);
  return ssid;
}

void handleRoot(void) {  // Handle web requests to "/" path.
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal()) {
    // -- Captive portal requests were already served.
    return;
  }
  const char *index =
    "<!DOCTYPE html>"
    "<html lang='en'>"
    "<head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no' />"
    "<title>MultiGeiger Configuration</title>"
    "</head>"
    "<body>"
    "<h1>Configuration</h1>"
    "<p>"
    "Go to the <a href='config'>configure page</a> to change settings or update firmware."
    "</p>"
    "</body>"
    "</html>\n";
  server.send(200, "text/html;charset=UTF-8", index);
}

static char lastWiFiSSID[IOTWEBCONF_WORD_LEN] = "";

void loadConfigVariables(void) {
  // check if WiFi SSID has changed. If so, restart cpu. Otherwise, the program will not use the new SSID
  if ((strcmp(lastWiFiSSID, "") != 0) && (strcmp(lastWiFiSSID, iotWebConf.getWifiSsidParameter()->valueBuffer) != 0)) {
    log(INFO, "Doing restart...");
    ESP.restart();
  }
  strcpy(lastWiFiSSID, iotWebConf.getWifiSsidParameter()->valueBuffer);

  parse_bool(speakerTick_c, &speakerTick);
  parse_bool(playSound_c, &playSound);
  parse_bool(ledTick_c, &ledTick);
  parse_bool(showDisplay_c, &showDisplay);
  parse_bool(sendToCommunity_c, &sendToCommunity);
  parse_bool(sendToMadavi_c, &sendToMadavi);
  parse_bool(sendToLora_c, &sendToLora);
}

void configSaved(void) {
  log(INFO, "Config saved. ");
  loadConfigVariables();
}

void initConfigVariables(void) {
  format_bool(&speakerTick, speakerTick_c);
  format_bool(&playSound, playSound_c);
  format_bool(&ledTick, ledTick_c);
  format_bool(&showDisplay, showDisplay_c);
  format_bool(&sendToCommunity, sendToCommunity_c);
  format_bool(&sendToMadavi, sendToMadavi_c);
  format_bool(&sendToLora, sendToLora_c);
}

void setup_webconf() {
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setupUpdateServer(&httpUpdater);

  // override the confusing default labels of IotWebConf:
  iotWebConf.getThingNameParameter()->label = "Geiger accesspoint SSID";
  iotWebConf.getApPasswordParameter()->label = "Geiger accesspoint password";
  iotWebConf.getWifiSsidParameter()->label = "WiFi client SSID";
  iotWebConf.getWifiPasswordParameter()->label = "WiFi client password";

  // fill parameters with userdefines.h values
  initConfigVariables();

  // add the setting parameter
  iotWebConf.addParameter(&sep0);
  iotWebConf.addParameter(&startSoundParam);
  iotWebConf.addParameter(&speakerTickParam);
  iotWebConf.addParameter(&ledTickParam);
  iotWebConf.addParameter(&showDisplayParam);
  iotWebConf.addParameter(&sep1);
  iotWebConf.addParameter(&sendToCommunityParam);
  iotWebConf.addParameter(&sendToMadaviParam);
  #if CPU==STICK
  iotWebConf.addParameter(&sep2);
  iotWebConf.addParameter(&sendToLoraParam);
  iotWebConf.addParameter(&deveuiParam);
  iotWebConf.addParameter(&appeuiParam);
  iotWebConf.addParameter(&appkeyParam);
  #endif

  // if we don't have LoRa hardware, do not send to LoRa
  #if CPU==WIFI
  sendToLora = false;
  #endif

  iotWebConf.init();

  loadConfigVariables();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", [] { iotWebConf.handleConfig(); });
  server.onNotFound([]() {
    iotWebConf.handleNotFound();
  });
}
