#ifndef CONNECTION_INTERFACE_H
#define CONNECTION_INTERFACE_H

#include "general/misc/errors.h"
#include <cstddef>
#include <expected>

enum ConnectionType { GPIO, SERIAL, NETWORK };

class ConnectionInterface {
public:
  virtual std::expected<void, SendError> send(std::byte *data) = 0;
  virtual std::expected<void, ReceiveError> recv(std::byte *out_buf) = 0;
};

#endif
