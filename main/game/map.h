/*
===============================================================================
  map.h — Carte de bataille (grille hexagonale + unités)
===============================================================================
*/
#pragma once
#include <vector>
#include <unordered_map>
#include "hex.h"
#include "types.h"

namespace wg {

struct Tile {
    Terrain terrain = Terrain::Plain;
};

// Clé de hachage simple pour Hex (q,r tenus dans des bornes raisonnables)
struct HexHash {
    size_t operator()(const Hex& h) const {
        return ((size_t)(uint32_t)(h.q + 1000) << 16) ^ (uint32_t)(h.r + 1000);
    }
};

class Map {
public:
    int width = 0;   // en colonnes (q de 0 à width-1)
    int height = 0;  // en lignes  (r de 0 à height-1)

    std::unordered_map<Hex, Tile, HexHash> tiles;
    std::vector<Unit> units;

    bool in_bounds(const Hex& h) const;
    bool is_walkable(const Hex& h) const;          // terrain franchissable
    Tile get_tile(const Hex& h) const;
    void set_tile(const Hex& h, Terrain t);

    Unit*       unit_at(const Hex& h);
    const Unit* unit_at(const Hex& h) const;

    // Calcule les cases atteignables par une unité ce tour (Dijkstra borné par moves_left)
    std::vector<Hex> reachable_tiles(const Unit& u) const;

    // Calcule les cibles attaquables (ennemis à portée d'attaque depuis la position actuelle)
    std::vector<Hex> attackable_tiles(const Unit& u) const;

    int count_alive(Faction f) const;
};

} // namespace wg
