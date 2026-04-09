#ifndef DUMMY_TRANSPORT_H
#define DUMMY_TRANSPORT_H

#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/misc/errors.h"
#include <cstddef>
#include <cstdint>

class DummyIOTransport : public IOTransport {
public:
  void cancel(const void *user_data) override;
  void submit() override;
  void process(uint64_t timeout) override;
};

#endif
