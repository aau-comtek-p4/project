
#ifndef ESP32_TEST_TASK_H
#define ESP32_TEST_TASK_H

void io_polling_task(void *args);
void tcp_test_task(void *args);
void uart_test_task(void *args);
void udp_test_task(void *args);
void espnow_test_task(void *args);

void intermidate_server_task(void *args);
void base_station_task(void *args);
#endif
