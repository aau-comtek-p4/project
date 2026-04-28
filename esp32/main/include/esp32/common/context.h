#ifndef ESP_CONTEXT_H
#define ESP_CONTEXT_H
#include "esp32/common.h"
#include "esp32/io/io.h"
#include "esp32/io/transports/serial_storage_transport.h"
#include "esp32/io/transports/wifi_udp_transport.h"
#include "esp32/logging/queue_logger.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/loggers/dummy_logger.h"
#include "general/interfaces/utility/loggers/fwrite_logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <cassert>
#include <cstdint>
#include <cstdio>

template <typename Config>
void innit_logger(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
                  AllocatorInterface *allocator) {

  LogSerializerInterface *serializer_ptr;
  switch (ctx_config.log_serializer_type) {
  case CtxtLoggerSerializer::JSON_SERIALIZER:
    serializer_ptr =
        (LogSerializerInterface *)allocator->allocate(sizeof(JsonLogSerializer))
            .value();
    new (serializer_ptr) JsonLogSerializer();
    break;
  default:
    serializer_ptr =
        (LogSerializerInterface *)allocator->allocate(sizeof(JsonLogSerializer))
            .value();
    new (serializer_ptr) JsonLogSerializer();
    break;
  }
  switch (ctx_config.logger_type) {
  case CtxtLoggerType::FILE_LOGGER: {
    fprintf(stderr, "File logger not allowed on linux");
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
    return;
  }
  case CtxtLoggerType::FWRITE_LOGGER: {
    auto logger_ptr =
        (FWriteLogger *)allocator->allocate(sizeof(FWriteLogger)).value();
    new (logger_ptr) FWriteLogger(serializer_ptr);
    ctxt->logger = logger_ptr;
    return;
  }
  case CtxtLoggerType::DUMMY_LOGGER: {
    auto logger_ptr =
        (DummyLogger *)allocator->allocate(sizeof(DummyLogger)).value();
    new (logger_ptr) DummyLogger();
    ctxt->logger = logger_ptr;
    return;
  }
  case CtxtLoggerType::ESP_LOGGER: {
    fprintf(stderr, "ESP logger not allowed on linux");
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
    return;
  }
  case CtxtLoggerType::ESP_QUEUE_LOGGER: {
    using queue_type = Queue<LogEntry, ctx_config.settings.max_log_amount>;
    auto *queue_ptr =
        (queue_type *)allocator->allocate(sizeof(queue_type)).value();
    new (queue_ptr) queue_type(NAME_LOGGING_QUEUE);
    auto logger_ptr =
        (ESPSerialLogger *)allocator->allocate(sizeof(ESPSerialLogger)).value();
    new (logger_ptr) ESPSerialLogger(queue_ptr, serializer_ptr);
    ctxt->logger = logger_ptr;
    return;
  }
  }
}

template <typename Config>
void innit_io(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
              AllocatorInterface *allocator) {
  uint8_t *transport_buffer =
      (uint8_t *)allocator->allocate(ctx_config.settings.max_io_transport_size)
          .value();
  ArenaAllocator *io_transport_allocator =
      (ArenaAllocator *)allocator->allocate(sizeof(ArenaAllocator)).value();
  new (io_transport_allocator)
      ArenaAllocator(NAME_IO_ALLOCATOR, transport_buffer,
                     ctx_config.settings.max_io_transport_size);
  ESPIOHandler *io_handler_ptr =
      (ESPIOHandler *)allocator->allocate(sizeof(IOHandler)).value();
  new (io_handler_ptr) ESPIOHandler(io_transport_allocator);
  new (io_handler_ptr->register_transport(
      IOMethod::IO_SERIAL, sizeof(ESPSerialTransport))) ESPSerialTransport();
  new (io_handler_ptr->register_transport(IOMethod::IO_WIFI_UDP,
                                          sizeof(ESPWIFIUDPTransport)))
      ESPWIFIUDPTransport();
  ctxt->io = io_handler_ptr;
  return;
}

#endif
