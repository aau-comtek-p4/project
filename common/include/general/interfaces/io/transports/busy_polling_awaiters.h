#ifndef COMMON_AWAITER_BUSY_POLLING_H
#define COMMON_AWAITER_BUSY_POLLING_H

#include "general/interfaces/io/io.h"
#include <cstdint>

namespace busy_polling_awaiters {
class IOSendAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t buffer_size;
  int fd;
  const uint8_t *buf;
  IOPackageType package_type;
  bool with_ack;

public:
  IOSendAwaiter(int fd, const uint8_t *buf, uint64_t buffer_size,
                IOPackageType package_type, bool with_ack);

  void submit() override;
};

class IORecvAwaiter : public BasicIOAwaiterInterface {
private:
  uint8_t *buf;
  uint64_t buffer_size;
  int fd;

public:
  IORecvAwaiter(int fd, uint8_t *buf, uint64_t buffer_size);
  void submit() override;
};

class IOConnectAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;
  const void *sock_addr;

public:
  IOConnectAwaiter(int fd, const void *sock_addr);
  void submit() override;
};

class IOAcceptAwaiter : public BasicIOAwaiterInterface {
private:
  void *sock_addr;
  int fd;

public:
  IOAcceptAwaiter(int fd, void *sock_addr);
  void submit() override;
};
class IOSendToAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;
  const uint8_t *buf;
  uint64_t buffer_size;
  const void *send_addr;
  IOPackageType package_type;
  bool with_ack;

public:
  IOSendToAwaiter(int fd, const void *send_addr, const uint8_t *buf,
                  uint64_t buffer_size, IOPackageType package_type,
                  bool with_ack);
  void submit() override;
};

class IORecvFromAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;
  uint8_t *buf;
  uint64_t buffer_size;
  void *recv_addr;

public:
  IORecvFromAwaiter(int fd, void *recv_addr, uint8_t *buf,
                    uint64_t buffer_size);
  void submit() override;
};

class IOFileOpenAwaiter : public BasicIOAwaiterInterface {
private:
  const char *file_path;
  int flag;
  uint32_t mode;

public:
  IOFileOpenAwaiter(const char *file_path, int flag, uint32_t mode);
  void submit() override;
};
class IOSerialOpenAwaiter : public BasicIOAwaiterInterface {
private:
  const char *file_path;

public:
  IOSerialOpenAwaiter(const char *file_path);
  void submit() override;
};

class IOUDPOpenAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t listen_size;
  const void *addr;

public:
  IOUDPOpenAwaiter(const void *addr, uint64_t listen_size);
  void submit() override;
};

class IOTCPOpenAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t listen_size;
  const void *addr;
  IOTCPType tcp_type;

public:
  IOTCPOpenAwaiter(const void *addr, uint64_t listen_size, IOTCPType tcp_type);
  void submit() override;
};

class IOTCPCloseAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;

public:
  IOTCPCloseAwaiter(int fd);
  void submit() override;
};

class IOUDPCloseAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;
  const void *conn_addr;

public:
  IOUDPCloseAwaiter(int fd, const void *conn_addr);
  void submit() override;
};

class IOSerialCloseAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;

public:
  IOSerialCloseAwaiter(int fd);
  void submit() override;
};

class IOFileCloseAwaiter : public BasicIOAwaiterInterface {
private:
  int fd;

public:
  IOFileCloseAwaiter(int fd);
  void submit() override;
};

class IOFileWriteAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t buf_size;
  int fd;
  const uint8_t *buf;

public:
  IOFileWriteAwaiter(int fd, const uint8_t *buf, uint64_t buf_size);
  void submit() override;
};

class IOSerialWriteAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t buf_size;
  int fd;
  const uint8_t *buf;
  IOPackageType package_type;
  bool with_ack;

public:
  IOSerialWriteAwaiter(int fd, const uint8_t *buf, uint64_t buf_size,
                       IOPackageType package_type, bool with_ack);
  void submit() override;
};

class IOFileReadAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t buf_size;
  int fd;
  uint8_t *buf;

public:
  IOFileReadAwaiter(int fd, uint8_t *buf, uint64_t buf_size);
  void submit() override;
};

class IOSerialReadAwaiter : public BasicIOAwaiterInterface {
private:
  uint64_t buf_size;
  int fd;
  uint8_t *buf;

public:
  IOSerialReadAwaiter(int fd, uint8_t *buf, uint64_t buf_size);
  void submit() override;
};

} // namespace busy_polling_awaiters
#endif
