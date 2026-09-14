#include "input.h"
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include "esp_timer.h"

extern gb_core g_core; // déclaré dans app_main.cpp

namespace wg {

static uint32_t s_combo_start = 0;

static inline uint32_t millis_now() {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void input_init() {
    g_core.buttons.update();
}

void input_poll() {
    g_core.buttons.update();
    g_core.joystick.update();
}

bool key_down(gb_buttons::gb_key k) {
    return g_core.buttons.pressed(k) || (g_core.buttons.state() & (uint16_t)k) != 0;
}

bool key_pressed(gb_buttons::gb_key k) {
    return g_core.buttons.pressed(k);
}

bool key_released(gb_buttons::gb_key k) {
    return g_core.buttons.released(k);
}

// -----------------------------------------------------------------------------
//  Retour au loader : maintenir RUN ("home") + MENU pendant ~500ms.
//  Adapté du snippet fourni, intégré à l'API gb_buttons du moteur.
// -----------------------------------------------------------------------------
void check_return_to_loader() {
    bool home_held = (g_core.buttons.state() & (uint16_t)gb_buttons::KEY_RUN) != 0;
    bool menu_held = (g_core.buttons.state() & (uint16_t)gb_buttons::KEY_MENU) != 0;

    if (home_held && menu_held) {
        if (!s_combo_start) {
            s_combo_start = millis_now();
        } else if (millis_now() - s_combo_start >= 500) {
            s_combo_start = 0;
            const esp_partition_t* loader = esp_partition_find_first(
                ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);
            if (loader) {
                esp_ota_set_boot_partition(loader);
                esp_restart();
            }
        }
    } else {
        s_combo_start = 0;
    }
}

} // namespace wg
