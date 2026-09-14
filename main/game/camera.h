/*
===============================================================================
  camera.h — Caméra de la carte (pan + zoom)
-------------------------------------------------------------------------------
  - zoom_fp est un facteur Q8.8 (256 = x1.0), réutilisé directement par les
    fonctions de blit scalé déjà fournies par la couche graphique AKA.
  - Le pan est exprimé en pixels écran (offset appliqué à l'origine de la grille).
===============================================================================
*/
#pragma once

namespace wg {

struct Camera {
    float offset_x = 20.0f;   // origine écran de la case (0,0), en pixels
    float offset_y = 20.0f;
    int   zoom_fp   = 224;    // Q8.8, ~0.875x par défaut (cases plus grandes et lisibles)
    static constexpr int kZoomMin = 96;   // 0.375x  (vue d'ensemble)
    static constexpr int kZoomMax = 384;  // 1.5x    (vue rapprochée)
    static constexpr float kBaseHexSize = 16.0f; // taille de case à zoom 1.0

    float hex_size() const { return kBaseHexSize * (zoom_fp / 256.0f); }

    void zoom_in()  { zoom_fp = (zoom_fp + 32 <= kZoomMax) ? zoom_fp + 32 : kZoomMax; }
    void zoom_out() { zoom_fp = (zoom_fp - 32 >= kZoomMin) ? zoom_fp - 32 : kZoomMin; }
    void pan(float dx, float dy) { offset_x += dx; offset_y += dy; }
};

} // namespace wg
