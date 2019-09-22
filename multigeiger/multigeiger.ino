//====================================================================================================================================
// Project:     Simple Arduino Geiger
// Description: With minimal exteral Components you are able to build a Geiger Counter that:
//   - is precice
//   - cheap
//   - has a audibal tick
//   - produces a listing via RS232 (via USB)
//   - is adaptable to your application
// 
// Information to the new Heltec board ESP32 WIFI OLED
// - for Information how to get the device up and running please check:
//   - https://robotzero.one/heltec-wifi-kit-32/
// - The driver for the USB=>UART-Chip CP2102 you can find here:
//   - http://esp32.net/usb-uart/#SiLabs
// - Informations from the Board-Manufacturer:
//   - http://www.heltec.cn/project/wifi-kit-32/?lang=en
// - it is sold in ebay e. g. under the following names
//   - "1X(ESP32 WIFI Bluetooth Entwicklungsboard OLED 0.96 "Display IOT Kit Modul GY"
// - there is also a LoRaWAN-Variant available. Check: 
//   - http://fambach.net/esp32-wifi-lora-433/
//   - https://www.hackerspace-ffm.de/wiki/index.php?title=Heltec_Wifi_LoRa_32
//
//
// Revision History:
//const char* revString = "2019_03_25";   // Juergen Boehringer      - Version 1 for ESP32-Board
//const char* revString = "2019_04_26";   // Juergen Boehringer      - added 1 Minute RS232-Logging-Mode
//const char* revString = "2019_05_12";   // Juergen Boehringer      - Added Bugfix for the "Double-Trigger-Problem". This was caused
//                                                                   by the rising edge falsly triggering an other pulse recording. 
//                                                                   The Problem is that there is no Schmitt-Trigger available in the 
//                                                                   Controller
//                                                                 - simplified serial printing modes
//                                                                 - made seconds in Display as inverse to be able to seperate it from
//                                                                   minutes
//                                                                 - cleaned up the code
//                                                                 - Fixed Overflow-Bug in Minute-Count
// const char* revString = "V1.0_2019_08_19";   // rxf             - added detection of LoRa-Device
//                                                                 - WiFiManager to enter WLAN data and other configs
//                                                                 - send to luftdaten.info every 2.5 min
// const char* revString = "V1.1_2019_09_01";   // rxf              - build SSID out of MAC corrected: first 3 Byte of MAC build SSID
//                                                                 - IoTWebConfig: setter for thingName added; lib moved into local source path          
//                                                                 - LoRa autodetection removed
// const char* revString = "V1.2_2019_09_02";   // rxf             - sending to madavi corrected
// const char* revString = "V1.3_2019_09_03";   // rxf             - Building MAC changed again. Now its identical to 'Feinstaubsensor'
// const char* revString = "V1.4_2019-09-03";   // rxf              - default config, measurment interval 10min
// const char* revString = "V1.5_2019-09-11";     // rxf               - added BME280 via I2C
//                                                                  - Display adapted for Wireless Stick
//                                                                  - added Lora 
// const char* revString = "V1.6_2019-09-13";     // rxf               - rearrangement of files
//                                                                  - test dip-switch
//                                                                  - Hardware-Layout V1.3 and lower - OLD Wifi-Kit-32 !
const char* revString = "V1.7_2019-09-22";     // rxf               - PINs rearranged, so we can use WiFi Stick Light
//                                                                  - Hardware-Layout V1.4 and up

// Fix Parameters
// Possible Values for Serial_Print_Mode  ! DONT TOUCH !
#define   Serial_None            0  // No Serial Printout
#define   Serial_Debug           1  // Only debug and error output will be printed via RS232(USB)
#define   Serial_Logging         2  // Log measurement as table via RS232(USB)
#define   Serial_One_Minute_Log  3  // One Minute logging will be printed via RS232(USB)
#define   Serial_Statistics_Log  4  // Lists time between two events in us via RS232(USB)
//       
// At luftdaten.info predefined counter tubes:
#define SBM20 "Radiation SBM-20"
#define SBM19 "Radiation SBM-19"
#define Si22G "Radiation Si22G"      

