// show status indication via WS2812 RGB LED

#include "userdefines.h"

#include <NeoPixelBus.h>

#include "log.h"
#include "status_led.h"

#define PIXEL_COUNT 1
#define PIXEL_PIN 25

RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);
RgbColor yellow(255, 255, 0);
RgbColor white(255, 255, 255);
RgbColor black(0, 0, 0);

static NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> LEDs(PIXEL_COUNT, PIXEL_PIN);

static RgbColor last_col;

static void set_LED(RgbColor col) {
  if (col == last_col)
    return;  // nothing to change

  LEDs.SetPixelColor(0, col);  // only 1 LED at index 0
  LEDs.Show();
  last_col = col;
}

void setup_status_LED(void) {
  LEDs.Begin();  // all LEDs off
  LEDs.Show();
  last_col = black;  // consistency sw state == hw state
}

static void compute_color(unsigned int indication,
                          unsigned int mask_r, unsigned int mask_g, unsigned int mask_b,
                          RgbColor *color) {
  // assign to *color depending on whether the r/g/b masked bits are set in the indication value
  *color = RgbColor((indication & mask_r) ? 255 : 0,
                    (indication & mask_g) ? 255 : 0,
                    (indication & mask_b) ? 255 : 0);
}

#define IDX_W 2
#define CSL 30

void indicate(float radiation, unsigned int indication) {
  // radiation [uSv/h] given to set the primary color R that is shown most of the time
  // indication: 32 bit flags to indicate additional stuff, see status_led.h.
  //
  // this code will generate a time sequence of LED colors:
  // index     color
  // ------------------------------------------------------
  // 0         dark (LED init)
  // 1         indication color 1
  // ...       reserved for more indications
  // IDX_W     white (LED test)
  // ...+1     radiation color
  // ...       radiation color
  // CSL-1     radiation color

  RgbColor col;
  static int index = 0;  // index counting modulo COLOR_SEQUENCE_LENGTH

  log(INFO, "LED index: %d, radiation: %f", index, radiation);

  switch (index) {
  case 0:  // show a fixed dark separator after LED init
    col = black;
    break;
  case 1:
    // indication color 1:    R             G       B
    compute_color(indication, I_CONN_ERROR, I_TEST, I_HV_ERROR, &col);
    break;
  case IDX_W:  // show a fixed white separator and LED test
    col = white;
    break;
  default:  // show radiation color
    if (radiation > 1.0) {
      col = red;
    } else if (radiation > 0.2) {
      col = yellow;
    } else if (radiation > 0.01) {
      col = green;
    } else {
      col = black;
    }
  }

  set_LED(col);

  if (++index == CSL)
    index = 0;
}
