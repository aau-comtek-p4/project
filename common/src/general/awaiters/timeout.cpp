#include "general/awaiters/timeout_awaiter.h"
#include "general/common.h"

Task<int> timeout_routine(const void *cancel_data) {
  auto handle = co_await get_ctxt();
  handle->trace->start();

  handle->trace->set_name("Timeout");
  if (!handle->cancelled) {
    program_ctxt->io->cancel(cancel_data);
    handle->trace->suspend_trace();
    co_return 0;
  }
  handle->trace->suspend_trace();
  co_return 0;
}
