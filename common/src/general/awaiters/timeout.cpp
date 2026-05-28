#include "general/awaiters/timeout_awaiter.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/names.h"

Task<int> timeout_routine() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_TIMEOUT_ROUTINE);
  self_ctxt->trace.start();

  if (self_ctxt->cancelled) {
    co_return 0;
  }
  if (self_ctxt->io_address) {
  }

  co_return 0;
}
