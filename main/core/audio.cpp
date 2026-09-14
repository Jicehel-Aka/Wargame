/*
===============================================================================
  audio.cpp — Système audio du Wargame AKA (SFX procéduraux)
===============================================================================
*/
#include "audio.h"
#include "gb_audio_player.h"
#include "gb_audio_track_tone.h"
#include "gb_ll_audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace wg {

static gb_audio_player     g_player;
static gb_audio_track_tone g_tone;
static int                 g_sfx_volume = 7; // 0..10 par défaut
static TaskHandle_t        s_audio_task = nullptr;

static inline float volume_f() {
    int v = g_sfx_volume;
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    return v / 10.0f;
}

static inline void tone(float freq, int ms, gb_audio_track_tone::tone_type type = gb_audio_track_tone::SINE) {
    if (g_sfx_volume <= 0 || ms <= 0) return;
    g_tone.play_tone(freq, volume_f(), (uint16_t)ms, type);
}

static inline void tone_sweep(float f0, float f1, int ms, gb_audio_track_tone::tone_type type = gb_audio_track_tone::SINE) {
    if (g_sfx_volume <= 0 || ms <= 0) return;
    float v = volume_f();
    g_tone.play_tone(f0, f1, v, v, (uint16_t)ms, type);
}

static void audio_task(void* arg) {
    (void)arg;
    while (true) {
        g_player.pool();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void audio_init() {
    gb_ll_audio_set_volume(200); // volume matériel global (0..255), distinct du volume SFX logique
    g_player.add_track(&g_tone);
    g_tone.set_track_volume(0.8f);

    if (s_audio_task == nullptr) {
        xTaskCreatePinnedToCore(audio_task, "AudioTask", 4096, nullptr, 4, &s_audio_task, 1);
    }
}

void audio_set_sfx_volume(int v) {
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    g_sfx_volume = v;
}

int audio_get_sfx_volume() { return g_sfx_volume; }

// -----------------------------------------------------------------------------
//  Effets sonores du gameplay
// -----------------------------------------------------------------------------
void sfx_select()         { tone(900, 40, gb_audio_track_tone::SQUARE); }
void sfx_cancel()         { tone(400, 50, gb_audio_track_tone::SQUARE); }
void sfx_move()           { tone_sweep(500, 700, 80, gb_audio_track_tone::SINE); }
void sfx_invalid()        { tone(150, 60, gb_audio_track_tone::NOISE); }
void sfx_attack()         { tone_sweep(300, 150, 70, gb_audio_track_tone::TRIANGLE); }
void sfx_hit()            { tone(120, 70, gb_audio_track_tone::NOISE); }
void sfx_unit_death()     { tone_sweep(400, 80, 280, gb_audio_track_tone::SINE); }
void sfx_end_turn()       { tone_sweep(500, 750, 120, gb_audio_track_tone::TRIANGLE); }
void sfx_castle_capture() { tone_sweep(400, 1000, 350, gb_audio_track_tone::SQUARE); }
void sfx_river_blocked()  { tone(180, 90, gb_audio_track_tone::NOISE); }
void sfx_victory()        { tone_sweep(400, 1200, 600, gb_audio_track_tone::TRIANGLE); }
void sfx_defeat()         { tone_sweep(350, 90, 700, gb_audio_track_tone::SQUARE); }
void sfx_menu_move()      { tone(700, 30, gb_audio_track_tone::SQUARE); }
void sfx_recruit()        { tone_sweep(600, 1000, 150, gb_audio_track_tone::SINE); }

} // namespace wg
