#include "general/common.h"
#include "general/interfaces/event_loop/co_routine.h"
#include "general/interfaces/utility/logger.h"
#include <cstdint>
GetCtxtAwaiter get_ctxt() { return GetCtxtAwaiter{}; }

void CoRoutineCtxt::set_name(uint64_t name_id) {
  uint64_t parent_id = (this->parent_ctxt) ? this->parent_ctxt->name_id : 0;
  program_ctxt->logger->log_entry(
      logging::log_coroutine_start(name_id, parent_id));
  this->name_id = name_id;
  this->trace->set_name(this->name_id);
}
