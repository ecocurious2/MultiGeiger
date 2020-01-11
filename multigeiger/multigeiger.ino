//====================================================================================================================================
// Project: Simple Multi-Geiger
// (c) 2019 by the authors, see AUTHORS file in toplevel directory.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// (see LICENSE file in toplevel directory)
//
// Description: With minimal external components you are able to build a Geiger Counter that:
//   - is precise
//   - cheap
//   - makes the typical tick sounds
//   - produces a listing via RS232 (via USB)
//   - is adaptable to your application
//
// Information about the new Heltec board ESP32 WIFI OLED:
// - how to get the device up and running:
//   - https://robotzero.one/heltec-wifi-kit-32/
// - driver for the USB=>UART-Chip CP2102:
//   - http://esp32.net/usb-uart/#SiLabs
// - infos from the Heltec, the board manufacturer:
//   - http://www.heltec.cn/project/wifi-kit-32/?lang=en
// - it is sold on ebay e.g. under the following names:
//   - "1X(ESP32 WIFI Bluetooth Entwicklungsboard OLED 0.96 "Display IOT Kit Modul GY"
// - there is also a variant with LoRaWAN:
//   - http://fambach.net/esp32-wifi-lora-433/
//   - https://www.hackerspace-ffm.de/wiki/index.php?title=Heltec_Wifi_LoRa_32
//
//
#include "version.h"

// Fix Parameters
// Values for Serial_Print_Mode to configure Serial (USB) output mode.  DON'T TOUCH!
#define   Serial_None            0  // No Serial output
#define   Serial_Debug           1  // Only debug and error messages
#define   Serial_Logging         2  // Log measurements as a table
#define   Serial_One_Minute_Log  3  // "One Minute logging"
#define   Serial_Statistics_Log  4  // Logs time [us] between two events
//
// At sensor.community predefined counter tubes:
#define SBM20 1
#define SBM19 2
#define Si22G 3


// Values for CPU (board types)
// WIFI -> Heltec Wifi Kit 32
#define WIFI 0
// LORA  ->  Heltec Wifi Lora 32 (V2)
#define LORA 1
// STICK ->  Heltec Wireless Stick (has LoRa on board)
#define STICK 2


// Includes
//====================================================================================================================================
#include "userdefines.h"
#include <Arduino.h>
#include <U8x8lib.h>

#include "IotWebConf.h"
#include <HTTPClient.h>

// for use with BME280:
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


// Check if a CPU (board) with LoRa is selected. If not, deactivate SEND2LORA.
#if !((CPU==LORA) || (CPU==STICK))
#undef SEND2LORA
#define SEND2LORA 0
#endif

// for LoRa
#if SEND2LORA==1
#include "loraWan.h"
#endif

//====================================================================================================================================
// IOs
//  used for OLED_SDA            4
//  used for OLED_SCL           15
//  used for OLED_RST           16
//
//  used for optional LoRa    SX1276 (pin) => ESP32 (pin)
//  used for optional LoRa    ==========================
//  used for optional LoRa    SCK  = GPIO5
//  used for optional LoRa    MISO = GPIO19
//  used for optional LoRa    MOSI = GPIO27
//  used for optional LoRa    CS   = GPIO18
//  used for optional LoRa    RESET = GPIO14
//  used for optional LoRa    DIO0 (8) = GPIO26 (15)
//  used for optional LoRa    DIO1 (9) = GPIO33 (13)
//  used for optional LoRa    DIO2 (10) = GPIO32 (12)
int PIN_HV_FET_OUTPUT       =  23;  //
int PIN_HV_CAP_FULL_INPUT   =  22;  // !! has to be capable of "interrupt on change"
int PIN_GMC_count_INPUT     =   2;  // !! has to be capable of "interrupt on change"
int PIN_SPEAKER_OUTPUT_P    =  12;
int PIN_SPEAKER_OUTPUT_N    =   0;

// Inputs for the switches
#if CPU == STICK
#define PIN_SWI_0 36
#define PIN_SWI_1 37
#define PIN_SWI_2 38
#define PIN_SWI_3 39
#else
#define PIN_SWI_0 39
#define PIN_SWI_1 38
#define PIN_SWI_2 37
#define PIN_SWI_3 36
#endif

// What are the switches good for?
enum {SPEAKER_ON, DISPLAY_ON, LED_ON, UNUSED};

// What to send to sensor.community etc.
enum {SEND_CPM,SEND_BME};

#define TESTPIN 13

#if CPU == STICK
#define PIN_DISPLAY_ON 25
#endif

// Measurement interval (default 2.5min) [sec]
#define MEASUREMENT_INTERVAL 150

// How many HV capacitor charge pulses to generate before giving up.
// (MAX_CHARGEPULSES * pulse_duration) should be less than the interval in
// the main loop (currently 1000ms) where it unconditionally recharges.
#define MAX_CHARGEPULSES 333

