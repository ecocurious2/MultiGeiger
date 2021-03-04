// measurements data transmission related code
// - via WiFi to internet servers
// - via LoRa to TTN (to internet servers)

#ifndef _TRANSMISSION_H_
#define _TRANSMISSION_H_

// Sensor-PINS.
// They are called PIN, because in the first days of Feinstaub sensor they were
// really the CPU-Pins. Now they are 'virtual' pins to distinguish different sensors.
// Since we send to sensor.community, we have to use their numbers.
// PIN number 0 doesn't exist, so we use it to disable the X-PIN header.
#define XPIN_NO_XPIN 0
#define XPIN_RADIATION 19
#define XPIN_BME280 11

void setup_transmission(const char *version, char *ssid, bool lora);
void transmit_data(String tube_type, int tube_nbr, unsigned int dt, unsigned int hv_pulses, unsigned int gm_counts, unsigned int cpm,
                   int have_thp, float temperature, float humidity, float pressure, int wifi_status);

// The Arduino LMIC wants to be polled from loop(). This takes care of that on LoRa boards.
void poll_transmission(void);

#endif // _TRANSMISSION_H_
