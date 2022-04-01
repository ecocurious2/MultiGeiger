// Web Configuration related code
// also: OTA updates
#include "version.h"
#include "log.h"
#include "speaker.h"
#include "transl.h"

#include "IotWebConf.h"
#include "IotWebConfTParameter.h"
#include <IotWebConfESP32HTTPUpdateServer.h>
#include "userdefines.h"
#include "utils.h"
#include "tube.h"
#include "webconf.h"

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
bool sendToInflux = SEND2INFLUX;
bool soundLocalAlarm = LOCAL_ALARM_SOUND;

char speakerTick_c[CHECKBOX_LEN];
char playSound_c[CHECKBOX_LEN];
char ledTick_c[CHECKBOX_LEN];
char showDisplay_c[CHECKBOX_LEN];
char sendToCommunity_c[CHECKBOX_LEN];
char sendToMadavi_c[CHECKBOX_LEN];
char sendToLora_c[CHECKBOX_LEN];
char sendToInflux_c[CHECKBOX_LEN];
char sendToBle_c[CHECKBOX_LEN];
char soundLocalAlarm_c[CHECKBOX_LEN];

char appeui[17] = "";
char deveui[17] = "";
char appkey[IOTWEBCONF_WORD_LEN] = "";
static bool isLoraBoard;

float localAlarmThreshold = LOCAL_ALARM_THRESHOLD;
int localAlarmFactor = (int)LOCAL_ALARM_FACTOR;

uint16_t influxPort = INFLUX_PORT;

char influxServer[100] = "";
char influxPath[100] = "";
char influxUser[65] = "";
char influxPassword[IOTWEBCONF_PASSWORD_LEN+1] = "";
char influxMeasurement[100] = "";

extern float Count_Rate;
extern float Dose_Rate;
extern unsigned long starttime;
extern int log_level;
extern float temperature;
extern float humidity;
extern float pressure;


iotwebconf::ParameterGroup grpMisc = iotwebconf::ParameterGroup("misc", "Misc. Settings");
iotwebconf::CheckboxParameter startSoundParam = iotwebconf::CheckboxParameter("Start sound", "startSound", playSound_c, CHECKBOX_LEN, playSound);
iotwebconf::CheckboxParameter speakerTickParam = iotwebconf::CheckboxParameter("Speaker tick", "speakerTick", speakerTick_c, CHECKBOX_LEN, speakerTick);
iotwebconf::CheckboxParameter ledTickParam = iotwebconf::CheckboxParameter("LED tick", "ledTick", ledTick_c, CHECKBOX_LEN, ledTick);
iotwebconf::CheckboxParameter showDisplayParam = iotwebconf::CheckboxParameter("Show display", "showDisplay", showDisplay_c, CHECKBOX_LEN, showDisplay);

iotwebconf::ParameterGroup grpTransmission = iotwebconf::ParameterGroup("transmission", "Transmission Settings");
iotwebconf::CheckboxParameter sendToCommunityParam = iotwebconf::CheckboxParameter("Send to sensor.community", "send2Community", sendToCommunity_c, CHECKBOX_LEN, sendToCommunity);
iotwebconf::CheckboxParameter sendToMadaviParam = iotwebconf::CheckboxParameter("Send to madavi.de", "send2Madavi", sendToMadavi_c, CHECKBOX_LEN, sendToMadavi);
iotwebconf::CheckboxParameter sendToBleParam = iotwebconf::CheckboxParameter("Send to BLE (Reboot required!)", "send2ble", sendToBle_c, CHECKBOX_LEN, sendToBle);
iotwebconf::CheckboxParameter sendToInfluxParam = iotwebconf::CheckboxParameter("Send to influx-db", "send2Influx", sendToInflux_c, CHECKBOX_LEN, sendToInflux);
// influx-db parameters
iotwebconf::ParameterGroup grpInfluxDB = iotwebconf::ParameterGroup("influxdb", "Influx-DB Settings");
iotwebconf::TextParameter influxServerParam = iotwebconf::TextParameter("Server", "influxserver", influxServer, 99,'\0',"influx-Server name");
iotwebconf::TextParameter influxPathParam = iotwebconf::TextParameter("Path", "influxpath", influxPath, 99,'\0',"e.g. /write?db=myInfluxDB&precision=s");
iotwebconf::IntTParameter<uint16_t> influxPortParam =
  iotwebconf::Builder<iotwebconf::IntTParameter<uint16_t>>("influxPort").
  label("Port").
  defaultValue(influxPort).
  min(1).max(65535).
  step(1).placeholder("1..65535").build();
