// Web Configuration related code
// also: OTA updates

#include "log.h"
#include "speaker.h"

#include "IotWebConf.h"
#include "IotWebConfTParameter.h"
#include <IotWebConfESP32HTTPUpdateServer.h>
#include "userdefines.h"

// Checkboxes have 'selected' if checked, so we need 9 byte for this string.
#define CHECKBOX_LEN 9

bool speakerTick = SPEAKER_TICK;
bool playSound = PLAY_SOUND;
bool ledTick = LED_TICK;
bool showDisplay = SHOW_DISPLAY;
bool sendToCommunity = SEND2SENSORCOMMUNITY;
bool sendToMadavi = SEND2MADAVI;
bool sendToLora = SEND2LORA;
bool sendToBle = SEND2BLE;
bool soundLocalAlarm = LOCAL_ALARM_SOUND;

char speakerTick_c[CHECKBOX_LEN];
char playSound_c[CHECKBOX_LEN];
char ledTick_c[CHECKBOX_LEN];
char showDisplay_c[CHECKBOX_LEN];
char sendToCommunity_c[CHECKBOX_LEN];
char sendToMadavi_c[CHECKBOX_LEN];
char sendToLora_c[CHECKBOX_LEN];
char sendToBle_c[CHECKBOX_LEN];
char soundLocalAlarm_c[CHECKBOX_LEN];

char appeui[17] = "";
char deveui[17] = "";
char appkey[IOTWEBCONF_WORD_LEN] = "";
static bool isLoraBoard;

float localAlarmThreshold = LOCAL_ALARM_THRESHOLD;
int localAlarmFactor = (int)LOCAL_ALARM_FACTOR;

int sendDataToMessengerEvery = (int)SEND_DATA_TO_MESSENGER_EVERY;
bool sendLocalAlarmToMessenger = SEND_LOCAL_ALARM_TO_MESSENGER;
char telegramBotToken[50] = "";  // "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
char telegramChatId[12] = "";  // "1234567890"

char sendLocalAlarmToMessenger_c[CHECKBOX_LEN];

iotwebconf::ParameterGroup grpMisc = iotwebconf::ParameterGroup("misc", "Misc. Settings");
iotwebconf::CheckboxParameter startSoundParam = iotwebconf::CheckboxParameter("Start sound", "startSound", playSound_c, CHECKBOX_LEN, playSound);
iotwebconf::CheckboxParameter speakerTickParam = iotwebconf::CheckboxParameter("Speaker tick", "speakerTick", speakerTick_c, CHECKBOX_LEN, speakerTick);
iotwebconf::CheckboxParameter ledTickParam = iotwebconf::CheckboxParameter("LED tick", "ledTick", ledTick_c, CHECKBOX_LEN, ledTick);
iotwebconf::CheckboxParameter showDisplayParam = iotwebconf::CheckboxParameter("Show display", "showDisplay", showDisplay_c, CHECKBOX_LEN, showDisplay);

iotwebconf::ParameterGroup grpTransmission = iotwebconf::ParameterGroup("transmission", "Transmission Settings");
iotwebconf::CheckboxParameter sendToCommunityParam = iotwebconf::CheckboxParameter("Send to sensor.community", "send2Community", sendToCommunity_c, CHECKBOX_LEN, sendToCommunity);
iotwebconf::CheckboxParameter sendToMadaviParam = iotwebconf::CheckboxParameter("Send to madavi.de", "send2Madavi", sendToMadavi_c, CHECKBOX_LEN, sendToMadavi);
iotwebconf::CheckboxParameter sendToBleParam = iotwebconf::CheckboxParameter("Send to BLE (Reboot required!)", "send2ble", sendToBle_c, CHECKBOX_LEN, sendToBle);

