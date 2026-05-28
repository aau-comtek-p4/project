#include "general/misc/shutdown.h"
#include "esp_debug_helpers.h"
#include "freertos/idf_additions.h"
#include "general/common.h"
#include "general/interfaces/utility/logger.h"
#include "general/misc/context.h"
#include <cinttypes>
#include <cstdint>
void print_backtrace() {
  uint32_t pc, sp, next_pc;
  esp_backtrace_get_start(&pc, &sp, &next_pc);
  esp_backtrace_frame_t frame{
      .pc = pc, .sp = sp, .next_pc = next_pc, .exc_frame = nullptr};
  frame.exc_frame = &frame;
  char buf[32] = {};
  int max_write;
  for (int i = 0; i < 32; i++) {
    max_write = snprintf(buf, sizeof(buf), "0x%" PRIx32 "", frame.pc);
    buf[max_write] = 0;

    program_ctxt->logger->log_entry(logging::log_backtrace(buf));

    if (!esp_backtrace_get_next_frame(&frame)) {
      break;
    }
  }
}
void safe_shutdown(ErrorWrapper err) {
  program_ctxt->logger->submit();
  print_backtrace();
  program_ctxt->logger->submit();
  program_ctxt->logger->log_entry(logging::log_shutdown(err));
  program_ctxt->metrics->print_total_metrics();
  for (int i = 0; i < 10; i++) {
    program_ctxt->logger->log_entry(logging::log_debug("STOP"));
  }
  program_ctxt->logger->submit();

  while (1) {
    vTaskDelay(1);
  }
}
