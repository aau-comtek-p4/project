
#include "esp32/io/intializers/uart_innit.h"
#include "driver/uart.h"
#include "esp32/io/polling/uart_polling.h"
#include "hal/uart_types.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#define UART_DRIVER_INSTALL_FLAGS 0

void self_uart_init(self_uart_init_config conf) {
  if (uart_is_driver_installed(conf.uart_port)) {
    printf("\nAlready installed port: %u\n", conf.uart_port);
    return;
  }
  uart_config_t uart_config = {
      .baud_rate = conf.baudrate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_param_config(conf.uart_port, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(conf.uart_port, conf.tx_pin, conf.rx_pin,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(conf.uart_port, conf.rx_size,
                                      conf.tx_size, 0, NULL,
                                      UART_DRIVER_INSTALL_FLAGS));
  ESP_ERROR_CHECK(uart_pattern_queue_reset(conf.uart_port, conf.queue_size));
}
