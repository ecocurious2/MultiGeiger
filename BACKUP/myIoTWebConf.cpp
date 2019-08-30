/**
 * IotWebConf01Minimal.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2018 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Minimal
 * Description:
 *   This example will shows the bare minimum required for IotWeConf to start up.
 *   After starting up the thing, please search for WiFi access points e.g. with
 *   your phone. Use password provided in the code!
 *   After connecting to the access point the root page will automatically appears.
 *   We call this "captive portal".
 *   
 *   Please set a new password for the Thing (for the access point) as well as
 *   the SSID and password of your local WiFi. You cannot move on without these steps.
 *   
 *   You have to leave the access point before to let the Thing continue operation
 *   with connecting to configured WiFi.
 *
 *   Note that you can find detailed debug information in the serial console depending
 *   on the settings IOTWEBCONF_DEBUG_TO_SERIAL, IOTWEBCONF_DEBUG_PWD_TO_SERIAL set
 *   in the IotWebConf.h .
 */

#include <IotWebConf.h>

#define CONFIG_VERSION "dem2"

// Prototypes:
void handleRoot(void);
void configSaved(void);


// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

DNSServer dnsServer;
WebServer server(80);

char displayOn[4]="ein";
char buzzerOn[4] = "aus";
char LEDOn[4] = "ein";
char soundOn[4] = "aus";

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
IotWebConfParameter displayParam = IotWebConfParameter("Display (default: ein)", "display", displayOn, 4,"text","ein/aus",NULL,"noch was");
IotWebConfSeparator separator1 = IotWebConfSeparator();
IotWebConfParameter buzzerParam = IotWebConfParameter("Piepser (default: ein)", "buzzer", buzzerOn, 4,"text","ein/aus");
IotWebConfSeparator separator2 = IotWebConfSeparator("Mit Text");
IotWebConfParameter soundParam = IotWebConfParameter("Startsound (default: aus)", "sound", soundOn, 4,"text","ein/aus");

// IotWebConfParameter intParam = IotWebConfParameter("Int param", "intParam", intParamValue, NUMBER_LEN, "number", "1..100", NULL, "min='1' max='100' step='1'");
// -- We can add a legend to the separator
// IotWebConfSeparator separator2 = IotWebConfSeparator("Calibration factor");
// IotWebConfParameter floatParam = IotWebConfParameter("Float param", "floatParam", floatParamValue, NUMBER_LEN, "number", "e.g. 23.4", NULL, "step='0.1'");

void setItUp(void)
{
//   Serial.begin(115200);
//   Serial.println();
//   Serial.println("Starting up...");

iotWebConf.addParameter(&displayParam);
iotWebConf.addParameter(&separator1);
iotWebConf.addParameter(&buzzerParam);
iotWebConf.addParameter(&separator2);
iotWebConf.addParameter(&soundParam);
iotWebConf.setConfigSavedCallback(&configSaved);
// iotWebConf.getApTimeoutParameter()->visible = true;
  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

//  Serial.println("Ready.");
}

void doLoop(void) {
  iotWebConf.doLoop();
}

byte getState(void) {
    return iotWebConf.getState();
}

String getSSID(void) {
    String ssid = String(iotWebConf.getWifiSsidParameter()->valueBuffer);
    return ssid;
}
/**
 * Handle web requests to "/" path.
 */
void handleRoot(void)
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>";
  s += "<ul>";
  s += "<li>Display: ";
  s += displayOn;
  s += "<li>Piepser: ";
  s += buzzerOn;
  s += "<li>Startsound: ";
  s += soundOn;
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved(void) {
    Serial.println("Config saved");
    // Serial.printf("Saved: display=%s\n",server.arg(displayParam.getId()).c_str());
}