

#include "esp32/io/polling/uart_polling.h"
#include "driver/uart.h"
#include "esp32/io/io.h"
#include "freertos/idf_additions.h"
#include "hal/uart_types.h"
#include <cstddef>
#include <cstdint>
void esp_io::prep_sqe_read(uart_port_t uart_port, uint8_t *buf,
                           uint64_t buf_size, const void *data) {
  ESPIOSqe sqe;
  sqe.user_data = data;
  sqe.payload.serial_uart_read.buf = buf;
  sqe.payload.serial_uart_read.buf_size = buf_size;
  xQueueSend(sqe_uart_queue[uart_port], &sqe, 0);
}
void esp_io::prep_sqe_write(uart_port_t uart_port, const uint8_t *buf,
                            uint64_t buf_size, const void *data) {
  ESPIOSqe sqe;
  sqe.user_data = data;
  sqe.payload.serial_uart_write.buf = buf;
  sqe.payload.serial_uart_write.buf_size = buf_size;
  xQueueSend(sqe_uart_queue[uart_port], &sqe, 0);
}

void poll_uart_port(uart_port_t uart_port) {
  ESPIOSqe sqe;
  uart_event_t event;
  size_t bytes_ready;
  while (xQueuePeek(sqe_uart_queue[SQE_UART_OFFSET + uart_port], &sqe, 0)) {
    QueueHandle_t uart_queue = uart_queues[uart_port];
    if (!uart_queue) {
      safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
    }
    if (!xQueuePeek(uart_queue, &event, 0)) {
      return;
    }
    if (event.type == UART_DATA) {
      uart_get_buffered_data_len(uart_port, &bytes_ready);
      if (!bytes_ready) {
        return;
      }
    }
    xQueueReceive(uart_queue, &event, 0);
    xQueueReceive(sqe_uart_queue[SQE_UART_OFFSET + uart_port], &sqe, 0);
    if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL) {
      uart_flush_input(uart_port);
      ESPIOCqe cqe{.res = -1, .user_data = sqe.user_data};
      xQueueSend(cqe_queue, &cqe, 0);
      continue;
    }
    if (event.type != UART_DATA) {
      ESPIOCqe cqe{.res = -1, .user_data = sqe.user_data};
      xQueueSend(cqe_queue, &cqe, 0);
      continue;
    }
    int32_t bytes_read =
        uart_read_bytes(uart_port, sqe.payload.serial_uart_read.buf,
                        sqe.payload.serial_uart_read.buf_size, 0);
    ESPIOCqe cqe{.res = bytes_read, .user_data = sqe.user_data};
    xQueueSend(cqe_queue, &cqe, 0);
  }
}
void poll_uart() {
  for (uint8_t i = 0; i < UART_NUM_MAX; i++) {
    return poll_uart_port((uart_port_t)i);
  }
}
