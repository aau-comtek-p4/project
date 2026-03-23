#include "general/misc/shutdown.h"
#include "general/common.h"

#include "general/interfaces/utility/logger.h"
#include "general/misc/errors.h"
#include <cstdlib>

void safe_shutdown(int err) {
  program_logger->log_err("PROGRAM", "Safe shutdown triggered, error: [%s]",
                          custom_strerror(err));
  exit(1);
}
