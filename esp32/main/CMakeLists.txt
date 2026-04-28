file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "src/io/*.cpp"
    "src/logging/*.cpp"
    "../../common/src/*.cpp"
)
idf_component_register(SRCS ${SRC_FILES} 
                       INCLUDE_DIRS "." "./include" "../../common/include"
					   REQUIRES esp_wifi nvs_flash bt esp_http_client app_update esp_https_ota esp_netif mbedtls esp_psram esp_driver_uart
				   )
target_compile_options(${COMPONENT_LIB} PUBLIC -std=gnu++23)

