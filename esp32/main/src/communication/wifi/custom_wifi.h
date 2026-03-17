#ifndef CUSTOM_WIFI
#define CUSTOM_WIFI

#include "esp_event_base.h"
#include "esp_wifi_types_generic.h"
#include <cstdint>
#define WIFI_TAG "WIFI"
#define WIFI_ERROR_TAG "WIFI ERROR"
class WIFIStation {
private:
  wifi_config_t wifi_config;
  esp_event_handler_t event_handler;

public:
  WIFIStation(uint8_t ssid[32], uint8_t password[32]);

  int innit();
};

#endif