// MAX time to wait until connected. [msec]
// If there is still no connection after that time,
// measurements will start, but won't be sent to servers.
#define MAX_WAIT_TIME 300000

// Max time the greeting display will be on. [msec]
#define AFTERSTART 5000

// Dummy server for debugging
#define SEND2DUMMY 0

// Config version for IoTWebConfig
#define CONFIG_VERSION "012"

typedef struct {
  const char* type;                                         // type string for sensor.community
  const char  nbr;                                          // number to be sent by LoRa
  const float cps_to_uSvph;                                 // factor to convert counts per second to µSievert per hour
} TUBETYPE;

TUBETYPE tubes[] = {
  {"Radiation unknown", 0, 0.0},                            // use 0.0 conversion factor for unknown tubes, so it computes an
                                                            // "obviously-wrong" 0.0 uSv/h value rather than a confusing one.
  // XXX FIXME: add comments about "how are the conversion factors determined / computed? sources for these values?"
  {"Radiation SBM-20", 20, 1/2.47},
  {"Radiation SBM-19", 19, 1/9.81888},
  // The Si22G conversion factor was determined by Juergen Boehringer like this:
  // Set up a Si22G based MultiGeiger close to the official odlinfo.bfs.de measurement unit in Sindelfingen.
  // Determine how many counts the Si22G gives within the same time the odlinfo unit needs for 1uSv.
  // Result: 44205 counts on the Si22G for 1 uSv.
  // So, to convert from cps to uSv/h, the calculation is: uSvh = cps * 3600 / 44205 = cps / 12.2792
  {"Radiation Si22G", 22, 1/12.2792}
};

//====================================================================================================================================
// Constants
const unsigned long GMC_dead_time = 190;  // Dead Time of the Geiger Counter. Has to be longer than the complete
                                          // pulse generated on the Pin PIN_GMC_count_INPUT. [µsec]

// Hosts for data delivery
#define MADAVI "http://api-rrd.madavi.de/data.php"
#define SENSORCOMMUNITY "http://api.sensor.community/v1/push-sensor-data/"
#define TOILET "http://ptsv2.com/t/enbwck3/post"

//====================================================================================================================================
// Variables
volatile bool          isr_GMC_cap_full       = 0;
volatile unsigned int  isr_GMC_counts         = 0;
volatile bool          isr_gotGMCpulse        = 0;
volatile unsigned long isr_count_timestamp    = millis();
volatile unsigned long isr_count_time_between = micros();
volatile unsigned int  isr_GMC_counts_2send   = 0;
volatile unsigned long isr_count_timestamp_2send= micros();

         unsigned int  GMC_counts             = 0;
         unsigned int  GMC_counts_2send       = 0;
         unsigned int  accumulated_GMC_counts = 0;
         unsigned long count_timestamp        = millis();
         unsigned long count_timestamp_2send  = millis();
         unsigned long last_count_timestamp   = millis();
         unsigned long last_count_timestamp_2send = millis();
         unsigned long accumulated_time       = 0;
         unsigned int  last_GMC_counts        = 0;
         unsigned int  hvpulsecnt2send        = 0;
         float         Count_Rate             = 0.0;
         float         Dose_Rate              = 0.0;
         float         accumulated_Count_Rate = 0.0;
         float         accumulated_Dose_Rate  = 0.0;
         unsigned long lastMinuteLog          = millis();
         unsigned int  lastMinuteLogCounts    = 0;
         unsigned int  current_cpm            = 0;

         unsigned long toSendTime             = millis();
         unsigned long afterStartTime         = 0;
         unsigned long time2hvpulse           = millis();
         unsigned long time2display           = millis();

         bool          showDisplay            = SHOW_DISPLAY;
         bool          speakerTick            = SPEAKER_TICK;
         bool          ledTick                = LED_TICK;
         bool          playSound              = PLAY_SOUND;
         bool          displayIsClear         = false;
         char          ssid[30];
         int           haveBME280             = 0;
         float         bme_temperature        = 0.0;
         float         bme_humidity           = 0.0;
         float         bme_pressure           = 0.0;
         float         GMC_factor_uSvph       = 0.0;
         portMUX_TYPE  mux_cap_full = portMUX_INITIALIZER_UNLOCKED;
         portMUX_TYPE  mux_GMC_count = portMUX_INITIALIZER_UNLOCKED;
         char          *Serial_Logging_Header = "%10s %15s %10s %9s %9s %8s %9s %9s %9s\r\n";
         char          *Serial_Logging_Body   = "%10d %15d %10f %9f %9d %8d %9d %9f %9f\r\n";
         char          *Serial_One_Minute_Log_Header = "%4s %10s %29s\r\n";
         char          *Serial_One_Minute_Log_Body   = "%4d %10d %29d\r\n";
         char          revString[25];
         unsigned int  lora_software_version; 
         String          dashes                = F("----------------------------------------------------------------------------------------------------------------------------------------------------");
         int           Serial_Print_Mode       = SERIAL_DEBUG;

