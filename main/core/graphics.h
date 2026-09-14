/*
===============================================================================
  graphics.h — Couche graphique haut niveau (wrap gb_graphics)
===============================================================================
*/
#pragma once
#include <stdint.h>

namespace wg {

using Color = uint16_t;

void gfx_init();
void gfx_clear(Color color);
void gfx_present();

void gfx_putpixel(int x, int y, Color color);
void gfx_fillRect(int x, int y, int w, int h, Color color);
void gfx_drawRect(int x, int y, int w, int h, Color color);
void gfx_drawLine(int x0, int y0, int x1, int y1, Color color);
void gfx_fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color);
void gfx_drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color color);
void gfx_fillCircle(int cx, int cy, int r, Color color);
void gfx_drawCircle(int cx, int cy, int r, Color color);

void gfx_text(int x, int y, const char* text, Color color);
void gfx_text_center(int y, const char* text, Color color);

// ---------------------------------------------------------------------------
// Sprites bitmap RGB565 (16 bits/pixel), pas de canal alpha : la transparence
// se fait par couleur-clé (chroma key) -> COLOR_CHROMA_KEY par convention.
// ---------------------------------------------------------------------------

// Dessine un bitmap w x h à l'échelle 1:1, en sautant les pixels == transparent_key.
void gfx_drawBitmap(int x, int y, int w, int h, const uint16_t* pixels, Color transparent_key);

// Dessine un bitmap w x h mis à l'échelle par un facteur Q8.8 (256 = x1.0),
// centré sur (cx, cy). Échantillonnage plus proche voisin (adapté au pixel art).
// zoom_fp doit correspondre à Camera::zoom_fp (cf camera.h) pour rester net aux
// mêmes paliers de zoom que la grille hexagonale.
void gfx_drawBitmapScaledCentered(int cx, int cy, int src_w, int src_h,
                                   const uint16_t* pixels, Color transparent_key, int zoom_fp);

// Variante pour les tuiles de terrain : ne dessine que les pixels tombant
// dans l'hexagone flat-top de circumrayon hex_size centré sur (cx, cy) —
// évite le débordement carré par-dessus les cases voisines (bords nets entre
// deux terrains différents). Le test d'appartenance est le même calcul de
// géométrie que hex_corners()/draw_hex_outline() côté jeu, donc le découpage
// correspond exactement au contour hexagonal affiché.
void gfx_drawBitmapScaledHexClip(int cx, int cy, int src_w, int src_h,
                                  const uint16_t* pixels, Color transparent_key,
                                  int zoom_fp, float hex_size);

int gfx_width();
int gfx_height();

// Le panneau ST7789V de l'AKA est configure en BGR (bit ST7789V_MADCTRL_RGB
// positionne dans gb_ll_lcd.c, cf. lcd_color_rgb() dans gb_ll_lcd.h qui packe
// le bleu dans les bits hauts) -> toute couleur logique (r,g,b) doit etre
// packee blue-high/red-low, PAS le RGB565 standard rouge-high/bleu-low.
static constexpr Color make_color(uint8_t r, uint8_t g, uint8_t b) {
    return (Color)((r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11));
}

static constexpr Color COLOR_CHROMA_KEY = make_color(255, 0, 255); // magenta — symetrique R/B, inchange par le sens BGR
static constexpr Color COLOR_BLACK    = make_color(0, 0, 0);
static constexpr Color COLOR_WHITE    = make_color(255, 255, 255);
static constexpr Color COLOR_YELLOW   = make_color(255, 255, 0);
static constexpr Color COLOR_RED      = make_color(255, 0, 0);
static constexpr Color COLOR_GREEN    = make_color(0, 255, 0);
static constexpr Color COLOR_BLUE     = make_color(0, 0, 255);
static constexpr Color COLOR_GREY     = make_color(132, 132, 132);
static constexpr Color COLOR_DARKGREY = make_color(66, 66, 66);
static constexpr Color COLOR_ORANGE   = make_color(255, 140, 0);
static constexpr Color COLOR_CYAN     = make_color(0, 255, 255);
static constexpr Color COLOR_PURPLE  = 0x8010;

} // namespace wg
