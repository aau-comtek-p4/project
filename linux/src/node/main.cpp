#include "common/utility/clocks/basic_clock.h"
#include "common/utility/loggers/fprint_logger.h"
#include "general/common.h"
#include "general/interfaces/storage/allocators/bucket_allocator.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include <coroutine>
#include <cstdint>
#include <ctime>
#include <liburing.h>
#include <unistd.h>

int main() {
  BasickClock basic_clock(CLOCK_MONOTONIC, NS_PR_MS * 10);
  tl_clock = &basic_clock;
  FPrintLogger logger;
  tl_logger = &logger;
}
