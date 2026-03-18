#ifndef GENERAL_COMMON_H
#define GENERAL_COMMON_H

#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/clock.h"
#include <cstddef>
#define MAX_U8_NUM 255

#include "general/interfaces/utility/logger.h"
#include "interfaces/event_loop/event_loop.h"

inline thread_local LoggerInterface *tl_logger = nullptr;
inline thread_local ClockInterface *tl_clock = nullptr;
inline thread_local EventloopInterface *tl_loop = nullptr;
inline thread_local AllocatorInterface *tl_coroutine_frame_allocator = nullptr;
inline thread_local size_t total_coroutine_counter = 0;

void safe_shutdown(int err);

#endif
