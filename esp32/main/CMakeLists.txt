idf_component_register(SRCS "main.cpp" "src/communication/ble/client.cpp" "src/communication/ble/ble_test.cpp"
                       INCLUDE_DIRS "." "../../common/include"
					   REQUIRES esp_wifi nvs_flash bt esp_http_client app_update esp_https_ota esp_netif mbedtls esp_psram
				   )
target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++23)