iotwebconf::TextParameter influxUserParam = iotwebconf::TextParameter("User", "influxuser", influxUser, 64,'\0',"Username");
iotwebconf::PasswordParameter influxPasswordParam = iotwebconf::PasswordParameter("Password", "influxpassword", influxPassword, IOTWEBCONF_PASSWORD_LEN,'\0',"Password");
iotwebconf::TextParameter influxMeasurementParam = iotwebconf::TextParameter("Measurement", "influxmeasurement", influxMeasurement, 99,'\0',"Measurement");

iotwebconf::ParameterGroup grpLoRa = iotwebconf::ParameterGroup("lora", "LoRa Settings");
iotwebconf::CheckboxParameter sendToLoraParam = iotwebconf::CheckboxParameter("Send to LoRa (=>TTN)", "send2lora", sendToLora_c, CHECKBOX_LEN, sendToLora);
iotwebconf::TextParameter deveuiParam = iotwebconf::TextParameter("DEVEUI", "deveui", deveui, 17,'\0',"Device EUI");
iotwebconf::TextParameter appeuiParam = iotwebconf::TextParameter("APPEUI", "appeui", appeui, 17,'\0',"Application EUI");
iotwebconf::TextParameter appkeyParam = iotwebconf::TextParameter("APPKEY", "appkey", appkey, 33,'\0',"App Key");

iotwebconf::ParameterGroup grpAlarm = iotwebconf::ParameterGroup("alarm", "Local Alarm Setting");
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
  /* removed, is shown now in the web header, MAC is wrong/reverted here, by the way  */
  //log(INFO, "ID: %08X", id);
  //log(INFO, "MAC: %04X%08X", (uint16_t)(espid >> 32), (uint32_t)espid);
  return id;
}

char *buildSSID() {
  // build SSID from ESP chip id
  uint32_t id = getESPchipID();
  sprintf(ssid, "ESP32-%d", id);
  return ssid;
}
int32_t calcWiFiSignalQuality(int32_t rssi) {
	// Treat 0 or positive values as 0%
	if (rssi >= 0 || rssi < -100) {
		rssi = -100;
	}
	if (rssi > -50) {
		rssi = -50;
	}
	return (rssi + 100) * 2;
}
//******************************************************************************
// output of one line with on the startpage with "|sensor|datatype|value unit|"
//******************************************************************************
void add_value_to_table(String& content, const __FlashStringHelper* sensor, const __FlashStringHelper* param, const String& value, const char* unit) {
	RESERVE_STRING(s, MED_STR);
	s = FPSTR(WEB_PAGE_DATA_LINE);
	s.replace("{s}", sensor);
  s.replace("{d}", param);
	s.replace("{val}", value);
	s.replace("{u}", String(unit));
	content += s;
}

//******************************************************************************
// Start page
//******************************************************************************
void handleRoot(void) {  // Handle web requests to "/" path.
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal()) {
    // -- Captive portal requests were already served.
    return;
  }
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
	server.sendHeader(F("Pragma"), F("no-cache"));
	server.sendHeader(F("Expires"), F("0"));

  // Enable Pagination (Chunked Transfer)
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, FPSTR(CONTENT_TYPE_TXT_HTML), "");
  RESERVE_STRING (index,XLARGE_STR);
  char tmp[50];
  const int last_signal_strength = WiFi.RSSI();
  index = FPSTR(WEB_PAGE_HEAD);
	index.replace("{t}", FPSTR(TRA_CURRENT_DATA));

// Paginate page after ~ 1500 Bytes
	server.sendContent(index);
	index = emptyString;
  index = FPSTR(WEB_PAGE_HEADLINE);
	index.replace("{id}", theName);
  index.replace("{mac}", WiFi.macAddress());
  index.replace("{fw}", VERSION_STR);
  server.sendContent(index);
  index = emptyString;

index =F("<div class='content'><h4>" TRA_ACT_VAL_HEADLINE "</h4>"
"<meta http-equiv=\"refresh\" content=\"10\">"
"<table class='v' cellspacing='0' border='1' cellpadding='5'>"
"<tr>"
"<th style=\"background-color: lightgray;\">" TRA_SENSOR "</th>"
"<th style=\"background-color: lightgray;\">" TRA_PARAMETER "</th>"
"<th style=\"background-color: lightgray;\">" TRA_VALUE "</th></tr>");

