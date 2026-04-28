
#include "esp32/io/intializers/wifi_innit.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/shutdown.h"
#include "sdkconfig.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#define WIFI_CONNECT_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < WIFI_CONNECT_MAXIMUM_RETRY) {
      program_ctxt->logger->log_entry(
          logging::log_debug("Retrying WIFI Connect"));
      vTaskDelay(pdMS_TO_TICKS(100));
      esp_wifi_connect();
      s_retry_num++;
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    // ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
void start_wifi_innit(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void initialize_wifi_station(uint8_t *wifi_ssid, uint8_t *wifi_password) {
  s_retry_num = 0;
  s_wifi_event_group = xEventGroupCreate();

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  wifi_config_t wifi_config = {0};
  strncpy((char *)wifi_config.sta.ssid, (char *)wifi_ssid,
          sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, (char *)wifi_password,
          sizeof(wifi_config.sta.password));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  program_ctxt->logger->log_entry(logging::log_debug("ESP WIFI STA started"));
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & WIFI_CONNECTED_BIT) {
    wifi_initialized = true;
  } else if (bits & WIFI_FAIL_BIT) {
    wifi_initialized = false;
  } else {
    program_ctxt->logger->log_entry(
        logging::log_debug("Unexpected WIFI Event"));
  }
}