// Usable CPU-Types
// WIFI -> Heltev Wifi Kit 32
#define WIFI 0
// LORA  ->  Heltec Wifi Lora 32 (V2)
#define LORA 1
// STICK ->  Heltec Wireless Stick  (has LoRa on board)
#define STICK 2
//
// Includes
//====================================================================================================================================
#include "userdefines.h"
//====================================================================================================================================
#include <Arduino.h>
#include <U8x8lib.h>

#include "IotWebConf.h"
#include <HTTPClient.h>

// for use with BME280:
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Check if LoRa-CPU is selected. If not, deactivate SEND2LORA
#if !((CPU==LORA) || (CPU==STICK))
#undef SEND2LORA
#define SEND2LORA 0
#endif

// for LoRa
#if SEND2LORA==1
#include "lorawan.h"
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
int PIN_GMZ_count_INPUT     =   2;  // !! has to be capable of "interrupt on change"
int PIN_SPEAKER_OUTPUT_P    =  12;
int PIN_SPEAKER_OUTPUT_N    =   0;

// Inputs for the switches
#define PIN_SWI_0 36
#define PIN_SWI_1 37
#define PIN_SWI_2 38
#define PIN_SWI_3 39

#define TESTPIN 13

#if CPU == STICK
#define  DISPLAY_ON 25
#endif

// Messinteravll (default 10min) [sec]
#define MESSINTERVAL 60

// MAX time to wait until connected. After then, meaurement starts but there is no sending to servers  [msec]
#define MAX_WAIT_TIME 300000

// Max this time the greeting display will be on. [msec]
#define AFTERSTART 5000

// Dummy server for debugging
#define SEND2DUMMY 0

// Config-Version for IoTWebConfig
#define CONFIG_VERSION "012"

//====================================================================================================================================
// Constants
const            float GMZ_factor_uSvph     = 1/2.47   ; // for SBM20
//const            float GMZ_factor_uSvph     = 1/9.81888   ; // for SBM19

const    unsigned long GMZ_dead_time        = 190;   // Dead Time of the Geiger-Counter. Has to be longer than the complete
                                                     // Pulse generated on the Pin PIN_GMZ_count_INPUT [µsec]

//Hosts for data delivery
#define MADAVI "http://api-rrd.madavi.de/data.php"
#define LUFTDATEN "http://api.luftdaten.info/v1/push-sensor-data/"
#define TOILET "http://ptsv2.com/t/enbwck3/post"

//====================================================================================================================================
// Variables
volatile bool          GMZ_cap_full           = 0;
volatile unsigned char isr_GMZ_counts         = 0;
         unsigned char counts_before          = 0;
volatile unsigned long isr_count_timestamp    = millis(); 
volatile unsigned long isr_count_time_between = micros();
         unsigned char GMZ_counts             = 0;
         unsigned int  accumulated_GMZ_counts = 0;
         unsigned long count_timestamp        = millis(); 
         unsigned long last_count_timestamp   = millis(); 
         unsigned long time_difference        = 1000; 
         unsigned long accumulated_time       = 0; 		 
         unsigned char last_GMZ_counts        = 0;
         unsigned char speaker_count          = 0;
              uint32_t HV_pulse_count         = 0;
         float         Count_Rate             = 0.0;
         float         Dose_Rate              = 0.0;
         float         accumulated_Count_Rate = 0.0;
         float         accumulated_Dose_Rate  = 0.0;
         unsigned long lastMinuteLog          = millis();		 
         unsigned int  lastMinuteLogCounts    = 0;
         unsigned int  lastMinuteLogCountRate = 0;

         unsigned long toSendTime             = 0;
         unsigned long afterStartTime         = 0;
         unsigned int  counts_p_interval      = 0;

         bool          showDisplay            = SHOW_DISPLAY;
         bool          speakerTick            = SPEAKER_TICK;
         bool          ledTick                = LED_TICK;
         bool          playSound              = PLAY_SOUND;
         bool          displayIsClear         = false;
         char          ssid[30];
         int           haveBME280             = 0;
         float         t                      = 0.0;
         float         h                      = 0.0;
         float         p                      = 0.0;
         

int  Serial_Print_Mode      = SERIAL_DEBUG;