iotwebconf::ParameterGroup grpLoRa = iotwebconf::ParameterGroup("lora", "LoRa Settings");
iotwebconf::CheckboxParameter sendToLoraParam = iotwebconf::CheckboxParameter("Send to LoRa (=>TTN)", "send2lora", sendToLora_c, CHECKBOX_LEN, sendToLora);
iotwebconf::TextParameter deveuiParam = iotwebconf::TextParameter("DEVEUI", "deveui", deveui, 17);
iotwebconf::TextParameter appeuiParam = iotwebconf::TextParameter("APPEUI", "appeui", appeui, 17);
iotwebconf::TextParameter appkeyParam = iotwebconf::TextParameter("APPKEY", "appkey", appkey, 33);

iotwebconf::ParameterGroup grpAlarm = iotwebconf::ParameterGroup("alarm", "Local Alarm Settings");
iotwebconf::CheckboxParameter soundLocalAlarmParam = iotwebconf::CheckboxParameter("Enable local alarm sound", "soundLocalAlarm", soundLocalAlarm_c, CHECKBOX_LEN, soundLocalAlarm);
iotwebconf::FloatTParameter localAlarmThresholdParam =
  iotwebconf::Builder<iotwebconf::FloatTParameter>("localAlarmThreshold").
  label("Local alarm threshold (µSv/h)").
  defaultValue(localAlarmThreshold).
  step(0.1).placeholder("e.g. 0.5").build();
iotwebconf::IntTParameter<int16_t> localAlarmFactorParam =
  iotwebconf::Builder<iotwebconf::IntTParameter<int16_t>>("localAlarmFactor").
  label("Factor of current dose rate vs. accumulated").
  defaultValue(localAlarmFactor).
  min(2).max(100).
  step(1).placeholder("2..100").build();

iotwebconf::ParameterGroup grpMessenger = iotwebconf::ParameterGroup("messenger", "Messenger Settings");
iotwebconf::CheckboxParameter sendLocalAlarmToMessengerParam = iotwebconf::CheckboxParameter("Send local alarm via Messenger", "sendLocalAlarmToMessenger", sendLocalAlarmToMessenger_c, CHECKBOX_LEN, sendLocalAlarmToMessenger);
iotwebconf::IntTParameter<int16_t> sendDataToMessengerEveryParam =
  iotwebconf::Builder<iotwebconf::IntTParameter<int16_t>>("sendDataToMessengerEvery").
  label("Send data via Messenger every N x 2.5min\n(0=never,24=1/h,576=1/d,4032=1/week,max:27719)").
  defaultValue(sendDataToMessengerEvery).
  min(0).max(27719).
  step(1).placeholder("0..27719").build();
// iotwebconf::TextParameter telegramBotTokenParam = iotwebconf::TextParameter("Telegram Bot Token", "telegramBotToken", telegramBotToken, 50);
iotwebconf::PasswordParameter telegramBotTokenParam = iotwebconf::PasswordParameter("Telegram Bot Token", "telegramBotToken", telegramBotToken, 50);
iotwebconf::PasswordParameter telegramChatIdParam = iotwebconf::PasswordParameter("Telegram Chat ID", "telegramChatId", telegramChatId, 12);


// This only needs to be changed if the layout of the configuration is changed.
// Appending new variables does not require a new version number here.
// If this value is changed, ALL configuration variables must be re-entered,
// including the WiFi credentials.
#define CONFIG_VERSION "015"

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
    "Go to the <a href='config'>config page</a> to change settings or update firmware."
    "</p>"
    "</body>"
    "</html>\n";
  server.send(200, "text/html;charset=UTF-8", index);
  // looks like user wants to do some configuration or maybe flash firmware.
  // while accessing the flash, we need to turn ticking off to avoid exceptions.
  // user needs to save the config (or flash firmware + reboot) to turn it on again.
  // note: it didn't look like there is an easy way to put this call at the right place
  // (start of fw flash / start of config save) - this is why it is here.
  tick_enable(false);
}

static char lastWiFiSSID[IOTWEBCONF_WORD_LEN] = "";

