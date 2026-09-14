/*
===============================================================================
  levels_data.h — Données des cartes de campagne
-------------------------------------------------------------------------------
  Chaque carte définit :
    - sa taille (largeur x hauteur en cases hexagonales)
    - le terrain (généré procéduralement à partir d'une recette simple)
    - l'armée ennemie de départ
    - les points de victoire gagnés en cas de victoire
    - le budget de renfort recommandé pour compenser les pertes moyennes
      attendues sur cette carte (affiché au joueur avant la bataille suivante)
===============================================================================
*/
#pragma once
#include <vector>
#include "types.h"
#include "map.h"

namespace wg {

enum class VictoryCondition : uint8_t {
    EliminateAll,   // détruire toute l'armée ennemie
    CaptureCastle,  // avoir une unité du joueur sur la case Chateau en fin de tour
    SurviveTurns    // rester en vie (au moins 1 unité) pendant N tours
};

struct EnemySpawn {
    UnitType type;
    Hex pos;
};

struct LevelDef {
    const char* name_fr;
    const char* name_en;
    int width;
    int height;
    int forest_seed;     // densité approximative de forêts (0-100)
    int hill_seed;       // densité approximative de collines (0-100)
    std::vector<EnemySpawn> enemies;
    std::vector<Hex> player_deploy_zone; // cases où le joueur peut placer son armée
    int victory_points_reward;
    int recommended_reinforcement_budget; // info affichée, pertes moyennes attendues

    VictoryCondition condition = VictoryCondition::EliminateAll;
    int survive_turns = 0;          // utilisé si condition == SurviveTurns

    // --- Spécialités tactiques ---------------------------------------------
    bool river_crossing = false;       // une rivière (Water) coupe la carte
    int  river_col = -1;                // colonne de la rivière (si river_crossing)
    std::vector<int> bridge_rows;       // lignes où la rivière est franchissable (Bridge)
    bool castle_walls = false;          // remparts (Wall) autour de la case Chateau

    const char* briefing_fr;        // texte d'introduction (FR) affiché avant la bataille
    const char* briefing_en;        // texte d'introduction (EN)
    const char* victory_text_fr;    // texte affiché après la victoire (FR)
    const char* victory_text_en;    // texte affiché après la victoire (EN)

    // Nom de l'illustration de briefing (sans extension), ou nullptr si aucune.
    // Fichier attendu : SD_GAME_DIR "/story/<story_image>.bin" (cf. story_image.h).
    // Champ optionnel en fin de struct -> les niveaux existants qui ne le
    // renseignent pas dans leur initialisation positionnelle restent valides.
    const char* story_image = nullptr;
};

// Accesseurs localisés (selon la langue courante, cf. language.h)
const char* level_name(const LevelDef& def);
const char* level_briefing(const LevelDef& def);
const char* level_victory_text(const LevelDef& def);

// Retourne le nombre total de cartes de la campagne
int level_count();

// Retourne la définition d'une carte (index 0..level_count()-1)
const LevelDef& level_get(int index);

// Construit la Map jouable (terrain + spawns ennemis) à partir d'une LevelDef
void level_build_map(const LevelDef& def, Map& out_map);

} // namespace wg
