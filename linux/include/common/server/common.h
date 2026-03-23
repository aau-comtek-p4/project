#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H
#include "general/interfaces/storage/allocator.h"
#include "general/interfaces/utility/clock.h"

#define SERVER_NS_PR_TICK NS_PR_MS * 10
void init_globals(AllocatorInterface *allocator);
#endif
