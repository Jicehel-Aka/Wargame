// Implémentation PC de gb_core / gb_buttons / gb_joystick : remplace le
// matériel réel (GPIO, expander I2C...) par une simple variable globale que
// main.cpp met à jour chaque frame à partir de l'état clavier SDL2.
#include "gb_core.h"
#include <chrono>
#include <cstdio>

// Rempli par main.cpp (SDL_GetKeyboardState -> bits GB_KEY_*), lu par
// gb_buttons::update() à chaque frame.
uint16_t g_pc_key_state = 0;

static uint16_t s_buttons = 0;
static uint16_t s_buttons_last = 0;

static int64_t now_us() {
    using namespace std::chrono;
    return (int64_t)duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
static const int64_t s_boot_us = now_us();

// ------------------------------------------------------------- gb_buttons
void gb_buttons::update() {
    s_buttons_last = s_buttons;
    s_buttons = g_pc_key_state;
}
uint16_t gb_buttons::state() { return s_buttons; }
uint16_t gb_buttons::pressed() { return s_buttons & ~s_buttons_last; }
bool gb_buttons::pressed(gb_key key) { return (pressed() & (uint16_t)key) != 0; }
uint16_t gb_buttons::released() { return ~s_buttons & s_buttons_last; }
bool gb_buttons::released(gb_key key) { return (released() & (uint16_t)key) != 0; }

// ------------------------------------------------------------- gb_joystick
// Non utilisé par ce jeu (contrôles au D-pad uniquement) : implémentation
// minimale pour satisfaire l'édition de liens.
void gb_joystick::update() {}
int16_t gb_joystick::get_y() { return 0; }
int16_t gb_joystick::get_x() { return 0; }
uint16_t gb_joystick::state() { return 0; }
uint16_t gb_joystick::pressed() { return 0; }
bool gb_joystick::pressed(gb_buttons::gb_key) { return false; }
uint16_t gb_joystick::released() { return 0; }
bool gb_joystick::released(gb_buttons::gb_key) { return false; }
float gb_joystick::get_posx() { return 160.0f; }
float gb_joystick::get_posy() { return 120.0f; }
void gb_joystick::calibrate_center() {}
void gb_joystick::set_posx_range(float, float) {}
void gb_joystick::set_posy_range(float, float) {}
void gb_joystick::set_posxy_speed(float) {}
void gb_joystick::set_posxy(float, float) {}

// ------------------------------------------------------------- gb_core
gb_core::gb_core() {}
gb_core::~gb_core() {}
void gb_core::init() {
    printf("[PC] gb_core.init() -- port SDL2, pas de matériel réel\n");
}
void gb_core::pool() {
    buttons.update();
    joystick.update();
}
void gb_core::delay_ms(uint32_t) { /* le rythme de frame est géré par main.cpp (SDL_Delay) */ }
uint32_t gb_core::get_millis() { return (uint32_t)((now_us() - s_boot_us) / 1000); }
int64_t  gb_core::get_micros() { return now_us() - s_boot_us; }
size_t gb_core::free_psram() { return 8 * 1024 * 1024; }
size_t gb_core::free_sram()  { return 512 * 1024; }
void gb_core::power_down() { printf("[PC] power_down() ignoré\n"); }
