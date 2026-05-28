#include "driver/uart.h"
#include "esp32//esp32_tasks/test_tasks.h"
#include "esp32/common.h"
#include "esp32/io/polling/uart_polling.h"
#include "esp32/io/polling/wifi_tcp_polling.h"
#include "esp32/io/polling/wifi_udp_polling.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include <cstdint>

void io_polling_task(void *args) {
  uint8_t buf[1024];
  while (!context_initialized) {
    vTaskDelay(1);
  }
  while (true) {
    poll_connections(buf);
    poll_general(buf);
    vTaskDelay(1);
  }
}
