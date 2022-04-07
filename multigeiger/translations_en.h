/* TR, 07.04.2022
 add new translation strings to ALL files translations_xx.h !!!
 do NOT add translation_xx.h to any of your routines, but transl.h
 Steps to add a new language :
  1. create a new file translation_xx.h where xx is the language code
  2. add this file name to transl.h
  3. add a new chapter into platformio.ini (like [env:geiger_en] (copy&paste), and only adopt the buildflag '-DTRANSL_XX' to the new language
     this defines TRANSL_XX which can be queried in transl.h during runtime in its if-elif and includes the corresponding translation_xx.h.
*/
#include <pgmspace.h>

#define LANGUAGE "EN"

const char TRA_CURRENT_DATA[] PROGMEM = "Actual values";
const char TRA_DEBUG_DATA[] PROGMEM = "Debug Info";
const char TRA_SET_LOGLEVEL_TO[] PROGMEM = "<h4>Seting Loglevel to {lvl}</h4>";
const char TRA_LOGLEVEL_IS[] PROGMEM = "<h4>Loglevel is: {lvl}</h4>";
#define TRA_ACT_VAL_HEADLINE "Actual Values"
#define  TRA_BUTTON_NOLOG "NOLOG"
#define  TRA_BUTTON_CRITICAL "CRITICAL"
#define  TRA_BUTTON_ERROR "ERROR"
#define  TRA_BUTTON_WARNING "WARNING"
#define  TRA_BUTTON_INFO "INFO"
#define  TRA_BUTTON_DEBUG "DEBUG"
#define  TRA_BUTTON_CONFIG "Configuration"
#define  TRA_BUTTON_BACK "Back to Homepage"
#define  TRA_BUTTON_LOG_PAGE "LogInfo page"
#define  TRA_BUTTON_REFRESH "Refresh"
#define  TRA_REFRESH_INFO "manual Refresh"
#define  TRA_LOG_PAGE_INFO "open LogInfo page"
#define  TRA_CONFIG_INFO "open Configuration"
#define  TRA_NOLOG_INFO "min. Info"
#define  TRA_DEBUG_INFO "max. Info"
const char TRA_SEND_TO_INFO [] PROGMEM = "Sending to {ext} ...";
const char TRA_SENT_TO_INFO [] PROGMEM = "Sent to {ext}, status: {s}, http: ";

#define TRA_SENSOR "Sensor"
#define TRA_PARAMETER "Parameter"
#define TRA_VALUE "Value"
#define TRA_MES_RESTART "Restarting..."
#define TRA_MES_CONF_SAVED "Config saved."
const char TRA_CPS[] PROGMEM="cps";
const char TRA_DOSERATE[] PROGMEM="Dose rate";
const char TRA_HV_PULSES[] PROGMEM="High Voltage pulses";
const char TRA_TEMP[] PROGMEM="Temperature";
const char TRA_PRESSURE[] PROGMEM = "Air pressure";
const char TRA_HUMIDITY[] PROGMEM = "rel. Humidity";
const char TRA_WIFISIGNAL[] PROGMEM = "Signal";
const char TRA_WIFIQUALITY[] PROGMEM = "Quality";
const char TRA_ESP_FREE_MEM[] PROGMEM = "Free Memory";
const char TRA_ESP_UPTIME[] PROGMEM = "Uptime";
