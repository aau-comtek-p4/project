#include "general/awaiters/timeout_awaiter.h"
#include "general/common.h"

Task<int> timeout_routine(const void *cancel_data) {
  auto handle = co_await get_ctxt();
  if (!handle->cancelled) {
    program_ctxt->io->cancel(cancel_data);
    co_return 0;
  }

  co_return 0;
}
