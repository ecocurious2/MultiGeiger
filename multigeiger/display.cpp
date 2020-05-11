// OLED display related code

#include <Arduino.h>
#include <U8x8lib.h>

#include "version.h"
#include "log.h"
#include "userdefines.h"

#include "display.h"

#define PIN_DISPLAY_ON 25

#define PIN_OLED_RST 16
#define PIN_OLED_SCL 15
#define PIN_OLED_SDA 4

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(PIN_OLED_RST, PIN_OLED_SCL, PIN_OLED_SDA);
U8X8_SSD1306_64X32_NONAME_HW_I2C u8x8_lora(PIN_OLED_RST, PIN_OLED_SCL, PIN_OLED_SDA);
U8X8 *pu8x8;

bool displayIsClear;
static bool isLoraBoard;

void display_start_screen(void) {
  char line[20];

  pu8x8->clear();
  if (isLoraBoard) {
    // Display is only 4 lines by 8 characters; lines counting from 2 to 5
    pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);        // u8x8 can handle only 8x8, smaller font has no advantage
    for (int i = 2; i < 6; i++) {
      pu8x8->drawString(0, i, "        ");  // clear all 4 lines
    }
  }

  if (isLoraBoard) {
    pu8x8->drawString(0, 2, "Geiger-");
    pu8x8->drawString(0, 3, " Counter");
    pu8x8->drawString(0, 4, "Version:");
    snprintf(line, 9, "%s", VERSION_STR);  // 8 chars + \0 termination
    pu8x8->drawString(0, 5, line);
  } else {
    pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);
    pu8x8->drawString(0, 0, "Geiger-Counter");
    pu8x8->drawString(0, 2, "==============");
    snprintf(line, 15, "%s", VERSION_STR);  // 14 chars + \0 termination
    pu8x8->drawString(0, 4, line);
    pu8x8->drawString(0, 6, "Info:boehri.de");
  }
  displayIsClear = false;
};

void setup_display(bool loraHardware) {
  isLoraBoard = loraHardware;
  if (isLoraBoard) {
    pu8x8 = &u8x8_lora;
    pinMode(PIN_DISPLAY_ON, OUTPUT);
    digitalWrite(PIN_DISPLAY_ON, HIGH);
  } else {
    pu8x8 = &u8x8;
  }
  pu8x8->begin();
  display_start_screen();
}

void clearDisplayLine(int line) {
  const char *blanks;
  blanks = isLoraBoard ? "        " : "                "; // 8 / 16
  pu8x8->drawString(0, line, blanks);
}

void displayStatusLine(String txt) {
  if (txt.length() == 0)
    return;
  int line = isLoraBoard ? 5 : 7;
  pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);
  clearDisplayLine(line);
  pu8x8->drawString(0, line, txt.c_str());
}

char *nullFill(int n, int digits) {
  static char erg[9];  // max. 8 digits possible!
  if (digits > 8) {
    digits = 8;
  }
  char format[5];
  sprintf(format, "%%%dd", digits);
  sprintf(erg, format, n);
  return erg;
}

void DisplayGMC(int TimeSec, int RadNSvph, int CPM, bool use_display, bool connected, bool ble_active) {
  if (!use_display) {
    if (!displayIsClear) {
      pu8x8->clear();
      clearDisplayLine(4);
      clearDisplayLine(5);
      displayIsClear = true;
    }
    return;
  }

  pu8x8->clear();

  if (!isLoraBoard) {
    char output[80];
    int TimeMin = TimeSec / 60;         // calculate number of minutes
    TimeMin = TimeMin % 1000;  // limit minutes to max. 999, roll over

    // print the upper line including time and measured radation
    pu8x8->setFont(u8x8_font_open_iconic_embedded_1x1);
    pu8x8->print(ble_active ? '\x4A' : ' '); // 0x4A corresponds to Bluetooth symbol in selected font.
    pu8x8->setFont(u8x8_font_7x14_1x2_f);

    if (TimeSec < 60)    {    // < 1 minute -> display in seconds
      sprintf(output, "%2ds", TimeSec);
      pu8x8->print(output);
    } else {                  // >= 1 minute -> display in minutes
      sprintf(output, "%3d", TimeMin);
      pu8x8->print(output);
    }
    sprintf(output, "%6d nSv/h", RadNSvph);
    pu8x8->print(output);
    pu8x8->setFont(u8x8_font_inb33_3x6_n);
    pu8x8->drawString(0, 2, nullFill(CPM, 5));
  } else {
    // print the upper line including BLE symbol in case of connection and measured radation
    pu8x8->setFont(u8x8_font_open_iconic_embedded_1x1);
    pu8x8->drawString(0, 2, ble_active ? "\x4A" : " "); // 0x4A corresponds to Bluetooth symbol in selected font.
    pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);
    pu8x8->drawString(1, 2, nullFill(RadNSvph, 7));
    pu8x8->setFont(u8x8_font_px437wyse700b_2x2_f);
    pu8x8->drawString(0, 3, nullFill(CPM, 4));
    pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);
    pu8x8->drawString(0, 5, "     cpm");
  }
  displayStatusLine(connected ? "" : "connecting...");
  displayIsClear = false;
};
