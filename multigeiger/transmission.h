// measurements data transmission related code
// - via WiFi to internet servers
// - via LoRa to TTN (to internet servers)

void setup_transmission(const char *version, char *ssid);
void transmit_data(String tube_type, int tube_nbr, unsigned int dt, unsigned int hv_pulses, unsigned int gm_counts, unsigned int cpm,
                   int have_thp, float temperature, float humidity, float pressure);
