#ifndef BLE_CLIENT
#define BLE_CLIENT
#include "host/ble_gap.h"
#include <cstddef>
#include <cstdint>
#include <optional>
#define BLE_ERROR_TAG "BLE ERROR"
#define BLE_TAG "BLE"

class BLEClient {
private:
  uint8_t own_addr_type;
  struct ble_gap_disc_params disc_params;
  bool disc_params_initialized = false;

public:
  int (*gap_callback)(struct ble_gap_event *event, void *arg);
  void (*on_sync)(void);
  BLEClient(void);
  int scan(void);
  void setup(void);
};

#endif
