// show status indication via WS2812 RGB LED

// indication value used to reset indications to "all off"
#define I_RESET 0
// indication value used to mask no bit, e.g. in compute_color
#define I_NONE 0

// indications, values are 32bit bit masks, valid values are 2^N
#define I_TEST 1  // reserved to test the indication code
#define I_HV_ERROR 2  // there is a problem with high voltage generation
#define I_CONN_ERROR 4  // there is a problem with the network connection

// indicate radiation and special indications via a color time sequence.
// you should call this in regular time intervals [e.g. 1s].
void indicate(float radiation, unsigned int indication);

void setup_status_LED(void);
