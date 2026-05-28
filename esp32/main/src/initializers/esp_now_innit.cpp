#include "esp32/io/io.h"
#include "esp_now.h"
#include "esp_wifi_types_generic.h"
#include "freertos/idf_additions.h"
#include <cstdint>
#include <cstring>
void espnow_receive_cb(const esp_now_recv_info_t *recv_info,
                       const uint8_t *data, int len) {
  ESPNOWRxPacket rx_packet;
  memcpy(rx_packet.mac, recv_info->src_addr, ESP_NOW_ETH_ALEN);
  memcpy(rx_packet.data, data, len);
  rx_packet.len = len;

  xQueueSend(esp_now_rx_queue, &rx_packet, 0);
}

void espnow_send_cb(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  esp_now_packet_in_flight = ESPNOW_PACKET_STATE::ESPNOW_PACKET_READY;
  esp_now_send_status = status;
}
void self_esp_now_innit() {
  esp_now_init();

  esp_now_register_send_cb(espnow_send_cb);
  esp_now_register_recv_cb(espnow_receive_cb);
  esp_now_rx_queue =
      xQueueCreate(ESP_NOW_RX_QUEUE_SIZE, sizeof(ESPNOWRxPacket));
}
