//===========================================================================================
// Project: Simple Multi-Geiger, Code for Debugging
// (c) 2019 by the authors, see AUTHORS file in toplevel directory.
// License: see LICENSE file in toplevel directory.
//
// Revision History:
//const char* revString = "2019_03_25";   // Juergen Boehringer      Version 1 for ESP32-Board
const char* revString = "2019_05_07";   // Juergen Boehringer      added 1 Minute RS232-Logging-Mode
//
//===========================================================================================

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


// IOs
// ==========================================================
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

// Parameters
bool debug                  = false;// false: no debug output
                                    // true : debug output will be printed via RS232(USB)
bool OneMinuteLogging       = true; // false: no One Minute logging
                                    // true:  One Minute logging will be printed via RS232(USB)
bool speaker_tick           = true; // false: no GMZ-Tick, no flickering LED
                                    // true : GMZ-Tick will be sounded, LED flickers every GMZ-Tick

// Constants
const            float GMZ_factor_uSvph     = 1/2.47   ; // for SBM20
//const            float GMZ_factor_uSvph     = 1/9.81888   ; // for SBM19

									
// Variables
volatile bool          GMZ_cap_full           = 0;
volatile unsigned char isr_GMZ_counts         = 0;
volatile unsigned long isr_count_timestamp    = millis(); 
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
         unsigned char lastMinuteLogCounts    = 0;
 
         
// ISRs
void isr_GMZ_capacitor_full() {
  GMZ_cap_full = 1;
}

void isr_GMZ_count() {
  isr_GMZ_counts++;                                     // Count
  isr_count_timestamp = millis();                       // notice (System)-Time of the Count
}


// Function Prototypes
int jb_HV_gen_charge__chargepules();
void DisplayGMZ(int TimeSec, int RadNSvph, int CPS);
void SoundStartsound();	
void jbTone(unsigned int frequency_mHz, unsigned int time_ms, unsigned char volume);






// Type of OLED-Display
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

void setup()
{
	// OLED-Display
    u8x8.begin();

    // set IO-Pins
    pinMode      (LED_BUILTIN,         OUTPUT);  
    pinMode      (PIN_HV_FET_OUTPUT  , OUTPUT);    
    pinMode      (PIN_SPEAKER_OUTPUT_P,OUTPUT);
    pinMode      (PIN_SPEAKER_OUTPUT_N,OUTPUT);
    
    // Initialize Pins
    digitalWrite (PIN_SPEAKER_OUTPUT_P, LOW);
    digitalWrite (PIN_SPEAKER_OUTPUT_N, HIGH); 
    
    // set Interrupts (on pin change), attach interrupt handler
    attachInterrupt (digitalPinToInterrupt (PIN_HV_CAP_FULL_INPUT), isr_GMZ_capacitor_full, RISING);// Capacitor full
    attachInterrupt (digitalPinToInterrupt (PIN_GMZ_count_INPUT), isr_GMZ_count, FALLING);          // GMZ pulse detected
	
    // set and init Serial Communication (if required)
    if (debug == true) {
      noInterrupts();
      Serial.begin(115200);
      while (!Serial) {};
  	                              // while(1){
                                  //   digitalWrite(PIN_HV_FET_OUTPUT, HIGH);
	                              //   delayMicroseconds(1500);              									
                                  //   digitalWrite(PIN_HV_FET_OUTPUT, LOW); 
	                              //   delayMicroseconds(1500);              									
                                  // };
                                 // pinMode      (0,         OUTPUT);  
                                 // pinMode      (2,         OUTPUT);  
								 // while(1){
                                 //   digitalWrite(0, LOW);  
                                 //   digitalWrite(2, HIGH);  
                                 //   delayMicroseconds(1000);
                                 //   digitalWrite(0, HIGH);   
                                 //   digitalWrite(2, LOW);   
                                 //   delayMicroseconds(1000);	                        
                                 // }

	
      // Write Header of Table	
      Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
      Serial.print  ("Simple Multi-Geiger, Version ");
      Serial.println(revString);
      Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
      Serial.println("GMZ_counts\tTime_difference\tCount_Rate\tDose_Rate\tHV Pulses  \tAccu_GMZ  \tAccu_Time \tAccu_Rate         \tAccu_Dose");
	  Serial.println("[Counts]  \t[ms]           \t[cps]     \t[uSv/h]  \t[-]        \t[Counts]  \t[ms]      \t[cps]             \t[uSv/h]");
      Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
      interrupts();	  
    }
	
    // set and init Serial Communication (if required)
    if (OneMinuteLogging == true) {
      noInterrupts();
      Serial.begin(115200);
      while (!Serial) {};

      // Write Header of Table	
      Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
      Serial.print  ("Debug-Sketch, Version ");
      Serial.println(revString);
      Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
      Serial.println("Time\tCounte-Rate\tCounts");
	  Serial.println("[sec]\t[cpm]\t[Counts per last measurment]");
      Serial.println("----------------------------------------------------------------------------------------------------------------------------------------------------");	  
      interrupts();	  
    }
	
    DisplayStartscreen();
	jb_HV_gen_charge__chargepules();
    SoundStartsound();	
}

