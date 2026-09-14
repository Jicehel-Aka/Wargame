// Stubs minimaux des en-têtes ESP-IDF utilisés par core/input.cpp, pour
// compiler ce fichier tel quel (sans le modifier) sur PC/Unix. Le combo
// RUN+MENU (retour loader) devient un no-op inoffensif ici.
#pragma once

typedef struct { int dummy; } esp_partition_t;
enum esp_partition_type_t { ESP_PARTITION_TYPE_APP = 0 };
enum esp_partition_subtype_t { ESP_PARTITION_SUBTYPE_APP_OTA_1 = 0 };

inline const esp_partition_t* esp_partition_find_first(esp_partition_type_t, esp_partition_subtype_t, const char*) {
    return nullptr; // jamais trouve sur PC -> le combo retour-loader ne fait rien
}
inline int esp_ota_set_boot_partition(const esp_partition_t*) { return 0; }
