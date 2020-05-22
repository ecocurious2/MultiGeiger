// Web Configuration related code
// also: OTA updates

#ifndef _WEBCONF_H_
#define _WEBCONF_H_

#include "IotWebConf.h"

extern bool speakerTick;
extern bool playSound;
extern bool ledTick;
extern bool showDisplay;
extern bool sendToCommunity;
extern bool sendToMadavi;
extern bool sendToLora;
extern bool sendToBle;

extern char appeui[];
extern char deveui[];
extern char appkey[];

extern char ssid[];
extern IotWebConf iotWebConf;

void setup_webconf(bool loraHardware);

#endif // _WEBCONF_H_
