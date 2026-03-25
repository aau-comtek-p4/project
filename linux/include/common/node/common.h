#ifndef NODE_COMMON_H
#define NODE_COMMON_H
#include "general/common.h"
#include "general/interfaces/storage/allocator.h"
#include <cstddef>

#define NODE_TAG "NODE"
#define NODE_ERROR_TAG "NODE ERROR"
#define CLOCK_MS_PR_TICK 10
#define MAX_STACK_SIZE 1024 * 40

#define MAX_COROUTINE_SIZE 350
#define MAX_COROUTINE_AMOUNT 20
#define MAX_COROUTINE_GENERATOR_SIZE 30
#define MAX_COROUTINE_GENERATOR_AMOUNT 20
#define MAX_READY_QUEUE 20
#define MAX_STAGING_QUEUE 20
#define MAX_BUFFER_SIZE 1024
#define MAX_BUFFER_AMOUNT 20
#define MAX_DEADLINES 20
#define MAX_QUEUE_DEPTH 20

void node_init_ctxt(ProgramContext *ctxt,
                    AllocatorInterface *general_allocator);

#endif