//====================================================================================================================================
// ISRs

void IRAM_ATTR isr_GMC_capacitor_full() {
  portENTER_CRITICAL_ISR(&mux_cap_full);
  isr_GMC_cap_full = 1;
  portEXIT_CRITICAL_ISR(&mux_cap_full);
}

void IRAM_ATTR isr_GMC_count() {
  portENTER_CRITICAL_ISR(&mux_GMC_count);
  static unsigned long isr_count_timestamp_us;
  static unsigned long isr_count_timestamp_us_prev;
  static unsigned long isr_count_timestamp_us_prev_used;
  digitalWrite(TESTPIN,HIGH);
  isr_count_timestamp_us_prev = isr_count_timestamp_us;
  isr_count_timestamp_us      = micros();
  if ((isr_count_timestamp_us-isr_count_timestamp_us_prev) > GMC_dead_time) {
    // the rest is only executed if GMC_dead_time is exceeded.
    // Reason: pulses occurring short after another pulse are false pulses generated by the rising edge on the PIN_GMC_count_INPUT.
    // This happens because we don't have an Schmitt-Trigger on this controller pin.
    isr_GMC_counts++;                                  // count the pulse
    isr_count_timestamp = isr_count_timestamp_2send = millis();   // notice (system) time of the pulse
    isr_GMC_counts_2send++;
    isr_gotGMCpulse = 1;

    isr_count_time_between           = isr_count_timestamp_us-isr_count_timestamp_us_prev_used;  // save for statistics debuging
    isr_count_timestamp_us_prev_used = isr_count_timestamp_us;
  }
  digitalWrite(TESTPIN,LOW);
  portEXIT_CRITICAL_ISR(&mux_GMC_count);
}


//====================================================================================================================================
// Function Prototypes
int jb_HV_gen_charge__chargepulses(int max_chargepulses);
void DisplayGMC(int TimeSec, int RadNSvph, int CPS);
void SoundStartsound();
void jbTone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume);
void DisplayStartscreen(void);
void sendData2TTN(int sendwhat, unsigned int hvpulses);
void sendData2http(const char* host, int sendwhat, unsigned int hvpulses, bool debug);
String buildhttpHeaderandBodyBME(HTTPClient *head, float t, float h, float p, bool addname);
String buildhttpHeaderandBodySBM(HTTPClient *head, int radiation_cpm, unsigned int hvpulses, bool addname);
void displayStatusLine(String txt);
void clearDisplayLine(int line);
void handleRoot(void);
void configSaved(void);
char * nullFill(int n, int digits);



