// Implémentation PC de gb_ll_lcd : pas d'écran physique, juste le
// framebuffer en mémoire (même storage que le vrai firmware, cf.
// "extern gb_pixel framebuffer[...]" dans gb_common.h) que main.cpp copie
// vers une texture SDL2 à chaque frame.
#include "gb_ll_lcd.h"
#include "gb_common.h"
#include "gb_graphics.h"
#include <cstring>
#include <cstdio>

// Definition reelle des glyphes 8x8 (extern declare cote core/graphics.cpp) —
// incluse ici une seule fois pour fournir le storage a l'edition de liens.
#include "font8x8_basic.h"

// Storage réel du framebuffer déclaré "extern" dans gb_common.h.
gb_pixel framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void gb_ll_lcd_init() { printf("[PC] gb_ll_lcd_init() -- framebuffer %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT); }
void LCD_FAST_test(const gb_pixel*) {}
uint32_t LCD_last_refresh_delay() { return 0; }
void lcd_refresh() { /* main.cpp pousse le framebuffer vers SDL2 lui-même */ }
uint32_t gb_ll_lcd_get_draw_count() { static uint32_t n = 0; return ++n; }

void lcd_clear(uint16_t color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) framebuffer[i] = color;
}
void lcd_dpo() {}
uint8_t lcd_refresh_completed() { return 1; }

void lcd_putpixel(uint16_t x, uint16_t y, gb_pixel color) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    framebuffer[(int)y * SCREEN_WIDTH + (int)x] = color;
}
gb_pixel lcd_getpixel(uint16_t x, uint16_t y) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return 0;
    return framebuffer[(int)y * SCREEN_WIDTH + (int)x];
}
void lcd_update_pwm(uint16_t) {}
void lcd_set_fps(uint8_t) {}
void lcd_scrool_vertical(int16_t) {}

// --------------------------------------------------------------------
// gb_graphics : seules les méthodes non-inline réellement appelées par
// core/graphics.cpp (set_backlight_percent, set_refresh_rate, update,
// fillRect) ont besoin d'un corps ici.
// --------------------------------------------------------------------
gb_graphics::gb_graphics() {}
gb_graphics::~gb_graphics() {}

void gb_graphics::fillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    for (int16_t iy = 0; iy < h; ++iy)
        for (int16_t ix = 0; ix < w; ++ix)
            lcd_putpixel(x + ix, y + iy, u16_color_pen);
}
void gb_graphics::set_backlight_percent(uint8_t) {}
void gb_graphics::set_refresh_rate(uint8_t) {}
void gb_graphics::update() {}
