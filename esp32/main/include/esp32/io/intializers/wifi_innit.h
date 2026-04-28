#ifndef ESP_WIFI_INNIT_H
#define ESP_WIFI_INNIT_H

#include <cstdint>
inline bool wifi_initialized = false;
void start_wifi_innit(void);
void initialize_wifi_station(uint8_t *wifi_ssid, uint8_t *wifi_password);

#endif
