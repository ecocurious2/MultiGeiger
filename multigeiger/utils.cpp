// Misc. utilities

#include <Arduino.h>

#include "log.h"
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
    //hexstring contains no data
    //or hexstring has an odd length
    return -1;
  }
  for (count = 0; count < len; count++) {
    char buf[5] = {'0', 'x', pos[0], pos[1], 0};
    data[count] = strtol(buf, &endptr, 0);
    pos += 2 * sizeof(char);
    if (endptr[0] != '\0') {
      //non-hexadecimal character encountered
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

