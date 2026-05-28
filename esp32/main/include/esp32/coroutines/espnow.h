#ifndef COROUTINES_ESPNOW_H
#define COROUTINES_ESPNOW_H
#include "general/interfaces/event_loop/coroutines/job.h"
Job espnow_client_sender_routine();
Job espnow_receiver_routine();

#endif
