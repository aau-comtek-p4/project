
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <netinet/in.h>
#include <print>
#include <termios.h>
#include <unistd.h>
#define MAGIC_HEADER 21

int main() {
  const char *port = "/dev/ttyACM0";
  io_uring ring;
  io_uring_queue_init(20, &ring, 0);

  io_uring_sqe *sqe = io_uring_get_sqe(&ring);
  io_uring_prep_open(sqe, port, O_RDWR | O_NOCTTY | O_NONBLOCK, 0644);
  io_uring_submit(&ring);
  io_uring_cqe *cqe;
  io_uring_wait_cqe(&ring, &cqe);
  io_uring_cqe_seen(&ring, cqe);
  int fd = cqe->res;
  printf("Opened with fd: [%i]\n", fd);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  struct termios tty;
  std::memset(&tty, 0, sizeof(tty));

  if (tcgetattr(fd, &tty) != 0) {
    perror("tcgetattr");
    return 1;
  }

  // Put into raw mode (IMPORTANT)
  cfmakeraw(&tty);

  // Set baud rate (mostly symbolic for USB CDC, but required)
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  // 8N1 (matches ESP32 config)
  tty.c_cflag &= ~PARENB; // no parity
  tty.c_cflag &= ~CSTOPB; // 1 stop bit
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8; // 8 data bits

  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~CRTSCTS; // no flow control

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
    return 1;
  }

  std::cout << "Reading /dev/ttyACM0...\n";

  uint8_t buf[1024];
  char text_buf[20] = {0};

  int n;
  int len = 0;
  while (true) {
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_read(sqe, fd, buf + len, sizeof(buf) - len, -1);
    io_uring_submit(&ring);
    io_uring_wait_cqe(&ring, &cqe);
    io_uring_cqe_seen(&ring, cqe);
    n = cqe->res;
    if (n <= 0) {
      break;
    }
    len += n;
    int i = 0;
    for (; i < len; i++) {
      uint16_t magic = 0;
      magic = (magic << 8) | buf[i];
      if (magic != MAGIC_HEADER) {
        continue;
      }
      uint8_t frame_len = buf[i + 1];
      printf("Frame len: [%u]\n", frame_len);
      memcpy(text_buf, (char *)&buf[i + 2], frame_len);
      text_buf[frame_len] = 0;
      printf("Got text: [%s]\n", text_buf);
    }
    memmove(buf, buf + i, len - 1);
    len -= i;
  }

  close(fd);
  return 0;
}
