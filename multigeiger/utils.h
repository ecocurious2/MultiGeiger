// Misc. utilities

#ifndef UTILS_H
#define UTILS_H
#include <HardwareSerial.h>

// Prototypes
int hex2data(unsigned char *data, const char *hexstring, unsigned int len);
void reverseByteArray(unsigned char *data, int len);
String delayToString(unsigned time_ms);

constexpr unsigned SMALL_STR = 64-1;
constexpr unsigned MED_STR = 256-1;
constexpr unsigned LARGE_STR = 512-1;
constexpr unsigned XLARGE_STR = 1024-1;

#define RESERVE_STRING(name, size) String name((const char*)nullptr); name.reserve(size)

/*****************************************************************
 * Debug output                                                  *
 *****************************************************************/

class LoggingSerial : public HardwareSerial {

public:
	LoggingSerial();
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
	String popLines();
    void Reset();

private:
    QueueHandle_t m_buffer;
};

extern class LoggingSerial Debug;

#endif
