#include "esp32/io/transports/serial_storage_transport.h"
#include "esp32/io/io.h"
#include "esp32/io/polling/uart_polling.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/names.h"
#include "hal/uart_types.h"
#include <cstdint>
class IOSerialWriteAwaiter : public ESPIOAwaiterInterface {
private:
  const uint8_t *buf;
  uint64_t buffer_size;
  uart_port_t uart_port;

public:
  IOSerialWriteAwaiter(uart_port_t uart_port, const uint8_t *buf,
                       uint64_t buffer_size) {
    this->uart_port = uart_port;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::WRITE;
    this->io_method = IOMethod::IO_SERIAL;
  }
  void submit() override {
    esp_io::prep_sqe_write(this->uart_port, this->buf, this->buffer_size, this);
    ;
  }
};

class IOSerialReadAwaiter : public ESPIOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;
  uart_port_t uart_port;

public:
  IOSerialReadAwaiter(uart_port_t uart_port, uint8_t *buf,
                      uint64_t buffer_size) {
    this->uart_port = uart_port;
    this->buf = buf;
    this->buffer_size = buffer_size;
    this->type = IOType::READ;
    this->io_method = IOMethod::IO_SERIAL;
  }
  void submit() override {
    esp_io::prep_sqe_read(this->uart_port, this->buf, this->buffer_size, this);
    ;
  }
};

void ESPSerialTransport::cancel(const void *user_data) {};
void ESPSerialTransport::submit() {};
void ESPSerialTransport::process(uint64_t timeout) {};

Task<std::expected<int, ErrorWrapper>>
ESPSerialTransport::io_open(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_OPEN);
  self_ctxt->trace.start();
  co_return std::unexpected(
      ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
};
Task<std::expected<int, ErrorWrapper>>
ESPSerialTransport::io_read(IOAddress addr, uint8_t *buf, uint64_t buf_size) {

  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_READ);
  self_ctxt->trace.start();
  auto res = co_await IOSerialReadAwaiter(addr.uart_port, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_SERIAL, IOType::READ, res);
};
Task<std::expected<int, ErrorWrapper>>
ESPSerialTransport::io_write(IOAddress addr, const uint8_t *buf,
                             uint64_t buf_size) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_READ);
  self_ctxt->trace.start();
  auto res = co_await IOSerialWriteAwaiter(addr.uart_port, buf, buf_size);
  co_return process_io_res(self_ctxt, IOMethod::IO_SERIAL, IOType::WRITE, res);
};
Task<std::expected<int, ErrorWrapper>>
ESPSerialTransport::io_close(IOAddress addr) {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_IO_SERIAL_CLOSE);
  self_ctxt->trace.start();
  co_return std::unexpected(
      ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
};
