#include "custom_wifi.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include <cstdint>
#include <string.h>

WIFIStation::WIFIStation(uint8_t ssid[32], uint8_t password[32]) {
  memcpy(this->wifi_config.sta.ssid, ssid, sizeof(this->wifi_config.sta.ssid));
  memcpy(this->wifi_config.sta.password, password,
         sizeof(this->wifi_config.sta.password));

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

int WIFIStation::innit() {
  ESP_ERROR_CHECK(!this->event_handler ? ESP_OK : ESP_FAIL);
  esp_event_handler_instance_t instance_any_id;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, this->event_handler, NULL,
      &instance_any_id));

  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, this->event_handler, NULL,
      &instance_got_ip));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(WIFI_TAG, "Successfully initialized WIFI");

  return ESP_OK;
}
