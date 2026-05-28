#ifndef CONTEXT_LINUX_H
#define CONTEXT_LINUX_H

#include "common/io/second_process_io.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/io/polling_io.h"
#include "general/interfaces/io/transports/busy_pollings_transports.h"
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
  case CtxtLoggerType::SECOND_PROCESS_LOGGER: {
    using queue_type = Queue<LogEntry, ctx_config.settings.max_log_amount>;
    auto *queue_ptr =
        (queue_type *)allocator->allocate(sizeof(queue_type)).value();
    new (queue_ptr) queue_type;
    auto file_logger_ptr = (SecondProcessIOFileLogger *)allocator
                               ->allocate(sizeof(SecondProcessIOFileLogger))
                               .value();
    new (file_logger_ptr) SecondProcessIOFileLogger(queue_ptr, serializer_ptr);
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
  auto io_handler_ptr =
      (BusyPollingIO *)allocator->allocate(sizeof(BusyPollingIO)).value();
  new (io_handler_ptr) BusyPollingIO(io_transport_allocator);
  io_handler_ptr->sqe_configs[IOMethod::IO_WIFI_TCP] =
      new (allocator->allocate(sizeof(TCPSQEConfig)).value()) TCPSQEConfig;
  io_handler_ptr->sqe_configs[IOMethod::IO_WIFI_UDP] =
      new (allocator->allocate(sizeof(UDPSQEConfig)).value()) UDPSQEConfig;
  io_handler_ptr->sqe_configs[IOMethod::IO_FILE] =
      new (allocator->allocate(sizeof(FileSQEConfig)).value()) FileSQEConfig;
  io_handler_ptr->sqe_configs[IOMethod::IO_SERIAL] =
      new (allocator->allocate(sizeof(SerialSQEConfig)).value())
          SerialSQEConfig;
  ctxt->io = io_handler_ptr;
  new (ctxt->io->register_transport(IOMethod::IO_WIFI_TCP,
                                    sizeof(BusyPollingTCPTransport)))
      BusyPollingTCPTransport;
  new (ctxt->io->register_transport(IOMethod::IO_FILE,
                                    sizeof(BusyPollingFileTransport)))
      BusyPollingFileTransport;

  new (ctxt->io->register_transport(IOMethod::IO_SERIAL,
                                    sizeof(BusyPollingSerialTransport)))
      BusyPollingSerialTransport;

  return;
}

#endif
