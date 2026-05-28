#include "esp32/io/io.h"
#include "esp_log.h"
#include "esp_now.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "general/common.h"
#include "general/interfaces/io/io.h"
#include "general/interfaces/utility/clock.h"
#include "general/misc/crc.h"
#include "general/misc/shutdown.h"
#include <cstdint>
#include <cstdio>
ESPBusyPollingIO::ESPBusyPollingIO(AllocatorInterface *transport_allocator)
    : BusyPollingIO(transport_allocator) {}

IOTransport *ESPBusyPollingIO::register_transport(IOMethod io_method,
                                                  uint64_t transport_size) {
  return BusyPollingIO::register_transport(io_method, transport_size);
}

void ESPBusyPollingIO::process_all(uint64_t timeout) {
  uint64_t start_timeout = timeout / NS_PR_MS;
  if (start_timeout >= pdTICKS_TO_MS(1)) {
    vTaskDelay(pdMS_TO_TICKS(start_timeout));
    start_timeout -= pdTICKS_TO_MS(1) * pdMS_TO_TICKS(start_timeout);
  }
  return BusyPollingIO::process_all(start_timeout);
}

bool espnow_ensure_peer(const uint8_t *mac) {
  if (esp_now_is_peer_exist(mac)) {
    return true;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, ESP_NOW_ETH_ALEN);

  peer.channel = CONFIG_ESPNOW_CHANNEL;
  peer.ifidx = WIFI_IF_STA; // or WIFI_IF_STA depending on IDF version
  peer.encrypt = false;

  return esp_now_add_peer(&peer) == ESP_OK;
}
