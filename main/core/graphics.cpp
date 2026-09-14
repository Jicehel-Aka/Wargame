/*
===============================================================================
  graphics.cpp — Couche graphique wargame_aka (basée sur gb_graphics / gb_ll_lcd)
===============================================================================
*/
#include "graphics.h"
#include "gb_graphics.h"
#include "gb_ll_lcd.h"
#include "gb_common.h"
#include <cmath>

extern char font8x8_basic[128][8];

namespace wg {

static gb_graphics g_gfx;

void gfx_init() {
    g_gfx.set_backlight_percent(80);
    g_gfx.set_refresh_rate(60);
}

void gfx_clear(Color color) {
    lcd_clear(color);
}

void gfx_present() {
    g_gfx.update();
}

void gfx_putpixel(int x, int y, Color color) {
    lcd_putpixel(x, y, color);
}

void gfx_fillRect(int x, int y, int w, int h, Color color) {
    g_gfx.setColor(color);
    g_gfx.fillRect(x, y, w, h);
}

void gfx_drawRect(int x, int y, int w, int h, Color color) {
    for (int ix = 0; ix < w; ++ix) {
        lcd_putpixel(x + ix, y, color);
        lcd_putpixel(x + ix, y + h - 1, color);
    }
    for (int iy = 0; iy < h; ++iy) {
        lcd_putpixel(x, y + iy, color);
        lcd_putpixel(x + w - 1, y + iy, color);
    }
}

void gfx_drawLine(int x0, int y0, int x1, int y1, Color color) {
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        lcd_putpixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// Tri-fill scanline simple (suffisant pour les hexagones de petite taille)
void gfx_fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color) {
    // tri par y croissant
    if (y0 > y1) { int t=x0;x0=x1;x1=t; t=y0;y0=y1;y1=t; }
    if (y1 > y2) { int t=x1;x1=x2;x2=t; t=y1;y1=y2;y2=t; }
    if (y0 > y1) { int t=x0;x0=x1;x1=t; t=y0;y0=y1;y1=t; }

    auto interp = [](int ya, int xa, int yb, int xb, int y) -> int {
        if (yb == ya) return xa;
        return xa + (xb - xa) * (y - ya) / (yb - ya);
    };

    for (int y = y0; y <= y2; ++y) {
        int xa, xb;
        if (y < y1) {
            xa = interp(y0, x0, y1, x1, y);
            xb = interp(y0, x0, y2, x2, y);
        } else {
            xa = interp(y1, x1, y2, x2, y);
            xb = interp(y0, x0, y2, x2, y);
        }
        if (xa > xb) { int t = xa; xa = xb; xb = t; }
        for (int x = xa; x <= xb; ++x)
            lcd_putpixel(x, y, color);
    }
}

void gfx_drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color) {
    gfx_drawLine(x0, y0, x1, y1, color);
    gfx_drawLine(x1, y1, x2, y2, color);
    gfx_drawLine(x2, y2, x0, y0, color);
}

