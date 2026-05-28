#ifndef ESP32_INIT_UART_H
#define ESP32_INIT_UART_H
#include "hal/uart_types.h"
#include <cstdint>
struct self_uart_init_config {
  uart_port_t uart_port;
  int32_t baudrate;
  uint64_t tx_size;
  uint64_t rx_size;
  uint64_t queue_size;
  int rx_pin;
  int tx_pin;
};
void self_uart_init(self_uart_init_config conf);

#endif
