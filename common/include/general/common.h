#ifndef GENERAL_COMMON_H
#define GENERAL_COMMON_H
#define MAX_U8_NUM 255
#define TESTING 1
class LoggerInterface;
class ClockInterface;
class EventLoopInterface;
class AllocatorInterface;
class DeadlineStorageInterface;
class IOHandler;
class MetricsInterface;
class RandomInterface;

struct ProgramContext;

inline ProgramContext *program_ctxt = nullptr;

#endif
