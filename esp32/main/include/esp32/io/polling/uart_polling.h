#ifndef UART_POLLING_H
#define UART_POLLING_H
#include "freertos/idf_additions.h"
#include "general/interfaces/io/io.h"
#include "hal/uart_types.h"
#include <cstdint>
namespace esp_io {
void prep_sqe_read(uart_port_t uart_port, uint8_t *buf, uint64_t buf_size,
                   const void *data);
void prep_sqe_write(uart_port_t uart_port, const uint8_t *buf,
                    uint64_t buf_size, const void *data,
                    IOPackageType pacage_type, bool with_ack);
} // namespace esp_io

#endif