// Type of OLED display
#if CPU != STICK
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);
#else
U8X8_SSD1306_64X32_NONAME_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);
#endif

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "ESP32Geiger";
const char* theName = "Default name for the SSID     ";       // 30 chars long!

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(theName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

Adafruit_BME280 bme;

unsigned long getESPchipID() {
  uint64_t espid = ESP.getEfuseMac();
  uint8_t *pespid = (uint8_t*)&espid;
  uint32_t id = 0;
  uint8_t *pid = (uint8_t *)&id;
  pid[0] = (uint8_t)pespid[5];
  pid[1] = (uint8_t)pespid[4];
  pid[2] = (uint8_t)pespid[3];
  Serial.printf("ID: %08X\r\n", id);
  Serial.printf("MAC: %04X%08X\r\n",(uint16_t)(espid>>32),(uint32_t)espid);
  return id;
}

//====================================================================================================================================
// *******  SETUP *******
//====================================================================================================================================
void setup()
{
  // OLED-Display
  u8x8.begin();

  // set IO-Pins
  pinMode (LED_BUILTIN,          OUTPUT);
  pinMode (PIN_HV_FET_OUTPUT,    OUTPUT);
  pinMode (PIN_SPEAKER_OUTPUT_P, OUTPUT);
  pinMode (PIN_SPEAKER_OUTPUT_N, OUTPUT);
  pinMode (PIN_GMC_count_INPUT,  INPUT);

  pinMode (PIN_SWI_0, INPUT);     // These pins DON'T HAVE PULLUPS!
  pinMode (PIN_SWI_1, INPUT);
  pinMode (PIN_SWI_2, INPUT);
  pinMode (PIN_SWI_3, INPUT);

  pinMode(TESTPIN, OUTPUT);
  digitalWrite(TESTPIN, LOW);

#if CPU == STICK
  pinMode (PIN_DISPLAY_ON, OUTPUT);
  digitalWrite(PIN_DISPLAY_ON, HIGH);
#endif
  // Initialize Pins
  digitalWrite (PIN_SPEAKER_OUTPUT_P, HIGH);
  digitalWrite (PIN_SPEAKER_OUTPUT_N, LOW);

  // set and init serial communication
  if (Serial_Print_Mode != Serial_None) {
    Serial.begin(115200);
    while (!Serial) {};
  }

  // just for fun
  Serial.printf("Let's go!\r\n");

  // build revString
  sprintf(revString,"V%d.%d.%d %s",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH,VERSION_DATE);

  // build SSID
  uint32_t xx = getESPchipID();
  sprintf(ssid,"ESP32-%d",xx);

#if SEND2LORA
  // build LoRa software version
  lora_software_version = (VERSION_MAJOR<<12)+ (VERSION_MINOR<<4) + VERSION_PATCH;
#endif
  // Check, if we have a BME280 connected:
  haveBME280 = bme.begin();
  Serial.printf("BME_Status: %d  ID:%0X\r\n", haveBME280, bme.sensorID());

  // Setup IoTWebConf
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setThingName(ssid);
  iotWebConf.init();

  // Set up conversion factor to uSv/h according to GM tube type:
  GMC_factor_uSvph = tubes[TUBE_TYPE].cps_to_uSvph;

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });


  // Write Header of Table, depending on the logging mode:

  if (Serial_Print_Mode == Serial_Logging) {
    Serial.println(dashes);
    Serial.print  ("Simple Multi-Geiger, Version ");
    Serial.println(revString);
    Serial.println(dashes);
    Serial.printf(Serial_Logging_Header,
                  "GMC_counts", "Time_difference", "Count_Rate", "Dose_Rate", "HV Pulses", "Accu_GMC", "Accu_Time", "Accu_Rate", "Accu_Dose");
    Serial.printf(Serial_Logging_Header,
                  "[Counts]",   "[ms]",            "[cps]",      "[uSv/h]",   "[-]",       "[Counts]", "[ms]",      "[cps]",     "[uSv/h]");
    Serial.println(dashes);
  }

  if (Serial_Print_Mode == Serial_One_Minute_Log) {
    Serial.println(dashes);
    Serial.print  ("Simple Multi-Geiger, Version ");
    Serial.println(revString);
    Serial.println(dashes);
    Serial.printf(Serial_One_Minute_Log_Header,
                  "Time", "Count_Rate", "Counts");
    Serial.printf(Serial_One_Minute_Log_Header,
                  "[s]",  "[cpm]",      "[Counts per last measurement]");
    Serial.println(dashes);
  }

  if (Serial_Print_Mode == Serial_Statistics_Log) {
    Serial.println(dashes);
    Serial.print  ("Simple Multi-Geiger, Version ");
    Serial.println(revString);
    Serial.println(dashes);
    Serial.println("Time between two impacts");
    Serial.println("[usec]");
    Serial.println(dashes);
  }

#if SEND2LORA
  // init LoRa
  lorawan_setup();
#endif

  DisplayStartscreen();
  displayIsClear = false;

  if(playSound) {
    SoundStartsound();
  }
  afterStartTime = AFTERSTART;

  // set interrupts (on pin change), attach interrupt handler
  attachInterrupt (digitalPinToInterrupt (PIN_HV_CAP_FULL_INPUT), isr_GMC_capacitor_full, RISING);  // capacitor full
  attachInterrupt (digitalPinToInterrupt (PIN_GMC_count_INPUT), isr_GMC_count, FALLING);            // GMC pulse detected
  
  // charge hv capacitor
  jb_HV_gen_charge__chargepulses(MAX_CHARGEPULSES);

}

