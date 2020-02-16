// OLED display related code
 
#include "userdefines.h"

void setup_display(void);
void DisplayGMC(int TimeSec, int RadNSvph, int CPS, bool use_display, bool connected);
void clearDisplayLine(int line);
void displayStatusLine(String txt);