void loadConfigVariables(void) {
  // check if WiFi SSID has changed. If so, restart cpu. Otherwise, the program will not use the new SSID
  if ((strcmp(lastWiFiSSID, "") != 0) && (strcmp(lastWiFiSSID, iotWebConf.getWifiSsidParameter()->valueBuffer) != 0)) {
    log(INFO, "Doing restart...");
    ESP.restart();
  }
  strcpy(lastWiFiSSID, iotWebConf.getWifiSsidParameter()->valueBuffer);

  speakerTick = speakerTickParam.isChecked();
  playSound = startSoundParam.isChecked();
  ledTick = ledTickParam.isChecked();
  showDisplay = showDisplayParam.isChecked();
  sendToCommunity = sendToCommunityParam.isChecked();
  sendToMadavi = sendToMadaviParam.isChecked();
  sendToLora = sendToLoraParam.isChecked();
  sendToBle = sendToBleParam.isChecked();
  soundLocalAlarm = soundLocalAlarmParam.isChecked();
  localAlarmThreshold = localAlarmThresholdParam.value();
  localAlarmFactor = localAlarmFactorParam.value();
  sendDataToMessengerEvery = sendDataToMessengerEveryParam.value();
  sendLocalAlarmToMessenger = sendLocalAlarmToMessengerParam.isChecked();
}

void configSaved(void) {
  log(INFO, "Config saved. ");
  loadConfigVariables();
  tick_enable(true);
}

void setup_webconf(bool loraHardware) {
  isLoraBoard = loraHardware;
  iotWebConf.setConfigSavedCallback(&configSaved);
  // *INDENT-OFF*   <- for 'astyle' to not format the following 3 lines
  iotWebConf.setupUpdateServer(
    [](const char *updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char *userName, char *password) { httpUpdater.updateCredentials(userName, password); });
  // *INDENT-ON*
  // override the confusing default labels of IotWebConf:
  iotWebConf.getThingNameParameter()->label = "Geiger accesspoint SSID";
  iotWebConf.getApPasswordParameter()->label = "Geiger accesspoint password";
  iotWebConf.getWifiSsidParameter()->label = "WiFi client SSID";
  iotWebConf.getWifiPasswordParameter()->label = "WiFi client password";

  // add the setting parameter
  grpMisc.addItem(&startSoundParam);
  grpMisc.addItem(&speakerTickParam);
  grpMisc.addItem(&ledTickParam);
  grpMisc.addItem(&showDisplayParam);
  iotWebConf.addParameterGroup(&grpMisc);
  grpTransmission.addItem(&sendToCommunityParam);
  grpTransmission.addItem(&sendToMadaviParam);
  grpTransmission.addItem(&sendToBleParam);
  iotWebConf.addParameterGroup(&grpTransmission);
  if (isLoraBoard) {
    grpLoRa.addItem(&sendToLoraParam);
    grpLoRa.addItem(&deveuiParam);
    grpLoRa.addItem(&appeuiParam);
    grpLoRa.addItem(&appkeyParam);
    iotWebConf.addParameterGroup(&grpLoRa);
  }
  grpAlarm.addItem(&soundLocalAlarmParam);
  grpAlarm.addItem(&localAlarmThresholdParam);
  grpAlarm.addItem(&localAlarmFactorParam);
  iotWebConf.addParameterGroup(&grpAlarm);
  grpMessenger.addItem(&sendDataToMessengerEveryParam);
  grpMessenger.addItem(&sendLocalAlarmToMessengerParam);
  grpMessenger.addItem(&telegramBotTokenParam);
  grpMessenger.addItem(&telegramChatIdParam);
  iotWebConf.addParameterGroup(&grpMessenger);

  // if we don't have LoRa hardware, do not send to LoRa
  if (!isLoraBoard)
    sendToLora = false;

  // if we don't have a valid Messenger config, do not send to Messenger
  if ((sizeof(telegramBotToken) < 40) || (sizeof(telegramChatId) < 7))
    sendDataToMessengerEvery = -1;

  iotWebConf.init();

  loadConfigVariables();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", [] { iotWebConf.handleConfig(); });
  server.onNotFound([]() {
    iotWebConf.handleNotFound();
  });
}