void gfx_fillCircle(int cx, int cy, int r, Color color) {
    int x = r, y = 0, err = 1 - r;
    while (x >= y) {
        for (int ix = cx - x; ix <= cx + x; ++ix) {
            lcd_putpixel(ix, cy + y, color);
            lcd_putpixel(ix, cy - y, color);
        }
        for (int ix = cx - y; ix <= cx + y; ++ix) {
            lcd_putpixel(ix, cy + x, color);
            lcd_putpixel(ix, cy - x, color);
        }
        y++;
        if (err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

void gfx_drawCircle(int cx, int cy, int r, Color color) {
    int x = r, y = 0, err = 1 - r;
    while (x >= y) {
        lcd_putpixel(cx + x, cy + y, color); lcd_putpixel(cx + y, cy + x, color);
        lcd_putpixel(cx - y, cy + x, color); lcd_putpixel(cx - x, cy + y, color);
        lcd_putpixel(cx - x, cy - y, color); lcd_putpixel(cx - y, cy - x, color);
        lcd_putpixel(cx + y, cy - x, color); lcd_putpixel(cx + x, cy - y, color);
        y++;
        if (err < 0) err += 2 * y + 1;
        else { x--; err += 2 * (y - x + 1); }
    }
}

void gfx_text(int x, int y, const char* text, Color color) {
    int px = x;
    while (*text) {
        unsigned char c = static_cast<unsigned char>(*text++);
        if (c >= 128) c = '?';
        const uint8_t* glyph = reinterpret_cast<const uint8_t*>(font8x8_basic[c]);
        for (int gy = 0; gy < 8; ++gy) {
            uint8_t row = glyph[gy];
            for (int gx = 0; gx < 8; ++gx)
                if (row & (1 << gx))
                    lcd_putpixel(px + gx, y + gy, color);
        }
        px += 8;
    }
}

void gfx_text_center(int y, const char* text, Color color) {
    int len = 0;
    for (const char* p = text; *p; ++p) len++;
    int x = (gfx_width() - len * 8) / 2;
    gfx_text(x, y, text, color);
}

void gfx_drawBitmap(int x, int y, int w, int h, const uint16_t* pixels, Color transparent_key) {
    if (!pixels) return;
    for (int iy = 0; iy < h; ++iy) {
        const uint16_t* row = pixels + iy * w;
        for (int ix = 0; ix < w; ++ix) {
            uint16_t c = row[ix];
            if (c == transparent_key) continue;
            lcd_putpixel(x + ix, y + iy, c);
        }
    }
}

void gfx_drawBitmapScaledHexClip(int cx, int cy, int src_w, int src_h,
                                  const uint16_t* pixels, Color transparent_key,
                                  int zoom_fp, float hex_size) {
    if (!pixels) return;
    int dst_w = (src_w * zoom_fp) >> 8;
    int dst_h = (src_h * zoom_fp) >> 8;
    if (dst_w <= 0 || dst_h <= 0) return;
    int x0 = cx - dst_w / 2;
    int y0 = cy - dst_h / 2;

    // Test d'appartenance a l'hexagone flat-top de circumrayon hex_size,
    // centre sur (cx,cy) : pour |y|<=hex_size*0.866 (moitie superieure des
    // aretes obliques), la largeur autorisee en x se retrecit lineairement
    // depuis le sommet (hex_size a y=0) jusqu'a la moitie sur les aretes
    // haute/basse plates (meme formule que hex_corners(), donc le decoupage
    // colle exactement au contour hexagonal dessine par ailleurs).
    const float kApothemRatio = 0.8660254f;   // sin(60deg)
    const float kInvSqrt3     = 0.5773503f;   // 1/sqrt(3)
    float half_h = hex_size * kApothemRatio;

    for (int dy = 0; dy < dst_h; ++dy) {
        float py = (float)(y0 + dy - cy);
        if (py < -half_h || py > half_h) continue;
        float max_x = hex_size - fabsf(py) * kInvSqrt3;

        int sy = (dy << 8) / zoom_fp;
        if (sy >= src_h) sy = src_h - 1;
        const uint16_t* row = pixels + sy * src_w;
        for (int dx = 0; dx < dst_w; ++dx) {
            float px = (float)(x0 + dx - cx);
            if (px < -max_x || px > max_x) continue;

            int sx = (dx << 8) / zoom_fp;
            if (sx >= src_w) sx = src_w - 1;
            uint16_t c = row[sx];
            if (c == transparent_key) continue;
            lcd_putpixel(x0 + dx, y0 + dy, c);
        }
    }
}

void gfx_drawBitmapScaledCentered(int cx, int cy, int src_w, int src_h,
                                   const uint16_t* pixels, Color transparent_key, int zoom_fp) {
    if (!pixels) return;
    int dst_w = (src_w * zoom_fp) >> 8;
    int dst_h = (src_h * zoom_fp) >> 8;
    if (dst_w <= 0 || dst_h <= 0) return;
    int x0 = cx - dst_w / 2;
    int y0 = cy - dst_h / 2;

    // Echantillonnage plus proche voisin : pour chaque pixel destination,
    // on retrouve le pixel source correspondant via le facteur inverse.
    for (int dy = 0; dy < dst_h; ++dy) {
        int sy = (dy << 8) / zoom_fp;
        if (sy >= src_h) sy = src_h - 1;
        const uint16_t* row = pixels + sy * src_w;
        for (int dx = 0; dx < dst_w; ++dx) {
            int sx = (dx << 8) / zoom_fp;
            if (sx >= src_w) sx = src_w - 1;
            uint16_t c = row[sx];
            if (c == transparent_key) continue;
            lcd_putpixel(x0 + dx, y0 + dy, c);
        }
    }
}

int gfx_width()  { return SCREEN_WIDTH; }
int gfx_height() { return SCREEN_HEIGHT; }

} // namespace wg