// =================================================================
// OLED-Functions
void pre(void)
{
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);    
  u8x8.clear();

  u8x8.inverse();
  u8x8.print(" U8x8 Library ");
  u8x8.setFont(u8x8_font_chroma48medium8_r);  
  u8x8.noInverse();
  u8x8.setCursor(0,1);
}

void draw_bar(uint8_t c, uint8_t is_inverse)
{	
  uint8_t r;
  u8x8.setInverseFont(is_inverse);
  for( r = 0; r < u8x8.getRows(); r++ )
  {
    u8x8.setCursor(c, r);
    u8x8.print(" ");
  }
}

void draw_ascii_row(uint8_t r, int start)
{
  int a;
  uint8_t c;
  for( c = 0; c < u8x8.getCols(); c++ )
  {
    u8x8.setCursor(c,r);
    a = start + c;
    if ( a <= 255 )
      u8x8.write(a);
  }
}
// =================================================================



void loop()
{
  // read out values from ISR
  noInterrupts();                                                  // disable Interrupts to be able to read the variables of the ISR correctly
  GMZ_counts     	     = isr_GMZ_counts     ;                    // copy values from ISR
  count_timestamp	     = isr_count_timestamp;                    
  interrupts();                                                    // re-enable Interrupts	

  // make LED-Flicker and speaker tick
  if (speaker_tick && (GMZ_counts != last_GMZ_counts)) {  
    digitalWrite(LED_BUILTIN, HIGH);                              // switch on LED
    for (speaker_count = 0; speaker_count <= 3; speaker_count++){ // make "Tick"-Sound
      digitalWrite (PIN_SPEAKER_OUTPUT_P, HIGH); 
      digitalWrite (PIN_SPEAKER_OUTPUT_N, LOW); 
      delayMicroseconds(500); 
      digitalWrite (PIN_SPEAKER_OUTPUT_P, LOW); 
      digitalWrite (PIN_SPEAKER_OUTPUT_N, HIGH); 
      delayMicroseconds(500);	
    }
    digitalWrite(LED_BUILTIN, LOW);                                // switch off LED
  }

    jb_HV_gen_charge__chargepules();        // Charge HV Capacitor    
	
	// Write it to the display
    DisplayGMZ(0,0,0);
}



// GMZ-Sub-Functions
int jb_HV_gen_charge__chargepules() {
  int  chargepules  = 0;
  do {
    digitalWrite(PIN_HV_FET_OUTPUT, HIGH);              // turn the HV FET on
	delayMicroseconds(1500);                            // 5000 usec gives 1,3 times more charge, 500 usec gives 1/20 th of charge
    digitalWrite(PIN_HV_FET_OUTPUT, LOW);               // turn the HV FET off
    delayMicroseconds(1000);	                        
	chargepules++;                                      
  }                                                     
  while (chargepules < 100);
  return chargepules;
}


// OLED-Subfunctions
void DisplayStartscreen(void){
  u8x8.clear();
  
  // Print the upper Line including Time and measured radation
  u8x8.setFont(u8x8_font_7x14_1x2_f);     // middle size
                                          
  u8x8.println("Geiger-Counter"); 
  u8x8.println("==============");
  u8x8.print  ("Ver :"); 
  u8x8.println(revString);
  u8x8.println("Info:boehri.de");
  return;
};


void DisplayGMZ(int TimeSec, int RadNSvph, int CPS){
  u8x8.clear();
  
  // Print the upper Line including Time and measured radation
  u8x8.setFont(u8x8_font_7x14_1x2_f);     // middle size
  u8x8.print("Cap         GMZ");
  
  // Print the lower Line including Time CPM
  u8x8.setFont(u8x8_font_inb33_3x6_n);  // Big in Size
  u8x8.setCursor(0, 2);

  if(GMZ_cap_full){
    u8x8.print("*");
  } else {
    u8x8.print("_");
  }

    u8x8.print("   ");
  
  if(isr_GMZ_counts){
    u8x8.print("*");
  } else {
    u8x8.print("_");
  }
  
  // Reset ISR-Values
  GMZ_cap_full = 0;
  isr_GMZ_counts = 0;
  return;
};


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


