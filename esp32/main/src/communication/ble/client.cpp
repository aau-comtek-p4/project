
#include "custom_ble.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_hs_id.h"

BLEClient::BLEClient(void) {
  int rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != ESP_OK) {
    ESP_LOGE(BLE_ERROR_TAG, "error determining adress type, rc = %d", rc);
    throw rc;
  }
  disc_params.filter_duplicates = pdTRUE;

  disc_params.itvl = 0;
  disc_params.window = 0;
  disc_params.filter_policy = 0;
  disc_params.limited = 0;
}

void BLEClient::setup(void) {
  ESP_ERROR_CHECK(!this->on_sync ? ESP_OK : ESP_FAIL);
  ble_hs_cfg.sync_cb = this->on_sync;
}

int BLEClient::scan(void) {
  ESP_ERROR_CHECK(!this->own_addr_type ? ESP_OK : ESP_FAIL);
  ESP_ERROR_CHECK(!this->disc_params_initialized ? ESP_OK : ESP_FAIL);
  ESP_ERROR_CHECK(!this->gap_callback ? ESP_OK : ESP_FAIL);

  int rc = ble_gap_disc(own_addr_type, (120 * 1000), &disc_params, gap_callback,
                        NULL);
  if (rc != ESP_OK) {
    ESP_LOGE(BLE_ERROR_TAG, "error determining adress type, rc = %d", rc);
    throw rc;
  }
  return 0;
}
