#include "esp32/io/io.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include <cstdint>
#include <cstdio>
ESPIOHandler::ESPIOHandler(AllocatorInterface *transport_allocator)
    : IOHandler(transport_allocator) {}

IOTransport *ESPIOHandler::register_transport(IOMethod io_method,
                                              uint64_t transport_size) {
  return IOHandler::register_transport(io_method, transport_size);
}

void ESPIOHandler::submit_all() { return IOHandler::submit_all(); }

void ESPIOHandler::process_all(uint64_t timeout) {
  uint64_t start_timeout = timeout;
  uint64_t delay_time = pdTICKS_TO_MS(1) * NS_PR_MS;

  if (start_timeout >= delay_time) {
    vTaskDelay(1);
    start_timeout -= delay_time;
  }
  ESPIOCqe cqe;
  xQueuePeek(cqe_queue, &cqe, pdMS_TO_TICKS(start_timeout / NS_PR_MS));
  uint64_t cqe_ready = uxQueueMessagesWaiting(cqe_queue);
  for (uint64_t i = 0; i < cqe_ready; i++) {
    xQueueReceive(cqe_queue, &cqe, 0);
    auto interface = (ESPIOAwaiterInterface *)cqe.user_data;
    interface->result = cqe.res;
    interface->handle.resume();
  }
}

void ESPIOHandler::cancel(IOMethod io_method, const void *user_data) {
  return IOHandler::cancel(io_method, user_data);
};
const char *ESPIOAwaiterInterface::get_type() {
  return parse_io_type(this->type);
};
bool ESPIOAwaiterInterface::await_ready() { return false; }
bool ESPIOAwaiterInterface::await_suspend(
    std::coroutine_handle<Task<std::expected<int, ErrorWrapper>>::promise_type>
        handle) {
  if (handle.promise().ctxt.cancelled) {
    this->result = -ECANCELED;
    return false;
  }
  handle.promise().ctxt.io_address = this;
  this->handle = handle;
  this->submit();
  return true;
}
int ESPIOAwaiterInterface::await_resume() { return this->result; }
std::expected<void, ErrorWrapper> FDTracker::register_fd(int fd) {
  for (int i = 0; i < SQE_MAX_QUEUES_FD; i++) {
    if (!this->holders[i].active) {
      this->holders[i].fd = fd;
      this->holders[i].active = true;
      return {};
    }
  }
  return std::unexpected(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
}
std::expected<QueueHandle_t, ErrorWrapper> FDTracker::get_queue(int fd) {
  for (int i = 0; i < SQE_MAX_QUEUES_FD; i++) {
    if (this->holders[i].fd == fd && this->holders[i].active) {
      return this->fd_queue[i];
    }
  }
  return std::unexpected(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
}
std::expected<void, ErrorWrapper> FDTracker::close_fd(int fd) {
  for (int i = 0; i < SQE_MAX_QUEUES_FD; i++) {
    if (this->holders[i].fd == fd && this->holders[i].active) {
      this->holders->active = false;
    }
  }
  return std::unexpected(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
}