extern "C" {
uint8_t temprature_sens_read();
}       

//====================================================================================================================================
// ISRs
void isr_GMZ_capacitor_full() {
  GMZ_cap_full = 1;
}

void isr_GMZ_count() {
  static unsigned long isr_count_timestamp_us     ;
  static unsigned long isr_count_timestamp_us_prev;
  static unsigned long isr_count_timestamp_us_prev_used;
  digitalWrite(TESTPIN,HIGH);
  noInterrupts();                                       // Disable interrupts (to read variables of the ISR correctly)

  isr_count_timestamp_us_prev = isr_count_timestamp_us;
  isr_count_timestamp_us      = micros();
  if ((isr_count_timestamp_us-isr_count_timestamp_us_prev) > GMZ_dead_time){ 
    // the rest ist only executed if GMZ_dead_time is exeded. 
    // Reason: Pulses occuring short after an other pulse are false pulses generated by the rising edge on PIN_GMZ_count_INPUT. This 
    // happens because we don't have an Schmitt-Trigger in this controller pin
    isr_GMZ_counts++;                                     // Count
    isr_count_timestamp       = millis();                 // notice (System)-Time of the Count
	
    isr_count_time_between           = isr_count_timestamp_us-isr_count_timestamp_us_prev_used;	// Save for Statistic Debuging
	  isr_count_timestamp_us_prev_used = isr_count_timestamp_us;
  } 
  interrupts();                                        // Re-enable interrupts  
  digitalWrite(TESTPIN,LOW);
}


//====================================================================================================================================
// Function Prototypes
int jb_HV_gen_charge__chargepules();
void DisplayGMZ(int TimeSec, int RadNSvph, int CPS);
void SoundStartsound();	
void jbTone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume);
void DisplayStartscreen(void);
void sendData2luftdaten(bool sendwhat, int wert, float t=0.0, float h=0.0, float p=0.0);
void sendData2madavi(bool sendwhat, int wert, float t=0.0, float h=0.0, float p=0.0);
void sendData2toilet(bool sendwhat, int wert, float t=0.0, float h=0.0, float p=0.0);
void sendData2TTN(int wert, float t=0.0, float h=0.0, float p=0.0);
String buildhttpHeaderandBodyBME(HTTPClient *head, float t, float h, float p, bool addname);
String buildhttpHeaderandBodySBM(HTTPClient *head, int wert, bool addname);
void displayStatusLine(String txt);
void handleRoot(void);
void configSaved(void);
char * nullFill(int n, int digits);



// Type of OLED-Display
#if CPU != STICK
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);
#else
U8X8_SSD1306_64X32_NONAME_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);
#endif

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "ESP32Geiger";
const char* theName = "Defaultname fuer die SSID     ";       // 30 chars log !      

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
  Serial.printf("ID: %08X\n", id);
  Serial.printf("MAC: %04X%08X\n",(uint16_t)(espid>>32),(uint32_t)espid);
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
  pinMode      (LED_BUILTIN,         OUTPUT);  
  pinMode      (PIN_HV_FET_OUTPUT  , OUTPUT);    
  pinMode      (PIN_SPEAKER_OUTPUT_P,OUTPUT); 
  pinMode      (PIN_SPEAKER_OUTPUT_N,OUTPUT);
  pinMode      (PIN_GMZ_count_INPUT,INPUT);

  pinMode (PIN_SWI_0,INPUT);                         // !!! These pins DON'T HAVE PULLUPS !!
  pinMode (PIN_SWI_1,INPUT);
  pinMode (PIN_SWI_2,INPUT);
  pinMode (PIN_SWI_3,INPUT);

  pinMode(TESTPIN,OUTPUT);
  digitalWrite(TESTPIN,LOW);

#if CPU == STICK
  pinMode      (DISPLAY_ON, OUTPUT);
  digitalWrite(DISPLAY_ON,HIGH);
