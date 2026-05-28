#include "driver/uart.h"
#include "esp32/common.h"
#include "esp32/esp32_tasks/test_tasks.h"
#include "esp32/io/intializers/uart_innit.h"
#include "esp32/io/intializers/wifi_innit.h"
#include "esp_debug_helpers.h"
#include "esp_err.h"
#include "esp_now.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "hal/uart_types.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <machine/endian.h>
#include <netinet/in.h>
#define TAG "TAG"
static void self_esp_now_innt(void) { ESP_ERROR_CHECK(esp_now_init()); }

extern "C" void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  if (pdTICKS_TO_MS(1) != 1) {
    fprintf(stderr, "MS pr tick is not 1, got: %" PRIu32, pdTICKS_TO_MS(1));
    vTaskDelay(portMAX_DELAY);
    exit(1);
  }
  self_uart_init_config num0_conf{
      .uart_port = UART_NUM_0,
      .baudrate = CONFIG_UART0_BAUDRATE,
      .tx_size = UART_TX_BUF_SIZE,
      .rx_size = UART_RX_BUF_SIZE,
      .queue_size = UART_QUEUE_SIZE,
      .rx_pin = UART_PIN_NO_CHANGE,
      .tx_pin = UART_PIN_NO_CHANGE,
  };
  self_uart_init(num0_conf);
  self_uart_init_config num1_conf = num0_conf;
  num1_conf.uart_port = UART_NUM_1;
  num1_conf.rx_pin = 25;
  num1_conf.tx_pin = 26;

  self_uart_init(num1_conf);

  self_uart_init_config num2_conf = num0_conf;
  num1_conf.uart_port = UART_NUM_2;
  num1_conf.rx_pin = 27;
  num1_conf.tx_pin = 33;

  self_uart_init(num2_conf);

  start_wifi_innit();
  esp_started = false;
  context_initialized = false;
  wifi_initialized = false;

  xTaskCreatePinnedToCore(io_polling_task, "IO Polling", TASK1_STACK_SIZE, NULL,
                          TASK1_PRIORITY, NULL, TASK1_CORE);

  xTaskCreatePinnedToCore(espnow_test_task, "Task2", TASK2_STACK_SIZE, NULL,
                          TASK2_PRIORITY, NULL, TASK2_CORE);
}
