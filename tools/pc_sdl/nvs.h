#pragma once
#include <cstdint>
#define ESP_OK 0
#define ESP_ERR_NVS_NO_FREE_PAGES 0x1101
#define ESP_ERR_NVS_NEW_VERSION_FOUND 0x1102
typedef int esp_err_t;
typedef uint32_t nvs_handle_t;
enum nvs_open_mode_t { NVS_READONLY = 0, NVS_READWRITE = 1 };

extern "C" {
esp_err_t nvs_open(const char* name, nvs_open_mode_t mode, nvs_handle_t* out_handle);
esp_err_t nvs_get_u8(nvs_handle_t h, const char* key, uint8_t* out_value);
esp_err_t nvs_set_u8(nvs_handle_t h, const char* key, uint8_t value);
esp_err_t nvs_commit(nvs_handle_t h);
void nvs_close(nvs_handle_t h);
}
