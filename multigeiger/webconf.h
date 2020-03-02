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

extern IotWebConfSeparator sep0;
extern IotWebConfParameter startSoundParam;
extern IotWebConfParameter speakerTickParam;
extern IotWebConfParameter ledTickParam;
extern IotWebConfParameter showDisplayParam;
extern IotWebConfSeparator sep1;
extern IotWebConfParameter sendToCommunityParam;
extern IotWebConfParameter sendToMadaviParam;
#if CPU==STICK
extern IotWebConfSeparator sep3;
extern IotWebConfParameter sendToLoraParam;
extern IotWebConfParameter appeuiParam;
extern IotWebConfParameter deveuiParam;
extern IotWebConfParameter appkeyParam;
#endif


extern char ssid[];
extern IotWebConf iotWebConf;

void setup_webconf(void);
boolean parameterTrue(char *parameter);

#endif // _WEBCONF_H_