#endif
  // Initialize Pins
  digitalWrite (PIN_SPEAKER_OUTPUT_P, LOW);
  digitalWrite (PIN_SPEAKER_OUTPUT_N, HIGH); 

  // set Interrupts (on pin change), attach interrupt handler
  attachInterrupt (digitalPinToInterrupt (PIN_HV_CAP_FULL_INPUT), isr_GMZ_capacitor_full, RISING);// Capacitor full
  attachInterrupt (digitalPinToInterrupt (PIN_GMZ_count_INPUT), isr_GMZ_count, FALLING);          // GMZ pulse detected

  // set and init Serial Communication 
  if (Serial_Print_Mode != Serial_None) {
    noInterrupts();
    Serial.begin(115200);
    while (!Serial) {};
  }
  
  Serial.printf("Los Gehts! \n");
  uint32_t xx = getESPchipID();

  // build SSID
  sprintf(ssid,"ESP32-%d",xx);
  
  // Check, if we have an BME280 connected:
  haveBME280 = bme.begin();
//  Serial.printf("BME_Status: %d  ID:%0X\n", haveBME280, bme.sensorID());

  // Setup IoTWebConf
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setThingName(ssid);
  iotWebConf.init();


  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });


  if (Serial_Print_Mode == Serial_Logging) {
    // Write Header of Table	
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    Serial.print  ("Simple Arduino Geiger, Version ");
    Serial.println(revString);
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    Serial.println("GMZ_counts\tTime_difference\tCount_Rate\tDose_Rate\tHV Pulses  \tAccu_GMZ  \tAccu_Time \tAccu_Rate         \tAccu_Dose");
    Serial.println("[Counts]  \t[ms]           \t[cps]     \t[uSv/h]  \t[-]        \t[Counts]  \t[ms]      \t[cps]             \t[uSv/h]");
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    interrupts();	  
  }

  if (Serial_Print_Mode == Serial_One_Minute_Log) {
    // Write Header of Table	
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    Serial.print  ("Simple Arduino Geiger, Version ");
    Serial.println(revString);
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    Serial.println("Time\tCounte-Rate\tCounts");
    Serial.println("[sec]\t[cpm]\t[Counts per last measurment]");
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    interrupts();	  
  }

  if (Serial_Print_Mode == Serial_Statistics_Log) {
    // Write Header of Table	
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    Serial.print  ("Simple Arduino Geiger, Version ");
    Serial.println(revString);
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    Serial.println("Time between two impacts");
    Serial.println("[usec]");
    Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
    interrupts();	  
  }

#if SEND2LORA
  // init LoRa
  lorawan_setup();
#endif

  // StartScreen 
  DisplayStartscreen();
  displayIsClear = false;

	jb_HV_gen_charge__chargepules();
  if(playSound) {
    SoundStartsound();
  }	
  afterStartTime = AFTERSTART;
  toSendTime = millis();
} // end of setup

