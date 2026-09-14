/*
===============================================================================
  hex.h — Géométrie de grille hexagonale (axial coordinates, flat-top)
-------------------------------------------------------------------------------
  Convention :
    - Coordonnées axiales (q, r), hexagones "flat-top" (sommet en haut/bas).
    - Conversion écran <-> hex tient compte de la caméra (pan + zoom).
  Auteur : wargame_aka
===============================================================================
*/
#pragma once
#include <vector>
#include <cstdint>

namespace wg {

struct Hex {
    int q = 0;
    int r = 0;
    bool operator==(const Hex& o) const { return q == o.q && r == o.r; }
    bool operator!=(const Hex& o) const { return !(*this == o); }
};

// Distance en nombre de cases entre deux hexagones
int hex_distance(const Hex& a, const Hex& b);

// Les 6 voisins d'un hexagone (ordre fixe, utile pour les flancs)
std::vector<Hex> hex_neighbors(const Hex& h);

// Tous les hexagones à distance <= range de "center" (inclus le centre)
std::vector<Hex> hex_range(const Hex& center, int range);

// Tous les hexagones à distance == range exactement (anneau)
std::vector<Hex> hex_ring(const Hex& center, int range);

// Conversion axial -> pixel (centre de la case), pour une taille de case "size" en pixels
// et un centre écran (origin_x, origin_y) correspondant à hex (0,0).
void hex_to_pixel(const Hex& h, float size, float origin_x, float origin_y, float* out_x, float* out_y);

// Conversion pixel -> axial le plus proche (inverse de hex_to_pixel)
Hex pixel_to_hex(float px, float py, float size, float origin_x, float origin_y);

} // namespace wg
