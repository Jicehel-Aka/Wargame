/*
===============================================================================
  sprites.h — Chargement des sprites bitmap 16x16 (terrain + unités) depuis SD
-------------------------------------------------------------------------------
  Format fichier : brut, 16x16 pixels RGB565 (uint16_t natif), 512 octets,
  sans en-tête. Un fichier manquant n'est pas une erreur fatale : le sprite
  correspondant reste simplement non chargé (nullptr), et l'appelant doit
  prévoir un repli (cf. draw_map / draw_units dans game.cpp qui retombent sur
  le rendu couleur pleine / glyphe existant).
===============================================================================
*/
#pragma once
#include <cstdint>
#include "../game/types.h"

namespace wg {

static constexpr int SPRITE_SIZE   = 16;
static constexpr int SPRITE_PIXELS = SPRITE_SIZE * SPRITE_SIZE;

// A appeler une fois au démarrage, après g_core.init() (qui monte la SD).
void sprites_init();

// Buffer RGB565 16x16 (SPRITE_PIXELS uint16_t) du terrain, ou nullptr si le
// fichier correspondant n'a pas été trouvé/chargé sur la SD.
const uint16_t* sprite_for_terrain(Terrain t);

// Buffer RGB565 16x16 de l'unité, ou nullptr si non chargé.
const uint16_t* sprite_for_unit(UnitType t);

} // namespace wg