// ===================================================================================================================================
// ===================================================================================================================================
void loop()
{
  // read out values from ISR
  noInterrupts();                                                  // disable Interrupts to be able to read the variables of the ISR correctly
  GMZ_counts     	     = isr_GMZ_counts     ;                    // copy values from ISR
  count_timestamp	     = isr_count_timestamp;      
  interrupts();                                                    // re-enable Interrupts	

/*
// DEBUG DEBUG DEBUG
 uint8_t temp_farenheit= temprature_sens_read();
  //convert farenheit to celcius
  double temp = ( temp_farenheit - 32 ) / 1.8;
  
  Serial.printf("internal temp [°C]: %.0f\n", temp);
  delay(1000);
*/

  // Loop for IoTWebConf
  iotWebConf.doLoop();


  // make LED-Flicker and speaker tick
  if (GMZ_counts != last_GMZ_counts) {  
    if(ledTick) {
      digitalWrite(LED_BUILTIN, HIGH);    // switch on LED
    }
    if(speakerTick) {                              
      for (speaker_count = 0; speaker_count <= 3; speaker_count++){ // make "Tick"-Sound
        digitalWrite (PIN_SPEAKER_OUTPUT_P, HIGH); 
        digitalWrite (PIN_SPEAKER_OUTPUT_N, LOW); 
        delayMicroseconds(500); 
        digitalWrite (PIN_SPEAKER_OUTPUT_P, LOW); 
        digitalWrite (PIN_SPEAKER_OUTPUT_N, HIGH); 
        delayMicroseconds(500);	
      }
    } 
    if(ledTick & !speakerTick) {
      delay(4);
    }
    if(ledTick) {
      digitalWrite(LED_BUILTIN, LOW);                                // switch off LED
    }
    last_GMZ_counts = GMZ_counts;                              // notice old value
  }



  // Check if there are enough pules detected or if enough time has elapsed. If yes than its 
  // time to charge the HV-capacitor and calculate the pulse rate  
  if ((GMZ_counts>=100) || ((count_timestamp - last_count_timestamp)>=10000)) {  
    isr_GMZ_counts           = 0;                                      // initialize ISR values
    time_difference = count_timestamp - last_count_timestamp;          // Calculate all derived values
    last_count_timestamp     = count_timestamp;                        // notice the old timestamp
    HV_pulse_count           = jb_HV_gen_charge__chargepules();        // Charge HV Capacitor    
    accumulated_time         += time_difference;                       // accumulate all the time
    accumulated_GMZ_counts   += GMZ_counts;                            // accumulate all the pulses
    lastMinuteLogCounts      += GMZ_counts;  
    counts_p_interval        += GMZ_counts;                            // counts to be sent to luftdaten.info

	  Count_Rate   = (float)GMZ_counts*1000.0/(float)time_difference;    // calculate the current coutrate
    Dose_Rate = Count_Rate *GMZ_factor_uSvph;                          // ... and dose rate
	
                                                                       // calculate the radiation over the complete 
                                                                       // time from start
  	accumulated_Count_Rate   = (float)accumulated_GMZ_counts*1000.0/(float)accumulated_time; 
    accumulated_Dose_Rate    = accumulated_Count_Rate *GMZ_factor_uSvph;
 
	  // Write it to the display
    if(showDisplay) {
      DisplayGMZ(((int)accumulated_time/1000), (int)(accumulated_Dose_Rate*1000), (int)(Count_Rate*60));
      displayIsClear = false;
    } else {
      if (!displayIsClear) {
        u8x8.clear();
        displayIsClear = true;
      }
    }
	


    if (Serial_Print_Mode == Serial_Logging) {                       // Report all 
      noInterrupts();
      Serial.print(GMZ_counts, DEC); 
      Serial.print("\t");    
      Serial.print(time_difference, DEC); 
      Serial.print("\t");    
      Serial.print(Count_Rate, 2); 
      Serial.print("\t");    
      Serial.print(Dose_Rate, 2); 
      Serial.print("\t");    
      Serial.print(HV_pulse_count, DEC); 
      Serial.print("\t");    
      Serial.print(accumulated_GMZ_counts, DEC); 
      Serial.print("\t");    
      Serial.print(accumulated_time, DEC); 
      Serial.print("\t");    
      Serial.print(accumulated_Count_Rate, DEC); 
      Serial.print("\t");    
      Serial.println(accumulated_Dose_Rate, DEC); 
      interrupts();
    }
    if (Serial_Print_Mode == Serial_One_Minute_Log) {              // 1 Minute Log active?  
      if (millis() > (lastMinuteLog + 60000)){                     // Time for a 1-Minute-Log?
        noInterrupts();	  
		    lastMinuteLogCountRate = ( (lastMinuteLogCounts*60000) / (millis()-lastMinuteLog) );    // = *60 /1000
		    if( ( ( ( (lastMinuteLogCounts*60000) % (millis()-lastMinuteLog) ) * 2 ) / (millis()-lastMinuteLog) ) >= 1 ) {
		      lastMinuteLogCountRate++;                                // Rounding
		    }
        Serial.print((millis()/1000), DEC); 
        Serial.print("\t");
        Serial.print(lastMinuteLogCountRate, DEC);    // = *60 /1000        +0.5: to reduce rounding-errors
        Serial.print("\t");
        Serial.println(lastMinuteLogCounts, DEC); 		
	      lastMinuteLogCounts = 0;
        lastMinuteLog       = millis();
        interrupts();
	    }
    }
  }
 
  if ((Serial_Print_Mode == Serial_Statistics_Log) && (counts_before != isr_GMZ_counts)) {              // Statistics Log active?  
    noInterrupts();	  
    Serial.println(isr_count_time_between, DEC);
    counts_before = isr_GMZ_counts;	
    interrupts();	  
  }	

  unsigned long tdiff;
  // If there were no Pulses after 3 sec after start,
  // clear display anyway and show 0 counts
  if(afterStartTime && ((millis()-toSendTime) >= afterStartTime)) {
    afterStartTime = 0;
    if(showDisplay) {
      DisplayGMZ(((int)accumulated_time/1000), (int)(accumulated_Dose_Rate*1000), (int)(Count_Rate*60));
      displayIsClear = false;
    }
  }


  // Check, if we have to send to luftdaten.info
  tdiff = millis() - toSendTime;
  if(tdiff >= (MESSINTERVAL*1000) ) {
    toSendTime = millis();
    int cpm = (int)(counts_p_interval*60000/tdiff);
    if (haveBME280) {
      t = bme.readTemperature();
      h = bme.readHumidity();
      p = bme.readPressure();
      Serial.printf("Gemessen: T=%.2f H=%.f P=%.f\n",t,h,p);
    }
    #if SEND2DUMMY
    displayStatusLine(F("Toilet"));
    Serial.printf("tdiff %ld, count: %d, cpm: %d\n",tdiff,counts_p_interval,cpm);
    Serial.println("SENDING TO TOILET");
    sendData2toilet(true,cpm);
    if (haveBME280) {
      sendData2toilet(false,0,t,h,p);
    }
    delay(300);
    #endif

    #if SEND2MADAVI
    Serial.println("sending to madavi ...");
    displayStatusLine(F("Madavi"));
    sendData2madavi(true,cpm);
    if (haveBME280) {
    sendData2madavi(false,0,t,h,p);
    }
    delay(300);
    #endif

    #if SEND2LUFTDATEN
    Serial.println("sending to luftdaten ...");
    displayStatusLine(F("Luftdaten"));
    sendData2luftdaten(true,cpm);
    if (haveBME280) {
    sendData2luftdaten(false,0,t,h,p);
    }
    delay(300);
    #endif

    // if we are LoRa, then send datas to TTN
    #if SEND2LORA
    Serial.println("sending to TTN ...");
    displayStatusLine(F("TTN"));
    if(haveBME280) {
      sendData2TTN(cpm,t,h,p);
    } else {
      sendData2TTN(cpm);
    }
    #endif

    displayStatusLine(" ");
    counts_p_interval = 0;

    // Read the switch
    Serial.printf("SW0: %d  SW1: %d  SW2: %d  SW3: %d\n",digitalRead(PIN_SWI_0),digitalRead(PIN_SWI_1),digitalRead(PIN_SWI_2),digitalRead(PIN_SWI_3));
  }

  // Read switch

}

