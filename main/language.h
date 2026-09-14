/*
===============================================================================
  language.h — Gestion multilingue (Wargame AKA)
-------------------------------------------------------------------------------
  Même pattern que le Tetris AKA fourni en référence : enum Language,
  enum TextID pour les textes fixes de l'UI, fonction tr(id), persistance NVS.
  Les textes longs propres à chaque carte (briefing, victoire) ne passent pas
  par TextID mais par des champs dédiés dans LevelDef (cf. levels_data.h).
===============================================================================
*/
#pragma once

namespace wg {

enum class Language {
    FR = 0,
    EN,
    COUNT
};

enum class TextID {
    // Écran titre
    TITLE_GAME_NAME,
    TITLE_SUBTITLE,
    TITLE_MAP_PROGRESS,
    TITLE_PRESS_A,
    TITLE_LANGUAGE_LABEL,
    TITLE_HINT_CHANGE_LANG,
    TITLE_DIFFICULTY_LABEL,
    TITLE_HINT_CHANGE_DIFF,
    TITLE_DIFFICULTY_LOCKED,
    TITLE_HINT_DEBUG_UNLOCK,

    // Briefing
    BRIEFING_HINT_START,

    // HUD bataille
    HUD_VP_PLAYERS_ENEMIES,
    HUD_TILE_INFO,
    HUD_TILE_IMPASSABLE,
    HUD_HINT_IDLE,
    HUD_HINT_MOVE,
    HUD_HINT_ATTACK,
    HUD_HINT_NO_ACTION,
    HUD_HINT_END_TURN,
    HUD_UNIT_STATS,
    HUD_OBJ_ELIMINATE,
    HUD_OBJ_CASTLE,
    HUD_OBJ_SURVIVE,

    // Aide (overlay MENU)
    HELP_TITLE,
    HELP_DPAD,
    HELP_A,
    HELP_A2,
    HELP_A3,
    HELP_B,
    HELP_C,
    HELP_ZOOM,
    HELP_DOT_GREEN,
    HELP_DOT_YELLOW,
    HELP_TERRAINS,

    // Victoire / Défaite / Campagne
    VICTORY_TITLE,
    VICTORY_VP,
    VICTORY_CONTINUE,
    DEFEAT_TITLE,
    DEFEAT_TEXT,
    DEFEAT_RETRY,
    CAMPAIGN_DONE_TITLE,
    CAMPAIGN_DONE_SCORE,

    // Boutique
    SHOP_TITLE,
    SHOP_VP_AVAILABLE,
    SHOP_COLUMN_ARMY,
    SHOP_COLUMN_RECRUIT,
    SHOP_HEAL_ACTION,
    SHOP_HEAL_FULL,
    SHOP_RECRUIT_ACTION,
    SHOP_NOT_ENOUGH,
    SHOP_EMPTY_ARMY,
    SHOP_SWITCH_HINT,
    SHOP_NEXT_HINT,

    // Terrains
    TERRAIN_PLAIN,
    TERRAIN_FOREST,
    TERRAIN_HILL,
    TERRAIN_WATER,
    TERRAIN_ROAD,
    TERRAIN_CASTLE,

    // Unités
    UNIT_INFANTRY,
    UNIT_ARCHER,
    UNIT_CAVALRY,
    UNIT_MAGE,
    UNIT_SIEGE,
    UNIT_HERO,

    COUNT
};

void language_set(Language lang);
Language language_get();
void language_next();
void language_prev();

const char* tr(TextID id);
const char* language_name(Language lang);

void language_load_from_nvs();
void language_save_to_nvs();

} // namespace wg
