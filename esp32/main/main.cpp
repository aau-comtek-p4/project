#include "driver/uart.h"
#include "endian.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/uart_types.h"
#include "nvs_flash.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <machine/endian.h>
#define TAG "TAG"
#define UART_BUF_SIZE (1024)
#define UART_RD_BUF_SIZE (UART_BUF_SIZE)
#define UART_PORT_NUM UART_NUM_0
#define UART_PATTERN_NUM (3)
static QueueHandle_t uart0_queue;

static void self_uart_init(void) {
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 2,
                                      UART_BUF_SIZE * 2, 20, &uart0_queue, 0));
  ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_enable_pattern_det_baud_intr(UART_PORT_NUM, '+',
                                                    UART_PATTERN_NUM, 9, 0, 0));
  ESP_ERROR_CHECK(uart_pattern_queue_reset(UART_PORT_NUM, 20));
}
static void self_wifi_init(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}
static void self_esp_now_innt(void) { ESP_ERROR_CHECK(esp_now_init()); }
struct UartFrame {
  uint16_t magic;
  uint8_t len;
  char msg[20];
};

void task1(void *args) {
  const char *buf = "Hello mom!!!";
  UartFrame frame{.magic = htobe16(21), .len = (uint8_t)strlen(buf)};
  strncpy(frame.msg, buf, strlen(buf));
  while (true) {
    uart_write_bytes(UART_PORT_NUM, &frame, sizeof(frame));
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
void task2(void *args) {
  uint64_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  ESP_LOGI(TAG, "Free memory: %llu", free_memory);
  while (true) {
    vTaskDelay(1);
  }
}
extern "C" void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  self_uart_init();
  self_wifi_init();
  self_esp_now_innt();
  xTaskCreate(task1, "Task1", 2048, NULL, 5, NULL);
  xTaskCreate(task2, "Task2", 2048, NULL, 5, NULL);
}