// ===================================================================================================================================
// *************  LOOP  *************************
// ===================================================================================================================================
void loop()
{
  unsigned long time_difference;
  unsigned int HV_pulse_count;
  char sw[4];
  unsigned long current_ms = millis();                           // to save multiple calls to millis()
  bool update_display;

  // Read Switches (active LOW!)
  sw[0] = !digitalRead(PIN_SWI_0);
  sw[1] = !digitalRead(PIN_SWI_1);
  sw[2] = !digitalRead(PIN_SWI_2);
  sw[3] = !digitalRead(PIN_SWI_3);

  #define DISPLAYREFRESH 10000
  #define MAXCOUNTS 100

  // copy values from ISR
  portENTER_CRITICAL(&mux_GMC_count);                            // enter critical section
  GMC_counts = isr_GMC_counts;
  // Check if there are enough pulses detected or if enough time has elapsed.
  // If yes, then it is time to calculate the pulse rate, update the display and recharge the HV capacitor.
  update_display = (GMC_counts >= MAXCOUNTS) || ((current_ms - time2display) >= DISPLAYREFRESH);
  if(update_display) isr_GMC_counts = 0;
  count_timestamp = isr_count_timestamp;
  GMC_counts_2send = isr_GMC_counts_2send;
  count_timestamp_2send = isr_count_timestamp_2send;
  portEXIT_CRITICAL(&mux_GMC_count);                             // leave critical section

  // Pulse the high voltage if we got enough GMC pulses to update the display or at least every 1000ms.
  #define HVPULSE_MS 1000
  if(update_display || (current_ms - time2hvpulse) >= HVPULSE_MS ) {
    HV_pulse_count = jb_HV_gen_charge__chargepulses(MAX_CHARGEPULSES);  // charge HV capacitor - restarts time2hvpulse!
    hvpulsecnt2send += HV_pulse_count;                      // count for sending
  }

  if(update_display) {
    time2display = current_ms;
    time_difference = count_timestamp - last_count_timestamp; // calculate all derived values
    last_count_timestamp = count_timestamp;                   // notice the old timestamp
    accumulated_time += time_difference;                      // accumulate all the time
    accumulated_GMC_counts += GMC_counts;                     // accumulate all the pulses
    lastMinuteLogCounts += GMC_counts;

    Count_Rate = 0.0;
    if (time_difference != 0) {
      Count_Rate = (float)GMC_counts*1000.0/(float)time_difference;  // calculate the current count rate
    } 

    Dose_Rate = Count_Rate *GMC_factor_uSvph;                        // ... and dose rate

    // calculate the radiation over the complete time from start
    accumulated_Count_Rate = 0.0;
    if (accumulated_time != 0) {
      accumulated_Count_Rate = (float)accumulated_GMC_counts*1000.0/(float)accumulated_time;
    }
    accumulated_Dose_Rate = accumulated_Count_Rate *GMC_factor_uSvph;

    // ... and display it.
    if(showDisplay && sw[DISPLAY_ON]) {
      DisplayGMC(((int)accumulated_time/1000), (int)(accumulated_Dose_Rate*1000), (int)(Count_Rate*60));
      displayIsClear = false;
    } else {
      if (!displayIsClear) {
        u8x8.clear();
        clearDisplayLine(4);
        clearDisplayLine(5);
        displayIsClear = true;
      }
    }

    if (Serial_Print_Mode == Serial_Logging) {                       // Report all
      Serial.printf(Serial_Logging_Body,
                    GMC_counts, time_difference, Count_Rate, Dose_Rate, HV_pulse_count,
                    accumulated_GMC_counts, accumulated_time, accumulated_Count_Rate, accumulated_Dose_Rate);
    }

    if (Serial_Print_Mode == Serial_One_Minute_Log) {              // 1 Minute Log active?
      if (current_ms > (lastMinuteLog + 60000)){                     // Time reached for next 1-Minute log?
        unsigned int lastMinuteLogCountRate = ( (lastMinuteLogCounts*60000) / (current_ms-lastMinuteLog) );    // = *60 /1000
        if( ( ( ( (lastMinuteLogCounts*60000) % (current_ms-lastMinuteLog) ) * 2 ) / (current_ms-lastMinuteLog) ) >= 1 ) {
            lastMinuteLogCountRate++;                              // Rounding
        }
        Serial.printf(Serial_One_Minute_Log_Body,
                      (current_ms/1000),
                      lastMinuteLogCountRate, // = *60 /1000 +0.5: to reduce rounding errors
                      lastMinuteLogCounts);
        lastMinuteLogCounts = 0;
        lastMinuteLog       = current_ms;
      }
    }
  }

  if ((Serial_Print_Mode == Serial_Statistics_Log) && isr_gotGMCpulse) {   // statistics log active?
    unsigned int count_time_between;
    portENTER_CRITICAL(&mux_GMC_count);
    count_time_between = isr_count_time_between;
    isr_gotGMCpulse = 0;
    portEXIT_CRITICAL(&mux_GMC_count);
    Serial.println(count_time_between, DEC);
  }

  // If there were no pulses after 3 secs after start,
  // clear display anyway and show 0 counts.
  if(afterStartTime && ((current_ms-toSendTime) >= afterStartTime)) {
    afterStartTime = 0;
    if(showDisplay) {
      DisplayGMC(((int)accumulated_time/1000), (int)(accumulated_Dose_Rate*1000), (int)(Count_Rate*60));
      displayIsClear = false;
    }
  }

  // Check, if we have to send to sensor.community etc.
  if((current_ms - toSendTime) >= (MEASUREMENT_INTERVAL*1000) ) {
    toSendTime = current_ms;
    portENTER_CRITICAL(&mux_GMC_count);
    GMC_counts_2send      = isr_GMC_counts_2send;                    // copy values from ISR
    count_timestamp_2send = isr_count_timestamp_2send;
    isr_GMC_counts_2send = 0;
    portEXIT_CRITICAL(&mux_GMC_count);
    unsigned int hvp = hvpulsecnt2send;
    hvpulsecnt2send = 0;
    time_difference = count_timestamp_2send - last_count_timestamp_2send;
    last_count_timestamp_2send = count_timestamp_2send;

    current_cpm = 0;
    if(time_difference != 0) {
      current_cpm = (int)(GMC_counts_2send*60000/time_difference);
    }
    
    if (haveBME280) {                                       // read in the BME280 values
      bme_temperature = bme.readTemperature();
      bme_humidity = bme.readHumidity();
      bme_pressure = bme.readPressure();
#if 0
      Serial.printf("Measured: cpm= %d HV=%d T=%.2f H=%.f P=%.f\r\n", current_cpm, hvp, bme_temperature, bme_humidity, bme_pressure);
    } else {
      Serial.printf("Measured: cpm= %d HV=%d\r\n",current_cpm, hvp);
#endif
    }

    #if SEND2DUMMY
    displayStatusLine(F("Toilet"));
    Serial.println("SENDING TO TOILET");
    sendData2http(TOILET,SEND_CPM,hvp,true);
    if(haveBME280) {
      sendData2http(TOILET,SEND_BME,hvp,true);
    }
    delay(300);
    #endif

    #if SEND2MADAVI
    Serial.println("Sending to Madavi ...");
    displayStatusLine(F("Madavi"));
    sendData2http(MADAVI,SEND_CPM,hvp,false);
    if(haveBME280) {
      sendData2http(MADAVI,SEND_BME,hvp,false);
    }
    delay(300);
    #endif

    #if SEND2SENSORCOMMUNITY
    Serial.println("Sending to sensor.community ...");
    displayStatusLine(F("sensor.community"));
    sendData2http(SENSORCOMMUNITY,SEND_CPM,hvp,false);
    if(haveBME280) {
      sendData2http(SENSORCOMMUNITY,SEND_BME,hvp,false);
    }
    delay(300);
    #endif

    #if SEND2LORA
    Serial.println("Sending to TTN ...");
    displayStatusLine(F("TTN"));
    sendData2TTN(SEND_CPM,hvp);
    if(haveBME280) {
      sendData2TTN(SEND_BME,hvp);
    }
    #endif

    displayStatusLine(" ");

    #if 0
    // log state of switch
    Serial.printf("SW0: %d  SW1: %d  SW2: %d  SW3: %d\r\n",sw[0],sw[1],sw[2],sw[3]);
    #endif
  }

  // make LED flicker and speaker tick
  if (GMC_counts != last_GMC_counts) {
    if(ledTick && sw[LED_ON]) {
      digitalWrite(LED_BUILTIN, HIGH);    // switch on LED
    }
    if(speakerTick && sw[SPEAKER_ON]) {   // make "Tick" sound
      for (int speaker_count = 0; speaker_count <= 3; speaker_count++) {
        digitalWrite (PIN_SPEAKER_OUTPUT_P, LOW);
        digitalWrite (PIN_SPEAKER_OUTPUT_N, HIGH);
        delayMicroseconds(500);
        digitalWrite (PIN_SPEAKER_OUTPUT_P, HIGH);
        digitalWrite (PIN_SPEAKER_OUTPUT_N, LOW);
        delayMicroseconds(500);
      }
    } else {
      if(ledTick && sw[LED_ON]) {
        delay(4);
      }
    }
    if(ledTick && sw[LED_ON]) {
      digitalWrite(LED_BUILTIN, LOW);     // switch off LED
    }
    last_GMC_counts = GMC_counts;         // notice old value
  }

  // Loop for IoTWebConf
  iotWebConf.doLoop();
}

