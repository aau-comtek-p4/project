
#include "common/coroutines/misc/statistics.h"
#include "common.h"
#include "general/awaiters/sleep_for.h"
#include "general/common.h"
#include "general/interfaces/event_loop/coroutines/job.h"
#include "general/interfaces/io/io.h"
#include "general/misc/names.h"
#include <cstdio>
Job metric_logger() {
  auto self_ctxt = co_await get_ctxt();
  self_ctxt->set_name(NAME_METRIC_PRINTER);
  self_ctxt->trace.start();
  while (true) {
    co_await sleep_for(5000);
    program_ctxt->metrics->print_metrics();
  }
}
