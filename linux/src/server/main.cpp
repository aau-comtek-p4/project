#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <liburing.h>
#include <linux/io_uring.h>

#define QUEUE_DEPTH 4
struct request {
  int type;
  char *buffer;
};
int main() {
  struct io_uring ring;
  int err = io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
  struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
  struct io_uring_cqe *cqe;
  int fd = open("test.txt", O_CREAT | O_RDWR, 0644);
  char *test = "Hello good sir\n";
  char out_buf[20] = {};
  io_uring_prep_write(sqe, fd, test, strlen(test), 0);
  sqe->flags |= IOSQE_IO_LINK;
  request req1{.type = 0, .buffer = test};
  io_uring_sqe_set_data(sqe, &req1);
  sqe = io_uring_get_sqe(&ring);
  io_uring_prep_read(sqe, fd, out_buf, 20, 0);
  request req2{.type = 1, .buffer = out_buf};
  io_uring_sqe_set_data(sqe, &req2);
  io_uring_submit(&ring);
  io_uring_wait_cqe(&ring, &cqe);
  fprintf(stderr, "Type: [%i], Res: %i, Buf: %s\n",
          ((request *)cqe->user_data)->type, cqe->res,
          ((request *)cqe->user_data)->buffer);
  io_uring_cqe_seen(&ring, cqe);

  io_uring_wait_cqe(&ring, &cqe);
  fprintf(stderr, "Type: [%i], Res: %i, Buf: %s\n",
          ((request *)cqe->user_data)->type, cqe->res,
          ((request *)cqe->user_data)->buffer);
  io_uring_cqe_seen(&ring, cqe);
}
