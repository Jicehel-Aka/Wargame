/*
===============================================================================
  difficulty.cpp — Gestion de la difficulté (Wargame AKA)
===============================================================================
*/
#include "difficulty.h"
#include "language.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <cstdlib>
#include <ctime>

namespace wg {

static Difficulty g_diff = Difficulty::Normal;

// Graine du generateur pseudo-aleatoire (cf. difficulty_enemy_attack_delta) --
// initialisee une seule fois, a la premiere utilisation. Qualite suffisante
// pour de la variabilite de combat, pas un usage cryptographique.
static void ensure_rng_seeded() {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(nullptr));
        seeded = true;
    }
}

void difficulty_set(Difficulty d) {
    g_diff = d;
    difficulty_save_to_nvs();
}

Difficulty difficulty_get() { return g_diff; }

void difficulty_next() {
    int v = ((int)g_diff + 1) % (int)Difficulty::COUNT;
    difficulty_set((Difficulty)v);
}

void difficulty_prev() {
    int v = ((int)g_diff - 1 + (int)Difficulty::COUNT) % (int)Difficulty::COUNT;
    difficulty_set((Difficulty)v);
}

const char* difficulty_name(Difficulty d) {
    bool fr = (language_get() == Language::FR);
    switch (d) {
        case Difficulty::Easy:   return fr ? "Facile"    : "Easy";
        case Difficulty::Normal: return fr ? "Normal"    : "Normal";
        case Difficulty::Hard:   return fr ? "Difficile" : "Hard";
        case Difficulty::COUNT:  break;
    }
    return "???";
}

int difficulty_enemy_attack_delta() {
    switch (g_diff) {
        case Difficulty::Easy:   return -2;
        case Difficulty::Normal: return 0;
        case Difficulty::Hard:
            // Meme le palier 0-1 (moyenne 0.5) fait chuter le taux de
            // victoire a ~7% au test -- ce niveau reagit tres sec au moindre
            // bonus d'attaque ennemie systematique. On passe a un bonus rare
            // plutot que quasi-systematique : 1 coup sur 4 seulement recoit
            // +1, les 3 autres restent a la normale (moyenne ~0.25).
            ensure_rng_seeded();
            return (rand() % 4 == 0) ? 1 : 0;
        case Difficulty::COUNT:  break;
    }
    return 0;
}

int difficulty_enemy_defense_delta() {
    switch (g_diff) {
        case Difficulty::Easy:   return -1;
        case Difficulty::Normal: return 0;
        case Difficulty::Hard:   return 0;
        case Difficulty::COUNT:  break;
    }
    return 0;
}

void difficulty_load_from_nvs() {
    g_diff = Difficulty::Normal;
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
        uint8_t v = (uint8_t)Difficulty::Normal;
        if (nvs_get_u8(h, "diff", &v) == ESP_OK && v < (uint8_t)Difficulty::COUNT)
            g_diff = (Difficulty)v;
        nvs_close(h);
    }
}

void difficulty_save_to_nvs() {
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, "diff", (uint8_t)g_diff);
        nvs_commit(h);
        nvs_close(h);
    }
}

} // namespace wg