sprintf(tmp,"%.3f",Count_Rate);
add_value_to_table(index,F(tubes[TUBE_TYPE].type),FPSTR(TRA_CPS),tmp,"c/s");

sprintf(tmp,"%.3f",Dose_Rate);

add_value_to_table(index,F(tubes[TUBE_TYPE].type),FPSTR(TRA_DOSERATE),tmp,"µSv/h");

index +=F("<tr><td colspan='3'>&nbsp;</td></tr>");
// Paginate page after ~ 1500 Bytes
	server.sendContent(index);
	index = emptyString;
if(have_thp){
  sprintf(tmp,"%.1f",temperature);
  add_value_to_table(index,F("BMEx80"),FPSTR(TRA_TEMP),tmp,"°C");
  sprintf(tmp,"%.1f",pressure);
  add_value_to_table(index,F("BMEx80"),FPSTR(TRA_PRESSURE),tmp,"hPa");
  sprintf(tmp,"%.1f",humidity);
  add_value_to_table(index,F("BMEx80"),FPSTR(TRA_HUMIDITY),tmp,"%");
  index +=F("<tr><td colspan='3'>&nbsp;</td></tr>");
}
add_value_to_table(index,F("WiFi"),FPSTR(TRA_WIFISIGNAL),String(last_signal_strength),"dBm");
add_value_to_table(index,F("WiFi"),FPSTR(TRA_WIFIQUALITY),String(calcWiFiSignalQuality(last_signal_strength)),"%");
add_value_to_table(index,F("ESP32"),FPSTR(TRA_ESP_FREE_MEM),String(ESP.getFreeHeap()),"Byte");
add_value_to_table(index,F("ESP32"),FPSTR(TRA_ESP_UPTIME),delayToString(millis() - starttime),"");

index += F("</table><br>");
index += FPSTR(WEB_PAGE_START_BUTTONS);
index +=F("</div></body></html>\n");

 server.sendContent(index);
  //server.send(200, FPSTR(CONTENT_TYPE_TXT_HTML), index);
  // looks like user wants to do some configuration or maybe flash firmware.
  // while accessing the flash, we need to turn ticking off to avoid exceptions.
  // user needs to save the config (or flash firmware + reboot) to turn it on again.
  // note: it didn't look like there is an easy way to put this call at the right place
  // (start of fw flash / start of config save) - this is why it is here.
  //tick_enable(false);TR : move to setup_webconf(). Then it's only off while in configution mode, and not during 'browsing' ...
}

static char lastWiFiSSID[IOTWEBCONF_WORD_LEN] = "";