// ===================================================================================================================================
// ===================================================================================================================================
// Subfunctions

// GMZ-Sub-Functions
int jb_HV_gen_charge__chargepules() {
  int  chargepules  = 0;
       GMZ_cap_full = 0;
  do {
    digitalWrite(PIN_HV_FET_OUTPUT, HIGH);              // turn the HV FET on
	delayMicroseconds(1500);                            // 5000 usec gives 1,3 times more charge, 500 usec gives 1/20 th of charge
    digitalWrite(PIN_HV_FET_OUTPUT, LOW);               // turn the HV FET off
    delayMicroseconds(1000);	                        
	chargepules++;                                      
  }                                                     
  while ( (chargepules < 1000) && !GMZ_cap_full);       // either Timeout or capacitor full stops this loop
  return chargepules;
}

// ===================================================================================================================================
// OLED-Subfunctions
void DisplayStartscreen(void){
  u8x8.clear();

#if CPU == STICK
  // Print the upper Line including Time and measured radation
  u8x8.setFont(u8x8_font_5x8_f);     // small size
  for (int i=2; i<6; i++) {
      u8x8.drawString(0,i,"        ");
  }
  u8x8.drawString(0,2,"Geiger-"); 
  u8x8.drawString(0,3," Counter");
  char rv[9];
  strncpy(rv,revString,4);
  rv[4] = '\0';
  u8x8.drawString(2,4,rv);
  strncpy(rv,&revString[5],4);
  strncpy(&rv[4],&revString[10],2);
  strncpy(&rv[6],&revString[13],2);
  rv[8] = '\0';
  u8x8.drawString(0,5,rv);
#else  
  // Print the upper Line including Time and measured radation
  u8x8.setFont(u8x8_font_7x14_1x2_f);     // middle size
//  u8x8.setFont(u8x8_font_5x8_f);     // middle size
                                          
  u8x8.println("Geiger-Counter"); 
  u8x8.println("==============");
  u8x8.println(revString);
  u8x8.println("Info:boehri.de");
#endif
};

