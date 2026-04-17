#include "general/misc/shutdown.h"
#include "general/common.h"

#include "general/interfaces/event_loop/event_loops/basic_event_loop.h"
#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/context.h"
#include "general/misc/errors.h"
#include <cstdio>
#include <cstdlib>

void safe_shutdown(ErrorWrapper err) {
  program_ctxt->logger->submit();
  program_ctxt->logger->log_entry(logging::log_shutdown(err));
  program_ctxt->metrics->print_metrics();
  program_ctxt->logger->submit();
  exit(1);
}
