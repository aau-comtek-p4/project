#include "general/misc/shutdown.h"
#include "common/io/second_process_io.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"

#include <cstdlib>
#include <cstring>
#include <execinfo.h>
void print_backtrace() {
  void *buf[64];
  int n = backtrace(buf, 64);
  char **syms = backtrace_symbols(buf, n);
  for (int i = 1; i < n - 2; i++) {
    char *start = strstr(syms[i], "(");
    char *end = strstr(syms[i], ")");
    end[0] = 0;
    program_ctxt->logger->log_entry(logging::log_backtrace((start + 1)));
  }
  free(syms);
}
void safe_shutdown(ErrorWrapper err) {
  program_ctxt->logger->submit();
  print_backtrace();
  program_ctxt->logger->submit();
  program_ctxt->logger->log_entry(logging::log_shutdown(err));
  program_ctxt->metrics->print_total_metrics();
  program_ctxt->logger->submit();
  second_process_io_signal_end(err);
  exit(1);
}
