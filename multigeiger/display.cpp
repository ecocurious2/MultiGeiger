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

static int status[STATUS_MAX] = {ST_NODISPLAY, ST_NODISPLAY, ST_NODISPLAY, ST_NODISPLAY,
                                 ST_NODISPLAY, ST_NODISPLAY, ST_NODISPLAY, ST_NODISPLAY
                                };  // current status of misc. subsystems

static const char *status_chars[STATUS_MAX] = {
  // group WiFi and transmission to internet servers
  ".W0wA", // ST_WIFI_OFF, ST_WIFI_CONNECTED, ST_WIFI_ERROR, ST_WIFI_CONNECTING, ST_WIFI_AP
  ".s1S",  // ST_SCOMM_OFF, ST_SCOMM_IDLE, ST_SCOMM_ERROR, ST_SCOMM_SENDING
  ".m2M",  // ST_MADAVI_OFF, ST_MADAVI_IDLE, ST_MADAVI_ERROR, ST_MADAVI_SENDING
  // group TTN (LoRa WAN)
  ".t3T",  // ST_TTN_OFF, ST_TTN_IDLE, ST_TTN_ERROR, ST_TTN_SENDING
  // group BlueTooth
  ".B4b",  // ST_BT_OFF, ST_BT_CONNECTED, ST_BT_ERROR, ST_BT_CONNECTING
  // group other
  ".",     // ST_NODISPLAY
  ".",     // ST_NODISPLAY
  ".H7",   // ST_NODISPLAY, ST_HV_OK, ST_HV_ERROR
};

void set_status(int index, int value) {
  if ((index >= 0) && (index < STATUS_MAX))
    status[index] = value;
  else
    log(ERROR, "invalid parameters: set_status(%d, %d)", index, value);
}

char get_status_char(int index) {
  if ((index >= 0) && (index < STATUS_MAX)) {
    int idx = status[index];
    if (idx < strlen(status_chars[index]))
      return status_chars[index][idx];
    else
      log(ERROR, "string status_chars[%d] is too short, no char at index %d", index, idx);
  } else
    log(ERROR, "invalid parameters: get_status_char(%d)", index);
  return '?';  // some error happened
}

void displayStatus(void) {
  char output[17];  // max. 16 chars wide display + \0 terminator
  const char *format = isLoraBoard ? "%c%c%c%c%c%c%c%c" : "%c %c %c %c %c %c %c %c";  // 8 or 16 chars wide
  snprintf(output, 17, format,
           get_status_char(0), get_status_char(1), get_status_char(2), get_status_char(3),
           get_status_char(4), get_status_char(5), get_status_char(6), get_status_char(7)
          );
  displayStatusLine(output);
}

char *format_time(int secs) {
  static char result[10];
  int mins = secs / 60;
  if (secs < 60) {
    sprintf(result, "%2ds", secs);
  } else {
    mins = mins % 1000;
    sprintf(result, "%3d", mins);
  }
  return result;
}

void DisplayGMC(int TimeSec, int RadNSvph, int CPM, bool use_display) {
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

  char tmp[40], output[40];
  if (!isLoraBoard) {
    pu8x8->setFont(u8x8_font_7x14_1x2_f);
    sprintf(output, "%3s%7d nSv/h", format_time(TimeSec), RadNSvph);
    pu8x8->drawString(0, 0, output);
    pu8x8->setFont(u8x8_font_inb33_3x6_n);
    sprintf(output, "%5d", CPM);
    pu8x8->drawString(0, 2, output);
  } else {
    pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);
    sprintf(output, " %7d", RadNSvph);
    pu8x8->drawString(0, 2, output);
    pu8x8->setFont(u8x8_font_px437wyse700b_2x2_f);
    sprintf(output, "%4d", CPM);
    pu8x8->drawString(0, 3, output);
    pu8x8->setFont(u8x8_font_amstrad_cpc_extended_f);
    pu8x8->drawString(0, 5, "     cpm");
  }
  displayStatus();
  displayIsClear = false;
};
