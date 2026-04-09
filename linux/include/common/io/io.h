#ifndef LINUX_IO_H
#define LINUX_IO_H

#include "general/interfaces/io/io.h"
#include "general/misc/errors.h"
#include <cstddef>
#include <cstdint>
#include <liburing.h>

struct IOAddress {
  enum { IO_SOCKADDR, FILE_DESCRIPTOR, FILE_PATH } addr_type;
  union {
    sockaddr_in sockaddr;
    int fd;
    char file_path[30];
  };
};

#endif
