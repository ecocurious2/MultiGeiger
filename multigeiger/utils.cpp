// Misc. utilities

#include <Arduino.h>
#include <WString.h>
#include "log.h"
#include "log_data.h"
#include "utils.h"

// ** convert hexstring to len bytes of data
// returns 0 on success, -1 on error
// data is a buffer of at least len bytes
// hexstring is upper or lower case hexadecimal, NOT prepended with "0x"
// convert hexstring to len bytes of data
// returns 0 on success, -1 on error
// data is a buffer of at least len bytes
// hexstring is upper or lower case hexadecimal, NOT prepended with "0x"
int hex2data(unsigned char *data, const char *hexstring, unsigned int len) {
  const char *pos = hexstring;
  char *endptr;
  size_t count = 0;
  if ((hexstring[0] == '\0') || (strlen(hexstring) % 2)) {
    // hexstring contains no data
    // or hexstring has an odd length
    return -1;
  }
  for (count = 0; count < len; count++) {
    char buf[5] = {'0', 'x', pos[0], pos[1], 0};
    data[count] = strtol(buf, &endptr, 0);
    pos += 2 * sizeof(char);
    if (endptr[0] != '\0') {
      // non-hexadecimal character encountered
      return -1;
    }
  }
  return 0;
}

// reverse a bytearray of even length len
void reverseByteArray(unsigned char *data, int len) {
  char temp;
  for (int i = 0; i < len / 2; i++) {
    temp = data[i];
    data[i] = data[len - i - 1];
    data[len - i - 1] = temp;
  }
}

/*****************************************************************
 * Debug output                                                  *
 *****************************************************************/

LoggingSerial Debug;

LoggingSerial::LoggingSerial()
    : HardwareSerial(0) {
	m_buffer = xQueueCreate(XLARGE_STR, sizeof(uint8_t));
}

size_t LoggingSerial::write(uint8_t c){
	xQueueSendToBack(m_buffer, ( void * ) &c, ( TickType_t ) 0);
	return HardwareSerial::write(c);
}

size_t LoggingSerial::write(const uint8_t *buffer, size_t size){
	for(int i = 0; i < size; i++) {
		xQueueSendToBack(m_buffer, ( void * ) &buffer[i], ( TickType_t ) 0);
	}
	return HardwareSerial::write(buffer, size);
}

String LoggingSerial::popLines(){
	String r;
	uint8_t c;
	while (xQueueReceive(m_buffer, &(c ), (TickType_t) 0 )) {
		r += (char) c;

		if (c == '\n' && r.length() > 10)
			break;
	}
	return r;
}
void LoggingSerial::Reset(){
	xQueueReset(m_buffer);
	write_log_header();
}

//taken from Luftdaten.info
String delayToString(unsigned time_ms) {

	char buf[64];
	String s;

	if (time_ms > 2 * 1000 * 60 * 60 * 24) {
		sprintf_P(buf, PSTR("%d days, "), time_ms / (1000 * 60 * 60 * 24));
		s += buf;
		time_ms %= 1000 * 60 * 60 * 24;
	}

	if (time_ms > 2 * 1000 * 60 * 60) {
		sprintf_P(buf, PSTR("%d hours, "), time_ms / (1000 * 60 * 60));
		s += buf;
		time_ms %= 1000 * 60 * 60;
	}

	if (time_ms > 2 * 1000 * 60) {
		sprintf_P(buf, PSTR("%d min, "), time_ms / (1000 * 60));
		s += buf;
		time_ms %= 1000 * 60;
	}

	if (time_ms > 2 * 1000) {
		sprintf_P(buf, PSTR("%ds, "), time_ms / 1000);
		s += buf;
	}

	if (s.length() > 2) {
		s = s.substring(0, s.length() - 2);
	}

	return s;
}