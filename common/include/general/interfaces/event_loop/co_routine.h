#ifndef CO_ROUTINE_INTERFACE_H
#define CO_ROUTINE_INTERFACE_H
#include <coroutine>

#define COROUTINE_TAG "COROUTINE"
#define COROUTINE_ERR_TAG "COROUTINE ERROR"
class IOAwaitInterface;

struct shared_promise_type {
  bool cancelled = false;
  void *self_cancellation = nullptr;
  IOAwaitInterface *io_address = nullptr;
};

struct CoRoutineInterface {
public:
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;
  handle_type h;
};

#endif
