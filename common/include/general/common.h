#ifndef GENERAL_COMMON_H
#define GENERAL_COMMON_H
#include <sys/cdefs.h>
#define MAX_U8_NUM 255
#define TESTING 1
class LoggerInterface;
class ClockInterface;
class EventLoopInterface;
class AllocatorInterface;
class DeadlineStorageInterface;
class IOInterface;
class MetricsInterface;
class RandomInterface;

struct ProgramContext {
  LoggerInterface *logger = nullptr;
  ClockInterface *clock = nullptr;
  EventLoopInterface *loop = nullptr;
  AllocatorInterface *frame_allocator = nullptr;
  AllocatorInterface *buffer_allocator = nullptr;
  DeadlineStorageInterface *deadline_tracker = nullptr;
  IOInterface *io = nullptr;
  MetricsInterface *metrics = nullptr;
  RandomInterface *random = nullptr;
};

inline ProgramContext *program_ctxt = nullptr;

#endif
