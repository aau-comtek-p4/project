#include "general/misc/shutdown.h"
#include "general/common.h"

#include "general/interfaces/utility/logger.h"
#include "general/interfaces/utility/metrics.h"
#include "general/misc/errors.h"
#include <cstdlib>

void safe_shutdown(ErrorWrapper err) {
  program_ctxt->logger->log_err(
      "PROGRAM", "Safe shutdown triggered, error: [%s]", custom_strerror(err));
  program_ctxt->metrics->print_metrics();
  exit(1);
}
