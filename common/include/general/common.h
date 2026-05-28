#ifndef GENERAL_COMMON_H
#define GENERAL_COMMON_H
#include <cstdint>
#define MAX_U8_NUM 255
class LoggerInterface;
class ClockInterface;
class EventLoopInterface;
class AllocatorInterface;
class DeadlineStorageInterface;
class IOHandler;
class MetricsInterface;
class RandomInterface;
class TraceHandlerInterface;
class IOConnectionHandler;

struct ProgramContext;

inline ProgramContext *program_ctxt = nullptr;
inline bool context_initialized = false;
inline uint8_t device_id = 255;
inline uint8_t device_type = 255;

enum ProgramState {
  PROGRAM_STATE_ESTABLISHING_CONNECTION,
  PROGRAM_STATE_ESTABLISHED_CONNECTION,

};
inline ProgramState program_state;

#endif