// ===================================================================================================================================
// ===================================================================================================================================
// Subfunctions

// GMC-Sub-Functions

int jb_HV_gen_charge__chargepulses(int max_chargepulses) {
  int chargepulses = 0;
  isr_GMC_cap_full = 0;
  do {
    digitalWrite(PIN_HV_FET_OUTPUT, HIGH);              // turn the HV FET on
    delayMicroseconds(1500);                            // 5000 usec gives 1,3 times more charge, 500 usec gives 1/20 th of charge
    digitalWrite(PIN_HV_FET_OUTPUT, LOW);               // turn the HV FET off
    delayMicroseconds(1000);
    chargepulses++;
  } while ((chargepulses < max_chargepulses) && !isr_GMC_cap_full); // either a timeout or a capacitor full interrupt stops this loop
  time2hvpulse = millis();                              // we just pulsed, so restart timer
  if((chargepulses == max_chargepulses) && !isr_GMC_cap_full)
    Serial.println("Error: HV charging failed!");       // pulsed a lot, but still the capacitor is not at desired voltage
  return chargepulses;
}

// ===================================================================================================================================
// OLED sub functions
void DisplayStartscreen(void){
  char line[20];

  u8x8.clear();
#if CPU == STICK
  // Display is only 4 lines by 8 characters; lines counting from 2 to 5
  u8x8.setFont(u8x8_font_5x8_f);                            // use really small font
  for (int i=2; i<6; i++) {
    u8x8.drawString(0, i, "        ");                      // clear all 4 lines
  }
  u8x8.drawString(0, 2, "Geiger-");
  u8x8.drawString(0, 3, " Counter");
  u8x8.drawString(0, 4, "Version:");
  sprintf(line,"%d.%d.%d",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH);
  u8x8.drawString(0, 5, line);
#else
  u8x8.setFont(u8x8_font_7x14_1x2_f);
  u8x8.println("Geiger-Counter");
  u8x8.println("==============");
  sprintf(line,"V%d.%d.%d",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH);
  u8x8.setCursor(7-strlen(line)/2, 4);
  u8x8.print(line);
  u8x8.setCursor(1, 6);
  u8x8.print("Info:boehri.de");
#endif
};

