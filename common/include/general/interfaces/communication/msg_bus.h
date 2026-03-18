#ifndef MSG_BUS_INTERFACE_H
#define MSG_BUS_INTERFACE_H

#include "connection.h"
#include "general/misc/errors.h"
#include <cstddef>
#include <cstdint>
#include <expected>
class MsgBusInterface {
public:
  virtual std::expected<void, SendError> send_to(uint32_t connection_identifier,
                                                 std::byte data) = 0;
  virtual std::expected<void, SendError> send_channel(uint32_t channel_id,
                                                      std::byte data) = 0;
  virtual std::expected<void, ReceiveError>
  recv_from(uint32_t connection_identifier, std::byte *out_buf) = 0;

  virtual std::expected<int, ConnectionError>
  add_connection(ConnectionInterface *connection) = 0;
  virtual std::expected<int, ConnectionError>
  add_connection(ConnectionInterface *connection,
                 uint32_t channel_identifier) = 0;
  virtual std::expected<int, ConnectionError>
  remove_connection(uint32_t connection_identifier) = 0;
  virtual std::expected<int, ConnectionError>
  remove_connection(uint32_t connection_identifier,
                    uint32_t channel_identifier) = 0;
};

#endif
