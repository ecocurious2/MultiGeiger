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

#define LANGUAGE "IT"

const char TRA_CURRENT_DATA[] PROGMEM = "Valori attuali";
const char TRA_DEBUG_DATA[] PROGMEM = "Informazioni Debug";
const char TRA_SET_LOGLEVEL_TO[] PROGMEM = "<h4>Cambia livello log a {lvl}</h4>";
const char TRA_LOGLEVEL_IS[] PROGMEM = "<h4>Livello log è: {lvl}</h4>";
#define TRA_ACT_VAL_HEADLINE "Valori Attuali"
#define  TRA_BUTTON_NOLOG "Nessuno"
//#define  TRA_BUTTON_CRITICAL "CRITICI"
#define  TRA_BUTTON_ERROR "Errori"
//#define  TRA_BUTTON_WARNING "AVVISI"
//#define  TRA_BUTTON_INFO "INFO"
#define  TRA_BUTTON_MININFO "min.Info"
#define  TRA_BUTTON_MEDINFO "med.Info"
#define  TRA_BUTTON_MAXINFO "max.Info"
#define  TRA_BUTTON_DEBUG "Debug"
#define  TRA_BUTTON_CONFIG "Configurazione"
#define  TRA_BUTTON_BACK "Ritorno a Homepage"
#define  TRA_BUTTON_LOG_PAGE "Pagina LogInfo"
#define  TRA_BUTTON_REFRESH "Riavviare"
#define  TRA_REFRESH_INFO "Riavviare adesso"
#define  TRA_LOG_PAGE_INFO "Apri pagina Loginfo"
#define  TRA_CONFIG_INFO "Apri configurazione"
#define  TRA_NOLOG_INFO "Solo allerte"
#define  TRA_DEBUG_INFO "Tutto, debug incluso"
const char TRA_SEND_TO_INFO [] PROGMEM = "Inviando a {ext} ...";
const char TRA_SENT_TO_INFO [] PROGMEM = "Inviato a {ext}, status: {s}, http: ";

#define TRA_SENSOR "Sensore"
#define TRA_PARAMETER "Parametro"
#define TRA_VALUE "Valore"
#define TRA_MES_RESTART "Avviando..."
#define TRA_MES_CONF_SAVED "Configurazione salvata"
const char TRA_CPS[] PROGMEM="cps";
const char TRA_DOSERATE[] PROGMEM="Rateo di dose";
const char TRA_HV_PULSES[] PROGMEM="numero pulsioni ad alto voltaggio";
const char TRA_TEMP[] PROGMEM="Temperatura";
const char TRA_PRESSURE[] PROGMEM = "Pressione d'aria";
const char TRA_HUMIDITY[] PROGMEM = "Umidità rel.";
const char TRA_WIFISIGNAL[] PROGMEM = "Segnale";
const char TRA_WIFIQUALITY[] PROGMEM = "Qualità";
const char TRA_ESP_FREE_MEM[] PROGMEM = "memoria disponibile";
const char TRA_ESP_UPTIME[] PROGMEM = "Tempo di esercizio";
