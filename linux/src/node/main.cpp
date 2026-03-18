#include "common/utility/clocks/basic_clock.h"
#include "common/utility/loggers/fprint_logger.h"
#include "general/interfaces/utility/clock.h"
#include "general/interfaces/utility/logger.h"
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <ctime>

int main() {
  BasickClock basic_clock(CLOCK_MONOTONIC, 10000000);
  FPrintLogger logger(&basic_clock);
  logger.log_err("TAG", "Hello %s age %u", "Jhon", 35);
}