// ===================================================================================================================================
void DisplayGMC(int TimeSec, int RadNSvph, int CPS){
  u8x8.clear();

#if CPU != STICK
  char output[80];
  int TimeMin = TimeSec / 60;             // calculate number of minutes
  if(TimeMin >=999 ) TimeMin=999;         // limit minutes to max. 999

  // print the upper line including time and measured radation
  u8x8.setFont(u8x8_font_7x14_1x2_f);

  if(TimeMin >= 1){                       // >= 1 minute -> display in minutes
    sprintf(output, "%3d", TimeMin);
    u8x8.print(output);
  } else {                                // < 1 minute -> display in seconds, inverse
    sprintf(output, "%3d", TimeSec);
    u8x8.inverse();
    u8x8.print(output);
    u8x8.noInverse();
  }

  sprintf(output, "%7d nSv/h", RadNSvph);
  u8x8.print(output);
#endif

  // print the lower line including time and CPM value
#if CPU != STICK
  u8x8.setFont(u8x8_font_inb33_3x6_n);
  u8x8.drawString(0, 2, nullFill(CPS, 5));
#else
  u8x8.setFont(u8x8_font_5x8_f);
  u8x8.drawString(0, 2, nullFill(RadNSvph, 8));
  u8x8.draw2x2String(0, 3, nullFill(CPS, 4));
  u8x8.drawString(0, 5, "     cpm");
#endif

#if CPU != STICK
  // Print 'connecting...' as long as we aren't connected
  if (iotWebConf.getState() != IOTWEBCONF_STATE_ONLINE) {
    displayStatusLine(F("connecting..."));
  } else {
    displayStatusLine(" ");
  }
#endif
};

#if CPU != STICK
void clearDisplayLine(int line) {
  String blank = F("                ");
  u8x8.drawString(0, line, blank.c_str());
}

void displayStatusLine(String txt) {
  u8x8.setFont(u8x8_font_5x8_f);
  clearDisplayLine(7);
  u8x8.drawString(0, 7, txt.c_str());
}
#else
void clearDisplayLine(int line) {
  String blank = F("        ");
  u8x8.drawString(0, line, blank.c_str());
}

void displayStatusLine(String txt) {
  u8x8.setFont(u8x8_font_5x8_f);
  clearDisplayLine(5);
  u8x8.drawString(0, 5, txt.c_str());
}
#endif

// ===================================================================================================================================
// Sound Subfunctions
void SoundStartsound(){
  float freq_factor = 0.75;
  int time_factor   =   85;

  jbTone(1174659*freq_factor,    2*time_factor, 1); // D
  delay(                         2*time_factor   ); // ---
  jbTone(1318510*freq_factor,    2*time_factor, 1); // E
  delay(                         2*time_factor   ); // ---
  jbTone(1479978*freq_factor,    2*time_factor, 1); // Fis
  delay(                         2*time_factor   ); // ---

  jbTone(1567982*freq_factor,    4*time_factor, 1); // G
  jbTone(1174659*freq_factor,    2*time_factor, 1); // D
  jbTone(1318510*freq_factor,    2*time_factor, 1); // E
  jbTone(1174659*freq_factor,    4*time_factor, 1); // D
  jbTone( 987767*freq_factor,    2*time_factor, 1); // H
  jbTone(1046502*freq_factor,    2*time_factor, 1); // C
  jbTone( 987767*freq_factor,    4*time_factor, 1); // H
  jbTone( 987767*freq_factor,    4*time_factor, 0); // H

  return;
}


void jbTone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume){
  unsigned int  cycle_time_us, cycle_1_time_us, cycle_2_time_us;
  unsigned long count_timestamp_end;

  cycle_time_us   = 1000000000/frequency_mHz;
  cycle_1_time_us = cycle_time_us/2;
  cycle_2_time_us = cycle_time_us - cycle_1_time_us;
  count_timestamp_end = millis() + time_ms;

  do{
    digitalWrite (PIN_SPEAKER_OUTPUT_P, (volume==1));
    digitalWrite (PIN_SPEAKER_OUTPUT_N, LOW);
    delayMicroseconds(cycle_1_time_us);
    digitalWrite (PIN_SPEAKER_OUTPUT_P, LOW);
    digitalWrite (PIN_SPEAKER_OUTPUT_N, HIGH);
    delayMicroseconds(cycle_2_time_us);
  } while(millis()<count_timestamp_end);
  return;
}

// ===================================================================================================================================
// Send to Server Subfunctions

