#ifndef NAMES_H
#define NAMES_H

#include <cstdint>
#include <cstring>

#define NAME_LEN 20
#define NAME_AMOUNT 64

#define NAME_NO 0
#define NAME_PROGRAM_ALLOCATOR 1
#define NAME_FRAME_ALLOCATOR 2
#define NAME_BUFFER_ALLOCATOR 3
#define NAME_READY_QUEUE 4
#define NAME_STAGING_QUEUE 5
#define NAME_LOGGING_QUEUE 6
#define NAME_TRACE_ALLOCATOR 7
#define NAME_IO_ALLOCATOR 8
#define NAME_TIMEOUT_ROUTINE 9
#define NAME_SLEEP_ROUTINE 10
#define NAME_CTRLC_ROUTINE 11

#define NAME_IO_FILE_WRITE 12
#define NAME_IO_FILE_READ 13
#define NAME_IO_FILE_OPEN 14
#define NAME_IO_FILE_CLOSE 15

#define NAME_IO_SERIAL_WRITE 16
#define NAME_IO_SERIAL_READ 17
#define NAME_IO_SERIAL_OPEN 18
#define NAME_IO_SERIAL_CLOSE 19

#define NAME_IO_WIFI_UDP_RECV 20
#define NAME_IO_WIFI_UDP_SEND 21
#define NAME_IO_WIFI_UDP_BIND 22
#define NAME_IO_WIFI_UDP_CLOSE 23

#define NAME_END 24

class NameLookupInterface {
public:
  virtual void set_name(uint64_t name_index, const char *name) = 0;
  virtual char *get_name(uint64_t name_index) = 0;
};

template <uint64_t max_name_len, uint64_t max_name_amount>
class NameLookup : public NameLookupInterface {
private:
  char names[max_name_amount][max_name_len] = {0};

public:
  void set_name(uint64_t name_index, const char *name) override;
  char *get_name(uint64_t name_index) override;
};

template <uint64_t max_name_len, uint64_t max_name_amount>
void NameLookup<max_name_len, max_name_amount>::set_name(uint64_t name_index,
                                                         const char *name) {
  strncpy(this->names[name_index], name, max_name_len - 1);
}

template <uint64_t max_name_len, uint64_t max_name_amount>
char *NameLookup<max_name_len, max_name_amount>::get_name(uint64_t name_index) {
  return this->names[name_index];
}

#endif
