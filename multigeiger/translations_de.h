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

#define LANGUAGE "DE"

const char TRA_CURRENT_DATA[] PROGMEM = "Aktuelle Werte";
const char TRA_DEBUG_DATA[] PROGMEM = "Debug Info";
const char TRA_SET_LOGLEVEL_TO[] PROGMEM = "<h4>Setze Loglevel auf {lvl}</h4>";
const char TRA_LOGLEVEL_IS[] PROGMEM = "<h4>Loglevel ist: {lvl}</h4>";
#define TRA_ACT_VAL_HEADLINE "Aktuelle Werte"
#define  TRA_BUTTON_NOLOG "NoLog"
//#define  TRA_BUTTON_CRITICAL "KRITISCH"
#define  TRA_BUTTON_ERROR "Fehler"
//#define  TRA_BUTTON_WARNING "WARNUNG"
//#define  TRA_BUTTON_INFO "INFO"
#define  TRA_BUTTON_MININFO "min.Info"
#define  TRA_BUTTON_MEDINFO "med.Info"
#define  TRA_BUTTON_MAXINFO "max.Info"
#define  TRA_BUTTON_DEBUG "Debug"
#define  TRA_BUTTON_CONFIG "Konfiguration"
#define  TRA_BUTTON_BACK "Zurück zur Startseite"
#define  TRA_BUTTON_LOG_PAGE "LogInfo Seite"
#define  TRA_BUTTON_REFRESH "Aktualisieren"
#define  TRA_REFRESH_INFO "man.Aktualisieren"
#define  TRA_LOG_PAGE_INFO "LogInfo Seite öffnen"
#define  TRA_CONFIG_INFO "Konfiguration öffnen"
#define  TRA_NOLOG_INFO "nur Alarme"
#define  TRA_DEBUG_INFO "alles, mit Debuginfo"
const char TRA_SEND_TO_INFO [] PROGMEM = "Sende an {ext} ...";
const char TRA_SENT_TO_INFO [] PROGMEM = "An {ext} gesandt, Status: {s}, http: ";

#define TRA_SENSOR "Sensor"
#define TRA_PARAMETER "Parameter"
#define TRA_VALUE "Wert"
#define TRA_MES_RESTART "Starte neu..."
#define TRA_MES_CONF_SAVED "Konfig. gespeichert"
const char TRA_CPS[] PROGMEM="cps";
const char TRA_DOSERATE[] PROGMEM="Dosisleistung";
const char TRA_HV_PULSES[] PROGMEM="HochVolt Pulse";
const char TRA_TEMP[] PROGMEM="Temperatur";
const char TRA_PRESSURE[] PROGMEM = "Luftdruck";
const char TRA_HUMIDITY[] PROGMEM = "rel. Luftfeuchte";
const char TRA_WIFISIGNAL[] PROGMEM = "Signal";
const char TRA_WIFIQUALITY[] PROGMEM = "Qualität";
const char TRA_ESP_FREE_MEM[] PROGMEM = "Freier Speicher";
const char TRA_ESP_UPTIME[] PROGMEM = "Laufzeit";

