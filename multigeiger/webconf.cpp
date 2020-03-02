// Web Configuration related code
// also: OTA updates

#include "log.h"

#include "IotWebConf.h"
#include <HTTPClient.h>
#include "userdefines.h"

// All these flags are ON (=1) at startup
// They all can be changed at any time using the web interface
char speakerTick[2] = "1";
char startSound[2] = "1";
char ledTick[2] = "1";
char showDisplay[2] = "1";
#if CPU==STICK
// this is LoRa hardware: don't send to WiFi servers by default
char sendToCommunity[2] = "0";
char sendToMadavi[2] = "0";
// send to LoRa by default
char sendToLora[2] = "1";
char appeui[17] = "";
char deveui[17] = "";
char appkey[IOTWEBCONF_WORD_LEN] = "";
#else
// WiFi hardware: send to wifi servers and not to LoRa
char sendToLora[2] = "0";
char sendToCommunity[2] = "1";
char sendToMadavi[2] = "1";
#endif

IotWebConfSeparator sep0 = IotWebConfSeparator("Tick settings");
IotWebConfParameter startSoundParam = IotWebConfParameter("Start sound (1=ON)", "startSound", startSound, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
IotWebConfParameter speakerTickParam = IotWebConfParameter("Speaker tick (1=ON)", "speakerTick", speakerTick, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
IotWebConfParameter ledTickParam = IotWebConfParameter("LED ticker (1=ON)", "ledTick", ledTick, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
IotWebConfParameter showDisplayParam = IotWebConfParameter("Show display (1=ON)", "showDisplay", showDisplay, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
IotWebConfSeparator sep1 = IotWebConfSeparator("Server settings");
IotWebConfParameter sendToCommunityParam = IotWebConfParameter("Send to sensors.community (1=ON)", "send2Community", sendToCommunity, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
IotWebConfParameter sendToMadaviParam = IotWebConfParameter("Send to madavi.de (1=ON)", "send2Madavi", sendToMadavi, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
#if CPU==STICK
IotWebConfSeparator sep3 = IotWebConfSeparator("LoRa settings");
IotWebConfParameter sendToLoraParam = IotWebConfParameter("Send to LoRa (=>TTN) (1=ON)", "send2lora", sendToLora, 2, "number", "0/1", NULL, "min='0' max='1' step='1'");
IotWebConfParameter appeuiParam = IotWebConfParameter("APPEUI", "appeui", appeui, 17);
IotWebConfParameter deveuiParam = IotWebConfParameter("DEVEUI", "deveui", deveui, 17);
IotWebConfParameter appkeyParam = IotWebConfParameter("APPKEY", "appkey", appkey, 33);
#endif

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

void configSaved(void) {
  log(INFO, "Config saved .. restarting");
  ESP.restart();
}

void setup_webconf() {
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setupUpdateServer(&httpUpdater);

  // override the confusing default labels of IotWebConf:
  iotWebConf.getThingNameParameter()->label = "Geiger accesspoint SSID";
  iotWebConf.getApPasswordParameter()->label = "Geiger accesspoint password";
  iotWebConf.getWifiSsidParameter()->label = "WiFi client SSID";
  iotWebConf.getWifiPasswordParameter()->label = "WiFi client password";

  // add the setting parameter
  iotWebConf.addParameter(&sep0);
  //iotWebConf.addParameter(&startSoundParam1);
  iotWebConf.addParameter(&startSoundParam);
  iotWebConf.addParameter(&speakerTickParam);
  iotWebConf.addParameter(&ledTickParam);
  iotWebConf.addParameter(&showDisplayParam);
  iotWebConf.addParameter(&sep1);
  iotWebConf.addParameter(&sendToCommunityParam);
  iotWebConf.addParameter(&sendToMadaviParam);
  #if CPU==STICK
  iotWebConf.addParameter(&sep3);
  iotWebConf.addParameter(&sendToLoraParam);
  iotWebConf.addParameter(&appeuiParam);
  iotWebConf.addParameter(&deveuiParam);
  iotWebConf.addParameter(&appkeyParam);
  #endif


  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", [] { iotWebConf.handleConfig(); });
  server.onNotFound([]() {
    iotWebConf.handleNotFound();
  });
}

// Check value of settings parameters
boolean parameterTrue(char *parameter) {
  return (strcmp(parameter, "1") == 0);
}