// ===================================================================================================================================
void DisplayGMZ(int TimeSec, int RadNSvph, int CPS){
  u8x8.clear();
#if CPU != STICK
  int TimeMin=0;
  // Print the upper Line including Time and measured radation
  u8x8.setFont(u8x8_font_7x14_1x2_f);     // middle size
                                          
  TimeMin=TimeSec/60;                     // calculate number of Minutes

  if (TimeMin){                           // Display in Minutes
    if(TimeMin>=999) TimeMin=999;         // limit stop at 999
    if(TimeMin <   100) u8x8.print(" ");
    if(TimeMin <    10) u8x8.print(" ");
    u8x8.print(TimeMin);                  // Arduino Print function	
  } else {                                // Display in Seconds
    u8x8.inverse();
    if(TimeSec <   100) u8x8.print(" ");
    if(TimeSec <    10) u8x8.print(" ");
    u8x8.print(TimeSec);                 
	u8x8.noInverse();
  }
  if(RadNSvph < 1000000) u8x8.print(" ");
  if(RadNSvph <  100000) u8x8.print(" ");
  if(RadNSvph <   10000) u8x8.print(" ");
  if(RadNSvph <    1000) u8x8.print(" ");
  if(RadNSvph <     100) u8x8.print(" ");
  if(RadNSvph <      10) u8x8.print(" ");
  u8x8.print(RadNSvph);                  // Arduino Print function	
  u8x8.print(" nSv/h");
#endif  
 
  // Print the lower Line including Time CPM
#if CPU != STICK
  u8x8.setFont(u8x8_font_inb33_3x6_n);  // Big in Size
  u8x8.drawString(0,2,nullFill(CPS,5));
#else
  u8x8.setFont(u8x8_font_5x8_f);     // small size
  u8x8.drawString(0,2,nullFill(RadNSvph,8));
  u8x8.draw2x2String(0,3,nullFill(CPS,4));
  u8x8.drawString(0,5,"     cpm");
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
void displayStatusLine(String txt) {
    u8x8.setFont(u8x8_font_5x8_f);      // small size
    u8x8.setCursor(0, 7);               // goto last line
    u8x8.print(F("                "));	// clear line
    u8x8.setCursor(0, 7);               // goto last line
    u8x8.print(txt);			              // print it
}
#else
void displayStatusLine(String txt) {
  String blank = F("        ");
  u8x8.setFont(u8x8_font_5x8_f);          // small size
  u8x8.drawString(0,5,blank.c_str());	    // clear line
  u8x8.drawString(0,5,txt.c_str());			  // print it
}
#endif
// ===================================================================================================================================
// Sound Subfunctions
void SoundStartsound(){
  float freq_factor = 0.75;    // Adaption-Factors
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
    
  cycle_time_us   = 1000000000/frequency_mHz;       // calculate all we need
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
  }
  while(millis()<count_timestamp_end);     
  return;
}

// ===================================================================================================================================
// Send to Server Subfunctions

String buildhttpHeaderandBodySBM(HTTPClient *head, int wert, boolean addname) {
  head->addHeader("Content-Type", "application/json; charset=UTF-8");
  head->addHeader("X-PIN","19");
  String chipID = String(ssid);
  chipID.replace("ESP32","esp32");
  head->addHeader("X-Sensor",chipID);
  head->addHeader("Connection","close");
  String rohr = ROHRNAME;
  String valuetype = (addname ? rohr.substring(10)+"_" : "");
  valuetype += "counts_per_minute";
  String body = "{\"software_version\":\""+String(revString)+"\",\
  \"sensordatavalues\":[{\"value_type\":\""+valuetype+"\",\
  \"value\":\""+String(wert)+"\"}]}";                       //Build the actual POST request
  if (DEBUG_SERVER_SEND == 1) {
    Serial.println(body);
  }
  return body;
}

