#ifndef GENERAL_COMMON_H
#define GENERAL_COMMON_H
#include <cstddef>
#define MAX_U8_NUM 255
#define TESTING 1
class LoggerInterface;
class ClockInterface;
class EventLoopInterface;
class AllocatorInterface;
class DeadlineStorageInterface;
class IOInterface;

inline LoggerInterface *program_logger = nullptr;
inline ClockInterface *program_clock = nullptr;
inline EventLoopInterface *program_loop = nullptr;
inline AllocatorInterface *program_coroutine_frame_allocator = nullptr;
inline AllocatorInterface *program_buffer_allocator = nullptr;
inline DeadlineStorageInterface *program_deadline_keeper = nullptr;
inline IOInterface *program_io = nullptr;

inline size_t total_coroutine_counter = 0;

#endif
