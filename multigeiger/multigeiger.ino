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
const char* revString = "V1.2_2019_09_02";   // rxf                - sending to madavi corrected

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
//
//====================================================================================================================================


//====================================================================================================================================
// ***************************************
// Durch den User einstellbare Parameter:
// ***************************************
//====================================================================================================================================

// Ausgabe auf der seriellen Schnittstelle (USB)
#define SERIAL_DEBUG Serial_Logging

// Wartezeit, bis Verbindungversuchmit dem gespeicherten WLAN beendet wird [sec]
#define CONNECT_TIMEOUT 30
// So lange in Sekunden wird der lokale Accesspoint aktivitert, um das WLAN zu konfigurieren [sec] (Siehe Docs)
#define WAIT_4_CONFIG 180

// die nächsten 3 Parameter können auch beim Konfigurieren über
// das WLAN eingestellt werden (die WLAN-Einstellungen haben Priorität)
// Speaker ticks  1-> ein,  0-> aus
#define SPEAKER_TICK 1
// weiße LED, die mit den Ticks blinkt
#define LED_TICK  1
// Display
#define SHOW_DISPLAY 1
// Start-Sound
#define PLAY_SOUND 0

// Send to servers:
// Madavi to see values in real time
#define SEND2MADAVI 1
// Luftdaten should always be 1 -> standard server
#define SEND2LUFTDATEN 1

// Print debug-Info while sending to servers
#define DEBUG_SERVER_SEND 0

// kind of used counter tube
#define ROHRNAME SBM20

// *********************************************************************************
// ENDE der durch den User einstellbaren Parameter. Ab hier nichts mehr ändern!
// *********************************************************************************
//====================================================================================================================================
//====================================================================================================================================



//====================================================================================================================================
// Includes
//   #include "WiFi.h"
#include <Arduino.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "IotWebConf.h"
#include <HTTPClient.h>

//====================================================================================================================================
// IOs
//  used for OLED:              4
//  used for OLED:             15
//  used for OLED:             16
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
int PIN_GMZ_count_INPUT     =  17;  // !! has to be capable of "interrupt on change"
int PIN_SPEAKER_OUTPUT_P    =   2;
int PIN_SPEAKER_OUTPUT_N    =   0;

#define  TESTPIN 12

// Messinteravll (default 10min) [sec]
#define MESSINTERVAL 150

// MAX time to wait until connected. After then, meaurement starts but there is no sending to servers
#define MAX_WAIT_TIME 300000          // 5min

// Dummy server for debugging
#define SEND2DUMMY 0

// Config-Version for IoTWebConfig
#define CONFIG_VERSION "001"

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
         unsigned int  counts_p_interval      = 0;

         bool          showDisplay            = SHOW_DISPLAY;
         bool          speakerTick            = SPEAKER_TICK;
         bool          ledTick                = LED_TICK;
         bool          playSound              = PLAY_SOUND;
         bool          displayIsClear         = false;
         char          ssid[30];

int  Serial_Print_Mode      = SERIAL_DEBUG;
         
//====================================================================================================================================
// ISRs
void isr_GMZ_capacitor_full() {
  GMZ_cap_full = 1;
}

void isr_GMZ_count() {
  static unsigned long isr_count_timestamp_us     ;
  static unsigned long isr_count_timestamp_us_prev;
  static unsigned long isr_count_timestamp_us_prev_used;
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
}


//====================================================================================================================================
// Function Prototypes
int jb_HV_gen_charge__chargepules();
void DisplayGMZ(int TimeSec, int RadNSvph, int CPS);
void SoundStartsound();	
void jbTone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume);
void DisplayStartscreen(void);
void sendData2luftdaten(int wert);
void sendData2madavi(int wert);
void sendData2toilet(int wert);
String buildhttpHeaderandBody(HTTPClient *, int, bool);
void displayStatusLine(String txt);

void handleRoot(void);
void configSaved(void);


