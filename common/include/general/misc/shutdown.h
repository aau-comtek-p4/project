#ifndef SHUTDOWN_H

#define SHUTDOWN_H
#include "general/misc/errors.h"

void print_backtrace();
void safe_shutdown(ErrorWrapper err);

#endif