String buildhttpHeaderandBodyBME(HTTPClient *head, float t, float h, float p, boolean addname) {
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
  String body = "{\"software_version\":\""+String(revString)+"\",\
  \"sensordatavalues\":[\
{\"value_type\":\""+temp+"\",\"value\":\""+String(t,2)+"\"},\
{\"value_type\":\""+humi+"\",\"value\":\""+String(h,2)+"\"},\
{\"value_type\":\""+press+"\",\"value\":\""+String(p,2)+"\"}\
]}";                     
  if (DEBUG_SERVER_SEND == 1) {
    Serial.println(body);
  }
  return body;
}

void sendData2luftdaten(bool sendwhat, int wert, float t, float h, float p) {
  HTTPClient http;   
  String body;
  http.begin(LUFTDATEN);
  if (sendwhat) {                                 // send SBM data 
    body = buildhttpHeaderandBodySBM(&http,wert,false);
  } else {                                        // send BME data
    body = buildhttpHeaderandBodyBME(&http,t,h,p,false);
  }  
  int httpResponseCode = http.POST(body);                   //Send the actual POST request
  if(httpResponseCode>0){
    String response = http.getString();                       //Get the response to the request
    if (DEBUG_SERVER_SEND == 1) {
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
   http.end(); 
}

void sendData2madavi(bool sendwhat, int wert, float t, float h, float p) {
  HTTPClient http;   
  String body;
  http.begin(MADAVI);
  if (sendwhat) {                                 // send SBM data 
    body = buildhttpHeaderandBodySBM(&http,wert,true);
  } else {                                           // send BME data
if (haveBME280) { 
    body = buildhttpHeaderandBodyBME(&http,t,h,p,true);
}    
  }  
  int httpResponseCode = http.POST(body);                   //Send the actual POST request
  if(httpResponseCode>0){
    String response = http.getString();                       //Get the response to the request
    if (DEBUG_SERVER_SEND == 1) {
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
   http.end(); 
}

void sendData2toilet(bool sendwhat, int wert, float t, float h, float p) {
  HTTPClient http;   
  String body;
  http.begin(TOILET);
  if (sendwhat) {                                 // send SBM data 
    body = buildhttpHeaderandBodySBM(&http,wert,false);
  } else {                                           // send BME data
if (haveBME280) { 
    body = buildhttpHeaderandBodyBME(&http,t,h,p,true);
}    
  }  
  int httpResponseCode = http.POST(body);                   //Send the actual POST request
  if(httpResponseCode>0){
    String response = http.getString();                      //Get the response to the request
    if (DEBUG_SERVER_SEND == 1) {
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
   http.end(); 
}

#if SEND2LORA
void sendData2TTN(int wert, float t, float h, float p) {
  unsigned char ttnData[20];
  int cnt;
  // put data for Cayenne
  ttnData[0] = 1;
  ttnData[1] = 0x65;
  ttnData[2] = wert >> 8;
  ttnData[3] = wert & 0xFF;
  cnt = 4;
  if (haveBME280) {
    ttnData[4] = 2;
    ttnData[5] = 0x67;
    ttnData[6] = ((int)(t*10)) >> 8;
    ttnData[7] = ((int)(t*10)) & 0xFF;
    ttnData[8] = 3;
    ttnData[9] = 0x68;
    ttnData[10] = (int)(h*2);
    ttnData[11] = 4;
    ttnData[12] = 0x73;
    ttnData[13] = ((int)(p/10)) >> 8;
    ttnData[14] = ((int)(p/10)) & 0xFF;
    cnt = 15;
  }
  lorawan_send(1,ttnData,cnt,false,NULL,NULL,NULL);
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
    // -- Captive portal request were already served.
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
  static char erg[9];                          // max. 8 Digits possible !
  if (digits > 8) {
    digits = 8;
  }
  char format[5];
  sprintf(format,"%%%dd",digits);
  sprintf(erg,format,n);
  return erg;
}
