/*
===============================================================================
  story_image.h — Illustrations de briefing (Wargame AKA)
-------------------------------------------------------------------------------
  Format fichier : brut, STORY_IMAGE_W x STORY_IMAGE_H pixels RGB565, sans
  en-tête (même convention que sprites.h, y compris la transparence par
  couleur-clé COLOR_CHROMA_KEY si besoin). Chargé à la demande depuis
  SD_GAME_DIR "/story/<name>.bin" — contrairement aux sprites d'unité/terrain
  (16 fichiers, tenus en permanence en RAM), les illustrations sont plus
  grandes et montrées une à la fois : un seul buffer est réutilisé, avec un
  petit cache "dernier nom charge" pour éviter de relire le même fichier à
  chaque frame pendant qu'un briefing reste affiché.
===============================================================================
*/
#pragma once
#include <cstdint>

namespace wg {

static constexpr int STORY_IMAGE_W = 160;
static constexpr int STORY_IMAGE_H = 64;

// Retourne le buffer RGB565 (STORY_IMAGE_W*STORY_IMAGE_H) de l'illustration
// nommee, ou nullptr si name est nul/vide ou si le fichier est introuvable.
// Le pointeur retourne n'est valide que jusqu'au prochain appel (buffer unique).
const uint16_t* story_image_load(const char* name);

} // namespace wg
