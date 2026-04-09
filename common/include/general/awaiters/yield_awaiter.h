#include "general/common.h"
#include "general/interfaces/event_loop/event_loop.h"
#include "general/misc/context.h"
#include <coroutine>
struct YieldAwaiter {
  bool await_ready() { return false; }
  void await_suspend(std::coroutine_handle<> h) {

    auto _ = program_ctxt->loop->enque_staging(h);
  }
  void await_resume() {}
};

YieldAwaiter yield_coroutine();
