#ifndef UART_JOBS_H
#define UART_JOBS_H
#include "general/interfaces/event_loop/coroutines/job.h"
Job read_uart();

Job uart_writer();

#endif