String buildhttpHeaderandBodySBM(HTTPClient *head, unsigned int hvpulses, boolean addname, bool debug) {
  head->addHeader("Content-Type", "application/json; charset=UTF-8");
  head->addHeader("X-PIN","19");
  String chipID = String(ssid);
  chipID.replace("ESP32","esp32");
  head->addHeader("X-Sensor",chipID);
  head->addHeader("Connection","close");
  String tubetype = tubes[TUBE_TYPE].type;
  tubetype = tubetype.substring(10);
  String valuetype = (addname ? tubetype+"_" : "");
  valuetype += "counts_per_minute";
  String body = "{\"software_version\":\""+String(revString)+"\",\"sensordatavalues\":[";
  body += "{\"value_type\":\""+valuetype+"\",\"value\":\""+current_cpm+"\"}";
  if (debug) {
    body += ",{\"value_type\":\"hv_pulses\",\"value\":\""+String(hvpulses)+"\"}";
    body += ",{\"value_type\":\"tube\",\"value\":\""+tubetype+"\"}";
  }
  body += "]}";
  if (DEBUG_SERVER_SEND == 1) {
    Serial.println(body);
  }
  return body;
}

String buildhttpHeaderandBodyBME(HTTPClient *head, boolean addname, bool debug) {
  head->addHeader("Content-Type", "application/json; charset=UTF-8");
  head->addHeader("X-PIN","11");
  String chipID = String(ssid);
  chipID.replace("ESP32","esp32");
  head->addHeader("X-Sensor",chipID);
  head->addHeader("Connection","close");
  String temp = (addname ? "BME280_" : "");
  temp += "temperature";
  String humi = (addname ? "BME280_" : "");
  humi += "humidity";
  String press = (addname ? "BME280_" : "");
  press += "pressure";
  String body = "{\"software_version\":\""+String(revString)+"\",\"sensordatavalues\":[\
{\"value_type\":\""+temp+"\",\"value\":\""+String(bme_temperature,2)+"\"},\
{\"value_type\":\""+humi+"\",\"value\":\""+String(bme_humidity,2)+"\"},\
{\"value_type\":\""+press+"\",\"value\":\""+String(bme_pressure,2)+"\"}\
]}";
  if (DEBUG_SERVER_SEND == 1) {
    Serial.println(body);
  }
  return body;
}

void sendData2http(const char* host, int sendwhat, unsigned int hvpulses, bool debug) {
  HTTPClient http;
  String body;
  http.begin(host);
  if (sendwhat == SEND_CPM) {
    body = buildhttpHeaderandBodySBM(&http,hvpulses,false,debug);
  }
  if (sendwhat == SEND_BME) {
    body = buildhttpHeaderandBodyBME(&http,false,debug);
  }
  int httpResponseCode = http.POST(body);
  if(httpResponseCode>0){
    String response = http.getString();
    if (DEBUG_SERVER_SEND == 1) {
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// LoRa payload:
// To minimise airtime, we only send necessary bytes. We do NOT use Cayenne LPP.
// The payload will be translated via http integration and a small python program
// to be compatible with sensor.community. For byte definitions see ttn2luft.pdf in
// docs directory.
#if SEND2LORA
void sendData2TTN(int sendwhat, unsigned int hvpulses) {
  unsigned char ttnData[20];
  int cnt;
  if(sendwhat == SEND_CPM) {
  // first two bytes are the cpm
  ttnData[0] = current_cpm >> 8;
  ttnData[1] = current_cpm & 0xFF;
  // next two bytes are the number of HV pulses
  ttnData[2] = hvpulses >> 8;
  ttnData[3] = hvpulses & 0xFF;
  // next byte is the tube version
  ttnData[4] = tubes[TUBE_TYPE].nbr;
  // and last is software version
  ttnData[5] = (lora_software_version>>8)&0xFF;
  ttnData[6] = lora_software_version&0xFF;
  cnt = 7;
  lorawan_send(1,ttnData,cnt,false,NULL,NULL,NULL);
  };
  if(sendwhat == SEND_BME) {
    ttnData[0] = ((int)(bme_temperature*10)) >> 8;
    ttnData[1] = ((int)(bme_temperature*10)) & 0xFF;
    ttnData[2] = (int)(bme_humidity*2);
    ttnData[3] = ((int)(bme_pressure/10)) >> 8;
    ttnData[4] = ((int)(bme_pressure/10)) & 0xFF;
    cnt = 5;
    lorawan_send(2,ttnData,cnt,false,NULL,NULL,NULL);
  }
}
#endif

/**
 * Handle web requests to "/" path.
 */
void handleRoot(void)
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal requests were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved(void) {
  Serial.println("Config saved");
}

char * nullFill(int n, int digits) {
  static char erg[9];                          // max. 8 digits possible!
  if (digits > 8) {
    digits = 8;
  }
  char format[5];
  sprintf(format,"%%%dd",digits);
  sprintf(erg,format,n);
  return erg;
}
