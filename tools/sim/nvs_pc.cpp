// Implémentation NVS pour PC : un simple fichier clé=valeur local
// (pc_nvs.txt, dans le répertoire courant) pour que langue/difficulté
// soient conservées d'un lancement à l'autre du port SDL2.
#include "nvs.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <map>

namespace {
std::map<std::string, uint8_t>& table() {
    static std::map<std::string, uint8_t> t;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        FILE* f = fopen("pc_nvs.txt", "r");
        if (f) {
            char key[64]; int val;
            while (fscanf(f, "%63s %d", key, &val) == 2)
                t[key] = (uint8_t)val;
            fclose(f);
        }
    }
    return t;
}
void save() {
    FILE* f = fopen("pc_nvs.txt", "w");
    if (!f) return;
    for (auto& kv : table())
        fprintf(f, "%s %d\n", kv.first.c_str(), kv.second);
    fclose(f);
}
} // namespace

extern "C" {

esp_err_t nvs_open(const char*, nvs_open_mode_t, nvs_handle_t* out_handle) {
    *out_handle = 1;
    return ESP_OK;
}

esp_err_t nvs_get_u8(nvs_handle_t, const char* key, uint8_t* out_value) {
    auto& t = table();
    auto it = t.find(key);
    if (it == t.end()) return -1;
    *out_value = it->second;
    return ESP_OK;
}

esp_err_t nvs_set_u8(nvs_handle_t, const char* key, uint8_t value) {
    table()[key] = value;
    return ESP_OK;
}

esp_err_t nvs_commit(nvs_handle_t) {
    save();
    return ESP_OK;
}

void nvs_close(nvs_handle_t) {}

} // extern "C"
