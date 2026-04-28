#include "cc.h"
#include "driver/uart.h"
#include "endian.h"
#include "esp32/common.h"
#include "esp32/common/context.h"
#include "esp32/io/intializers/wifi_innit.h"
#include "esp32/io/io.h"
#include "esp32/io/polling/uart_polling.h"
#include "esp32/io/polling/wifi_udp_polling.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_log_write.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_rom_crc.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/awaiters/sleep_for.h"
#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/event_loop/coroutines/task.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/transports/storage_transport.h"
#include "general/interfaces/io/transports/udp_transport.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/context_innit.h"
#include "general/misc/crc.h"
#include "general/misc/shutdown.h"
#include "hal/uart_types.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <machine/endian.h>
#include <netinet/in.h>
#define TAG "TAG"
#define UART_RX_BUF_SIZE (1024)
#define UART_TX_BUF_SIZE (8 * 1024)

static void self_uart_init(uart_port_t uart_port, int32_t baudrate) {
  uart_config_t uart_config = {
      .baud_rate = baudrate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(uart_port, UART_RX_BUF_SIZE,
                                      UART_TX_BUF_SIZE, 20,
                                      &uart_queues[uart_port], 0));
  ESP_ERROR_CHECK(uart_param_config(uart_port, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_port, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_pattern_queue_reset(uart_port, 20));
}
static void self_esp_now_innt(void) { ESP_ERROR_CHECK(esp_now_init()); }

void task1(void *args) {
  uint8_t byte = 'a';

  while (true) {
    if (uart_read_bytes(UART_NUM_0, &byte, 1, portMAX_DELAY)) {
      if (byte == 'Z') {
        break;
      }
    }
    vTaskDelay(1);
  }
  esp_started = true;
  while (!context_initialized) {
    vTaskDelay(1);
  }
  while (true) {
    poll_uart();
    poll_wifi_udp();
    vTaskDelay(1);
  }
}

Job metric_printer() {
  auto self_ctx = co_await get_ctxt();
  self_ctx->set_name(NAME_END + 2);
  self_ctx->trace.start();
  while (true) {
    program_ctxt->metrics->print_metrics();
    co_await sleep_for(5000);
  }
}
Job reader() {
  auto self_ctx = co_await get_ctxt();
  self_ctx->set_name(NAME_END + 3);
  self_ctx->trace.start();
  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();
  auto transport =
      program_ctxt->io->get_transport<StorageIOTransport>(IOMethod::IO_SERIAL);
  IOAddress port_addr{.addr_type = IOAddress::UART_PORT,
                      .uart_port = UART_NUM_0};
  uint32_t crc;
  while (true) {
    auto res = co_await transport->io_read(port_addr, buf, 1024);
    if (!res.has_value()) {
      program_ctxt->logger->log_entry(
          logging::log_debug("Failed to read UART"));
      continue;
    }
    program_ctxt->logger->log_entry(
        logging::log_debug("Read %i bytes", res.value()));
    if (res.value() != sizeof(UARTLogFrame)) {
      program_ctxt->logger->log_entry(logging::log_debug("Wrong size"));
      continue;
    }
    auto frame = (UARTLogFrame *)buf;
    if (frame->magic != UART_MAGIC_HEADER) {
      program_ctxt->logger->log_entry(logging::log_debug("Wrong magic"));
      continue;
    }
    crc = esp_rom_crc32_le(CRC32_START, (uint8_t *)frame,
                           sizeof(UARTLogFrame) - sizeof(uint32_t));
    if (crc != frame->check) {
      program_ctxt->logger->log_entry(logging::log_debug("Wrong CRC"));
      continue;
    }
    if (frame->entry.reason != LogReason::REASON_DEBUG) {
      continue;
    }
    program_ctxt->logger->log_entry(
        logging::log_debug("Got: %s", frame->entry.payload.debug.debug));
  }
}
Job udp_test() {
  auto self_ctx = co_await get_ctxt();
  self_ctx->set_name(NAME_END + 4);
  self_ctx->trace.start();
  auto udp_transport =
      program_ctxt->io->get_transport<UDPIOTransport>(IOMethod::IO_WIFI_UDP);
  IOAddress fd_addr{.addr_type = IOAddress::FILE_DESCRIPTOR, .fd = -1};
  IOAddress port_addr = {};
  port_addr.sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  port_addr.sockaddr.sin_port = htons(8080);
  port_addr.sockaddr.sin_family = AF_INET;
  port_addr.addr_type = IOAddress::IO_SOCKADDR;

  auto res = co_await udp_transport->initialize(&fd_addr, &port_addr);
  if (!res.has_value()) {
    safe_shutdown(res.error());
  }
  program_ctxt->logger->log_entry(logging::log_debug("FD: %i", fd_addr.fd));
  IOAddress recv_addr = {};
  recv_addr.addr_type = IOAddress::IO_SOCKADDR;
  auto buf = (uint8_t *)program_ctxt->buffer_allocator->allocate(1024).value();
  char addr_buf[INET6_ADDRSTRLEN] = {0};
  while (true) {
    auto res =
        co_await udp_transport->recv_from(fd_addr, &recv_addr, buf, 1024);
    printf("Got connection\n");
    if (!res.has_value()) {
      program_ctxt->logger->log_entry(logging::log_debug("Recv failed"));
      safe_shutdown(res.error());
    }
    inet_ntop(AF_INET, &recv_addr.sockaddr.sin_addr, addr_buf, INET_ADDRSTRLEN);
    printf("From: %s\n", addr_buf);
    printf("Port: %u\n", ntohs(recv_addr.sockaddr.sin_port));
    printf("Got: ");
    for (int i = 0; i < res.value(); i++) {
      printf("%x ", buf[i]);
    }
    printf("\n");
  }
}
void task2(void *args) {
  while (!esp_started) {
    vTaskDelay(1);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
  ContextConfig<ESPContextSettings> ctx_config;
  ctx_config.clock_type = CtxtClockType::WALL;
  ctx_config.ctx_type = ContextType::NODE;
  ctx_config.logger_type = CtxtLoggerType::FWRITE_LOGGER;
  ctx_config.log_serializer_type = CtxtLoggerSerializer::JSON_SERIALIZER;
  ctx_config.random_type = CtxtRandomType::NONE;
  ctx_config.deadline_tracker_type = CtxtDeadlineTrackerType::MIN_HEAP;
  ProgramContext ctxt;
  uint8_t *total_buffer =
      (uint8_t *)calloc(ctx_config.settings.max_total_size, sizeof(uint8_t));
  if (!total_buffer) {
    ESP_LOGE(TAG, "Could not allocate buffer\n");
    return;
  }
  ArenaAllocator total_allocator(NAME_PROGRAM_ALLOCATOR, total_buffer,
                                 ctx_config.settings.max_total_size);
  program_ctxt = &ctxt;
  innit_ctx(&ctxt, ctx_config, &total_allocator);

  program_ctxt->name_lookup->set_name(NAME_END, "Tester");
  program_ctxt->name_lookup->set_name(NAME_END + 1, "Printer");
  program_ctxt->name_lookup->set_name(NAME_END + 2, "Metric");
  program_ctxt->name_lookup->set_name(NAME_END + 3, "Reader");
  program_ctxt->name_lookup->set_name(NAME_END + 4, "UDP test");
  initialize_wifi_station((uint8_t *)CONFIG_ESP_WIFI_SSID,
                          (uint8_t *)CONFIG_ESP_WIFI_PASSWORD);
  if (!wifi_initialized) {
    program_ctxt->logger->log_entry(logging::log_debug("WIFI failed"));
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
  }
  program_ctxt->logger->log_entry(logging::log_debug("WIFI connected"));
  uint64_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  program_ctxt->logger->log_entry(
      logging::log_debug("Free memory: %llu", free_memory));
  UBaseType_t remaining = uxTaskGetStackHighWaterMark(NULL);
  program_ctxt->logger->log_entry(
      logging::log_debug("Stack: %u", remaining * 4));

  auto _ = spawn(metric_printer());
  // auto _ = spawn(test());
  //_ = spawn(reader());
  _ = spawn(udp_test());

  _ = program_ctxt->loop->run(0);
  program_ctxt->logger->submit();
  program_ctxt->metrics->print_metrics();
  for (int i = 0; i < 10; i++) {
    program_ctxt->logger->log_entry(logging::log_debug("STOP"));
  }
  program_ctxt->logger->submit();
  while (true) {
    vTaskDelay(1);
  }
}

void innit_sqe(uint64_t sqe_size) {
  cqe_queue = xQueueCreate(sqe_size * 2, sizeof(ESPIOCqe));
  uint64_t sqe_queue_size =
      (sqe_size / 2) / SQE_MAX_QUEUES_TOTAL / IOMethod::IO_FILE;
  sqe_no_retry = xQueueCreate(sqe_size / 2, sizeof(ESPIOSqe));

  for (uint64_t queue_index = 0; queue_index < SQE_MAX_QUEUES_TOTAL;
       queue_index++) {
    if (queue_index < SQE_MAX_QUEUES_UART) {
      sqe_uart_queue[queue_index] =
          xQueueCreate(sqe_queue_size, sizeof(ESPIOSqe));
    } else if (queue_index < SQE_MAX_QUEUES_UART + SQE_MAX_QUEUES_FD) {
      wifi_udp_tracker.fd_queue[queue_index] =
          xQueueCreate(sqe_queue_size, sizeof(ESPIOSqe));
    } else if (queue_index < SQE_MAX_QUEUES_UART + SQE_MAX_QUEUES_FD * 2) {
      wifi_tcp_tracker.fd_queue[queue_index] =
          xQueueCreate(sqe_queue_size, sizeof(ESPIOSqe));
    } else if (queue_index < SQE_MAX_QUEUES_UART + SQE_MAX_QUEUES_FD * 2 +
                                 SQE_MAX_QUEUES_PEERS) {
      sqe_esp_now_queue[queue_index] =
          xQueueCreate(sqe_queue_size, sizeof(ESPIOSqe));
    }
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
  self_uart_init(UART_NUM_0, CONFIG_UART0_BAUDRATE);
  start_wifi_innit();
  self_esp_now_innt();
  innit_sqe(1024);
  esp_started = false;
  context_initialized = false;
  log_count = 0;
  wifi_initialized = false;

  xTaskCreatePinnedToCore(task1, "Task1", 4096, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(task2, "Task2", 16384, NULL, 5, NULL, 1);
}
