
#include "esp32/coroutines/misc/statistics.h"
#include "general/awaiters/sleep_for.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/misc/names.h"
Job metric_printer() {
  auto self_ctx = co_await get_ctxt();
  self_ctx->set_name(NAME_METRIC_PRINTER);
  self_ctx->trace.start();
  while (true) {
    program_ctxt->metrics->print_metrics();

    co_await sleep_for(1000);
  }
}
