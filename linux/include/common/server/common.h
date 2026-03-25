#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include "general/common.h"
#define SERVER_TAG "SERVER"
#define SERVER_ERROR_TAG "SERVER ERROR"
#define CLOCK_MS_PR_TICK 10
#define MAX_STACK_SIZE 1024 * 40

#define SERVER_PORT 7000
#define SERVER_CONNECTION_QUEUE_SIZE 5

#define MAX_COROUTINE_SIZE 300
#define MAX_COROUTINE_AMOUNT 20
#define MAX_COROUTINE_GENERATOR_SIZE 30
#define MAX_COROUTINE_GENERATOR_AMOUNT 20
#define MAX_READY_QUEUE 20
#define MAX_STAGING_QUEUE 20
#define MAX_BUFFER_SIZE 1024
#define MAX_BUFFER_AMOUNT 20
#define MAX_DEADLINES 20
#define MAX_QUEUE_DEPTH 20

void server_init_ctxt(ProgramContext *ctxt,
                      AllocatorInterface *general_allocator);

#endif
