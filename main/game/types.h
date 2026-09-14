/*
===============================================================================
  types.h — Types fondamentaux du wargame (unités, terrain, armée)
===============================================================================
*/
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "hex.h"

namespace wg {

// -----------------------------------------------------------------------------
//  Terrains
// -----------------------------------------------------------------------------
enum class Terrain : uint8_t {
    Plain = 0,
    Forest,
    Hill,
    Water,      // infranchissable pour les unités terrestres
    Road,       // coût de déplacement réduit
    Castle,     // bonus défensif fort, point stratégique (capture)
    Bridge,     // seul point de franchissement d'une rivière (Water)
    Wall,       // remparts/fortifications entourant un château assiégé
    COUNT
};

struct TerrainInfo {
    const char* name_fr;
    const char* name_en;
    int   move_cost;     // coût en PM pour entrer dans la case (999 = infranchissable)
    int   def_bonus;     // bonus de défense en %
    uint16_t color;       // couleur de remplissage RGB565
};

const TerrainInfo& terrain_info(Terrain t);
const char* terrain_name(Terrain t); // nom dans la langue courante

// -----------------------------------------------------------------------------
//  Types d'unité (6 types prévus pour la v1)
// -----------------------------------------------------------------------------
enum class UnitType : uint8_t {
    Infantry = 0,   // troupe de ligne, équilibrée
    Archer,         // attaque à distance, faible en mêlée
    Cavalry,        // grande mobilité, bon contre les flancs
    Mage,           // dégâts magiques ignorant une partie de la défense, fragile
    SiegeEngine,    // catapulte : très fort contre Castle/groupes, lent, faible en défense
    Hero,           // unité forte, plus de PV, capacité spéciale (ralliement)
    Sapper,         // génie de siège : bonus de dégâts contre Chateau/Remparts
    Griffon,        // unité volante : ignore le coût et l'infranchissabilité du terrain
    COUNT
};

struct UnitStats {
    const char* name_fr;
    const char* name_en;
    char        glyph;      // lettre affichée sur la carte (pas d'atlas d'images en v1)
    int hp_max;
    int attack;
    int defense;
    int move_range;   // PM par tour
    int atk_range;    // 1 = mêlée, >1 = tir à distance
    int cost;         // coût en points de victoire pour le recrutement / renfort
    bool flying = false;       // ignore le coût de terrain ET l'infranchissabilité (rivières, etc.)
    bool siege_bonus = false;  // dégâts bonus contre les cases Chateau/Remparts
};

const UnitStats& unit_stats(UnitType t);
const char* unit_name(UnitType t); // nom dans la langue courante

// -----------------------------------------------------------------------------
//  Camp
// -----------------------------------------------------------------------------
enum class Faction : uint8_t { Player = 0, Enemy = 1 };

// -----------------------------------------------------------------------------
//  Unité instanciée sur la carte
// -----------------------------------------------------------------------------
struct Unit {
    UnitType type = UnitType::Infantry;
    Faction  faction = Faction::Player;
    Hex      pos {0, 0};
    int      hp = 1;
    int      moves_left = 0;
    bool     action_used = false;   // a déjà attaqué/agit ce tour
    bool     alive = true;
    uint32_t id = 0;                // identifiant unique stable (pour la persistance)

    bool can_act() const { return alive && (moves_left > 0 || !action_used); }
};

// -----------------------------------------------------------------------------
//  Armée persistante du joueur (garde l'état entre les cartes)
// -----------------------------------------------------------------------------
struct ArmyUnit {
    UnitType type;
    int hp;          // PV courants (les blessures restent d'une carte à l'autre)
    uint32_t id;
};

struct PlayerArmy {
    std::vector<ArmyUnit> units;
    int victory_points = 0;
    int campaign_map_index = 0;   // prochaine carte à jouer
    uint32_t next_unit_id = 1;
};

} // namespace wg
