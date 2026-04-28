#ifndef CONTEXT_LINUX_H
#define CONTEXT_LINUX_H

#include "common/io/io.h"
#include "common/io/transports/network/liburing_wifi_udp_transport.h"
#include "common/io/transports/storage/liburing_file_write.h"
#include "common/io/transports/storage/serial_transport.h"
#include "common/logger/file_logger.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/storage/allocators/arena_allocator.h"
#include "general/interfaces/storage/queue.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/loggers/dummy_logger.h"
#include "general/interfaces/utility/loggers/fwrite_logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/names.h"
#include "general/misc/shutdown.h"
#include <cstdint>
#include <cstdio>
#include <liburing.h>

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
  }
  switch (ctx_config.logger_type) {
  case CtxtLoggerType::FILE_LOGGER: {
    using queue_type = Queue<LogEntry, ctx_config.settings.max_log_amount>;
    auto *queue_ptr =
        (queue_type *)allocator->allocate(sizeof(queue_type)).value();
    new (queue_ptr) queue_type(NAME_LOGGING_QUEUE);
    auto file_logger_ptr =
        (FileLogger *)allocator->allocate(sizeof(FileLogger)).value();
    new (file_logger_ptr) FileLogger(queue_ptr, serializer_ptr);
    ctxt->logger = file_logger_ptr;
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
    fprintf(stderr, "ESP queue logger not allowed on linux");
    safe_shutdown(ErrorWrapper{.error = 1, .tag = ErrorWrapper::CUSTOM});
    return;
  }
  }
}

template <typename Config>
void innit_io(ProgramContext *ctxt, ContextConfig<Config> ctx_config,
              AllocatorInterface *allocator) {
  io_uring_queue_init(ctx_config.settings.max_queue_depth, &program_uring, 0);
  uint8_t *transport_buffer =
      (uint8_t *)allocator->allocate(ctx_config.settings.max_io_transport_size)
          .value();
  ArenaAllocator *io_transport_allocator =
      (ArenaAllocator *)allocator->allocate(sizeof(ArenaAllocator)).value();
  new (io_transport_allocator)
      ArenaAllocator(NAME_IO_ALLOCATOR, transport_buffer,
                     ctx_config.settings.max_io_transport_size);
  IOHandler *io_handler_ptr =
      (IOHandler *)allocator->allocate(sizeof(IOHandler)).value();
  new (io_handler_ptr) IOHandler(io_transport_allocator);
  ctxt->io = io_handler_ptr;
  new (ctxt->io->register_transport(IOMethod::IO_FILE,
                                    sizeof(LiburingFileWriteIOTransport)))
      LiburingFileWriteIOTransport;
  new (ctxt->io->register_transport(
      IOMethod::IO_SERIAL, sizeof(SerialIOTransport))) SerialIOTransport;
  new (ctxt->io->register_transport(IOMethod::IO_WIFI_UDP,
                                    sizeof(LiburingWIFIUDPTransport)))
      LiburingWIFIUDPTransport;
  /*
  new (program_ctxt->io->register_transport(
      IOMethod::IO_FILE, sizeof(BlockingFileWriteIOTransport)))
      BlockingFileWriteIOTransport(1024);
          */
  return;
}

#endif
