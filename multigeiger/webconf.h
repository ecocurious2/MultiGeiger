// Web Configuration related code
// also: OTA updates

#ifndef _WEBCONF_H_
#define _WEBCONF_H_

#include "IotWebConf.h"

extern char startSound[];
extern char speakerTick[];
extern char ledTick[];
extern char showDisplay[];
extern char sendToCommunity[];
extern char sendToMadavi[];
#if CPU==STICK
extern char sendToLora[];
extern char appeui[];
extern char deveui[];
extern char appkey[];
#endif

extern char ssid[];
extern IotWebConf iotWebConf;

void setup_webconf(void);
boolean parameterTrue(char *parameter);

#endif // _WEBCONF_H_
