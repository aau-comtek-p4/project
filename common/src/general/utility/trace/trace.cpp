#include "general/interfaces/utility/trace.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include "general/misc/shutdown.h"

TraceHandler::TraceHandler(AllocatorInterface *trace_allocator)
    : trace_allocator(trace_allocator) {}

Trace *TraceHandler::get_trace() {
  auto res = this->trace_allocator->allocate(sizeof(Trace));
  if (!res.has_value()) {
    program_ctxt->logger->log_info(
        TRACE_ERROR_TAG, "Trace handler failed to allocate new trace: [%s]",
        custom_strerror(res.error()));
    safe_shutdown(res.error());
  }
  auto trace_ptr = (Trace *)res.value();
  return trace_ptr;
}