// Type of OLED-Display
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ 16, /* clock=*/ 15, /* data=*/ 4);
// U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "ESP32Geiger";
const char* theName = "Defaultname fuer die SSID     ";       // 30 chars log !      

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(theName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

//====================================================================================================================================
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
  pinMode      (TESTPIN,             OUTPUT);    


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
  
  uint64_t esp_chipid = ESP.getEfuseMac();
  Serial.printf("Los Gehts! \n");
  Serial.printf("MAC: %04X%08X\n",(uint16_t)(esp_chipid>>32),(uint32_t)esp_chipid);
  
  // build SSID
  sprintf(ssid,"ESP32-%d",(uint32_t)(esp_chipid >> 24));
  
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

  // StartScreen 
  DisplayStartscreen();
  displayIsClear = false;

	jb_HV_gen_charge__chargepules();
  if(playSound) {
    SoundStartsound();
  }	

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


  // Loop for IoTWebConf
  iotWebConf.doLoop();


  // make LED-Flicker and speaker tick
  if (GMZ_counts != last_GMZ_counts) {  
    digitalWrite(TESTPIN,HIGH);
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
    digitalWrite(TESTPIN,LOW);
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



  // Check, if we have to send to luftdaten.info
  unsigned long tdiff;
  tdiff = millis() - toSendTime;
  if(tdiff >= (MESSINTERVAL*1000) ) {
    toSendTime = millis();
    int cpm = (int)(counts_p_interval*60000/tdiff);
    #if SEND2DUMMY
    Serial.printf("tdiff %ld, count: %d, cpm: %d\n",tdiff,counts_p_interval,cpm);
    Serial.println("SENDING TO TOILET");
    sendData2toilet(cpm);
    #endif
    #if SEND2MADAVI
    Serial.println("sending to madavi ...");
    displayStatusLine(F("Madavi"));
    sendData2madavi(cpm);
    delay(300);
    #endif
    #if SEND2LUFTDATEN
    Serial.println("sending to luftdaten ...");
    displayStatusLine(F("Luftdaten"));
    sendData2luftdaten(cpm);
    delay(300);
    #endif
    displayStatusLine(" ");
    counts_p_interval = 0;

  }

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
  
  // Print the upper Line including Time and measured radation
  u8x8.setFont(u8x8_font_7x14_1x2_f);     // middle size
                                          
  u8x8.println("Geiger-Counter"); 
  u8x8.println("==============");
  u8x8.println(revString);
  u8x8.println("Info:boehri.de");
  return;
};

// ===================================================================================================================================
void DisplayGMZ(int TimeSec, int RadNSvph, int CPS){
  int TimeMin=0;

  u8x8.clear();
  
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
  
 
  // Print the lower Line including Time CPM
  u8x8.setFont(u8x8_font_inb33_3x6_n);  // Big in Size
  u8x8.setCursor(0, 2);
  if(CPS< 10000) u8x8.print(" ");  // to always have the 1-digit on the same place
  if(CPS<  1000) u8x8.print(" ");
  if(CPS<   100) u8x8.print(" ");
  if(CPS<    10) u8x8.print(" ");
  u8x8.print(CPS);			// Arduino Print function
  
  // Print 'connecting...' as long as we aren't connected
  if (iotWebConf.getState() != IOTWEBCONF_STATE_ONLINE) {
    displayStatusLine(F("connecting..."));
  } else {
    displayStatusLine(" ");
  }
  return;
};

void displayStatusLine(String txt) {
    u8x8.setFont(u8x8_font_5x8_f);      // small size
    u8x8.setCursor(0, 7);               // goto last line
    u8x8.print(F("                "));	// clear line
    u8x8.setCursor(0, 7);               // goto last line
    u8x8.print(txt);			              // print it
}
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

String buildhttpHeaderandBody(HTTPClient *head, int wert, boolean addname) {
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

void sendData2luftdaten(int wert) {
  HTTPClient http;   
  http.begin(LUFTDATEN);
  String body = buildhttpHeaderandBody(&http,wert,false);
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

void sendData2madavi(int wert) {
  HTTPClient http;   
  http.begin(MADAVI);
  String body = buildhttpHeaderandBody(&http,wert,true);
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

void sendData2toilet(int wert) {
  HTTPClient http;   
  http.begin(TOILET);
  String body = buildhttpHeaderandBody(&http,wert,false);
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
