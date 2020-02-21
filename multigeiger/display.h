// OLED display related code

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void setup_display(void);
void DisplayGMC(int TimeSec, int RadNSvph, int CPS, bool use_display, bool connected);
void clearDisplayLine(int line);
void displayStatusLine(String txt);

#endif // _DISPLAY_H_