void loadConfigVariables(void) {
  // check if WiFi SSID has changed. If so, restart cpu. Otherwise, the program will not use the new SSID
  if ((strcmp(lastWiFiSSID, "") != 0) && (strcmp(lastWiFiSSID, iotWebConf.getWifiSsidParameter()->valueBuffer) != 0)) {
    log(INFO, TRA_MES_RESTART);
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
  sendToInflux = sendToInfluxParam.isChecked();
  influxPort = influxPortParam.value();
  strcpy(influxServer,influxServerParam.valueBuffer);
  strcpy(influxPath,influxPathParam.valueBuffer);
  strcpy(influxUser,influxUserParam.valueBuffer);
  strcpy(influxPassword,influxPasswordParam.valueBuffer);
  strcpy(influxMeasurement,influxMeasurementParam.valueBuffer);
  soundLocalAlarm = soundLocalAlarmParam.isChecked();
  localAlarmThreshold = localAlarmThresholdParam.value();
  localAlarmFactor = localAlarmFactorParam.value();
}

void configSaved(void) {
  log(INFO, TRA_MES_CONF_SAVED);
  loadConfigVariables();
  tick_enable(true);
}

//******************************************************************************
// Aufruf original iotWebConf.handleConfig, aber vorher tick abschalten ...
//******************************************************************************
void handleConfig(void){
  tick_enable(false);
  iotWebConf.handleConfig();
}

//******************************************************************************
// Ausgabe der seriellen Daten auf der Debugseite
//******************************************************************************
void handleSerial(void){
 String s(Debug.popLines());
	server.send(s.length() ? 200 : 204, "text/plain", s);
}

//******************************************************************************
// Debug-Seite
//******************************************************************************
void handleDebug(void){
	server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
	server.sendHeader(F("Pragma"), F("no-cache"));
	server.sendHeader(F("Expires"), F("0"));
  // Enable Pagination (Chunked Transfer)
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, FPSTR(CONTENT_TYPE_TXT_HTML), "");
  char s[10];
  RESERVE_STRING(page_content, XLARGE_STR);

  page_content = FPSTR(WEB_PAGE_HEAD);
	page_content.replace("{t}", FPSTR(TRA_DEBUG_DATA));
  //page_content.replace("{lng}", F("DE"));

  server.sendContent(page_content);

	page_content = emptyString;
  page_content = FPSTR(WEB_PAGE_HEADLINE);
	page_content.replace("{id}", theName);
  page_content.replace("{mac}", WiFi.macAddress());
  page_content.replace("{fw}", VERSION_STR);

	server.sendContent(page_content);
	page_content = emptyString;

  page_content += FPSTR(TRA_LOGLEVEL_IS);
  int lvl=NOLOG;
  if (server.hasArg("lvl")) {
    lvl = server.arg("lvl").toInt();
    setloglevel(lvl);
  }else { lvl=log_level;}
  if (lvl == 5) strcpy(s,"DEBUG");
  else if (lvl == 4) strcpy(s,"INFO");
  else if (lvl == 3) strcpy(s,"WARNING");
  else if (lvl == 2) strcpy(s,"ERROR");
  else if (lvl == 1) strcpy(s,"CRITICAL");
  else strcpy(s,"NOLOG");

  page_content.replace("{lvl}", String(s));
//  page_content += F(".</h4><br/><pre id='slog' class='panels'>");
  page_content += F("<div class='debuglist'><br/><pre id='slog' class='panels'>");
	page_content += Debug.popLines();
	/*page_content += F("</pre><script>function slog_update(){\
	fetch('/serial').then(r=>r.text()).then((r)=>{\
  document.getElementById('slog').innerText+=r;}).catch(err=>console.log(err));};\
	setInterval(slog_update,3000);</script>");*/
  page_content += FPSTR(WEB_PAGE_DBG_SCRIPT);
  page_content += F("</div><div>");
  server.sendContent(page_content);
	page_content = emptyString;

	page_content += FPSTR(TRA_SET_LOGLEVEL_TO);
  page_content.replace("{lvl}", F("..."));
  page_content += FPSTR(WEB_PAGE_DBG_BUTTONS);
  page_content += F("<a class='button' href='/'>" TRA_BUTTON_BACK "</a></div></body></html>");
  server.sendContent(page_content);
}
/*****************************************************************
 * Webserver Images                                              *
 *****************************************************************/
static void webserver_static() {
	server.sendHeader(F("Cache-Control"), F("max-age=2592000, public"));

	if (server.arg(String('r')) == F("logo")) {
		server.send_P(200, CONTENT_TYPE_IMAGE_PNG,
			LOGO_PNG, LOGO_PNG_SIZE);
	}
	else if (server.arg(String('r')) == F("css")) {
		server.send_P(200, CONTENT_TYPE_TEXT_CSS,
			WEB_PAGE_STATIC_CSS, sizeof(WEB_PAGE_STATIC_CSS)-1);
	} else {
		iotWebConf.handleNotFound();
	}
}

//******************************************************************************
// Config settings
//******************************************************************************
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
  grpInfluxDB.addItem(&sendToInfluxParam);
  grpInfluxDB.addItem(&influxServerParam);
  grpInfluxDB.addItem(&influxPathParam);
  grpInfluxDB.addItem(&influxPortParam);
  grpInfluxDB.addItem(&influxUserParam);
  grpInfluxDB.addItem(&influxPasswordParam);
  grpInfluxDB.addItem(&influxMeasurementParam);
  iotWebConf.addParameterGroup(&grpInfluxDB);
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

  // if we don't have LoRa hardware, do not send to LoRa
  if (!isLoraBoard)
    sendToLora = false;

  iotWebConf.init();

  loadConfigVariables();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  // using our own /config which simply turns of ticks first, then calls iotWebConf.handleConfig()
  // server.on("/config", [] { iotWebConf.handleConfig(); }); //TR : original code
  //server.on("/config", [] { handleConfig(); });  //works, but slow ???
  server.on("/config", handleConfig);
  server.on("/debug", handleDebug);              //debug page
	server.on("/serial", handleSerial );           //needed for the serial ring buffer on the debug page
  server.on(F(STATIC_PREFIX), webserver_static); // need to copy static data (logo,css) for speed reasons
  server.onNotFound([]() {
    iotWebConf.handleNotFound();
  });
}
