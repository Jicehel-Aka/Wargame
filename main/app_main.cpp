//===============================================================================
// app_main.cpp — Point d'entrée du Wargame AKA
//===============================================================================
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "gb_core.h"

#include "core/graphics.h"
#include "core/input.h"
#include "core/audio.h"
#include "core/sprites.h"
#include "language.h"
#include "difficulty.h"

#include "tasks/task_game.h"

gb_core g_core;

static void hardware_init() {
    printf("\n=== WARGAME AKA - HARDWARE INIT ===\n");

    // Initialisation NVS (nécessaire pour la persistance de la langue choisie).
    // Pattern standard ESP-IDF : si les pages NVS sont pleines/incompatibles
    // après une mise à jour de partition, on efface et on réessaie une fois.
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_err = nvs_flash_init();
    }

    g_core.init();
    wg::gfx_init();
    wg::input_init();
    wg::audio_init();
    wg::sprites_init();   // apres g_core.init() : la SD est montee (gb_ll_sd_init)
    wg::language_load_from_nvs();
    wg::difficulty_load_from_nvs();
    printf("=== HARDWARE INIT DONE ===\n\n");
}

extern "C" void app_main(void) {
    printf("\n=============================================\n");
    printf("  Wargame Medieval Fantastique - Gamebuino AKA\n");
    printf("=============================================\n\n");

    hardware_init();

    xTaskCreatePinnedToCore(
        wg::task_game,
        "GameTask",
        8192,
        nullptr,
        6,
        nullptr,
        1
    );

    printf("[Wargame] Tache lancee. Entree en idle loop.\n");

    while (true)
        vTaskDelay(pdMS_TO_TICKS(1000));
}
