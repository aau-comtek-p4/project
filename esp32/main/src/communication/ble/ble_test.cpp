#include "ble_test.h"
#include "custom_ble.h"
#include "esp_err.h"
#include "esp_log.h"
#include "host/ble_hs_id.h"
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <optional>

int gap_event_handler_test(struct ble_gap_event *event, void *arg) {
  switch (event->type) {
  case BLE_GAP_EVENT_DISC:
    return 0;
  case BLE_GAP_EVENT_CONNECT:
    return 0;
  case BLE_GAP_EVENT_DISCONNECT:
    return 0;
  default:
    return 0;
  }
}

void on_sync_test(void) {
  int rc;
  rc = ble_hs_util_ensure_addr(0);
  ESP_ERROR_CHECK(rc == 0);
}
extern "C" void test_ble_client() {
  try {
    BLEClient ble_client = BLEClient();
    ble_client.gap_callback = gap_event_handler_test;
    ble_client.on_sync = on_sync_test;
    ble_client.setup();
    ble_client.scan();
    ESP_LOGI(BLE_TAG, "successfully started ble scan");

  } catch (int e) {
    ESP_LOGE(BLE_ERROR_TAG, "failed initialize ble scan, e=%d", e);
  }
}
