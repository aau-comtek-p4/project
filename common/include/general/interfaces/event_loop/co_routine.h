#ifndef CO_ROUTINE_INTERFACE_H
#define CO_ROUTINE_INTERFACE_H
#include <coroutine>
#include <cstdint>

#define COROUTINE_TAG "COROUTINE"
#define COROUTINE_ERR_TAG "COROUTINE ERROR"
class IOAwaitInterface;

struct CoRoutineCtxt {
  bool cancelled = false;
  CoRoutineCtxt *parent_ctxt = nullptr;
  void *self_cancellation = nullptr;
  IOAwaitInterface *io_address = nullptr;
  std::coroutine_handle<> handle;
  uint64_t id;
};

struct shared_promise_type {
  CoRoutineCtxt ctxt;
};
struct GetCtxtAwaiter {
  CoRoutineCtxt *ctxt;
  bool await_ready() { return false; }
  template <typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h) {
    this->ctxt = &h.promise().ctxt;
    return false;
  }
  CoRoutineCtxt *await_resume() { return this->ctxt; }
};

GetCtxtAwaiter get_ctxt();

struct CoRoutineInterface {
public:
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type h;
};

#endif
