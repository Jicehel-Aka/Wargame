/*
===============================================================================
  audio.h — Système audio du Wargame AKA
-------------------------------------------------------------------------------
  Tous les sons sont générés de façon procédurale via gb_audio_track_tone
  (bips synthétiques : pas besoin de fichiers son externes). Cela reste
  simple à étendre plus tard vers de la musique PMF si souhaité (cf. TAKATRIS).
===============================================================================
*/
#pragma once

namespace wg {

// Initialise le lecteur audio et démarre la tâche de mixage interne.
// À appeler une fois au démarrage (depuis app_main / hardware_init).
void audio_init();

// Règle le volume des effets sonores (0 = muet, 10 = max).
void audio_set_sfx_volume(int v);
int  audio_get_sfx_volume();

// --- Effets sonores du gameplay ---------------------------------------------
void sfx_select();          // sélection d'une unité
void sfx_cancel();          // annulation (B)
void sfx_move();            // déplacement d'une unité
void sfx_invalid();         // action refusée (case invalide, pas assez de PV...)
void sfx_attack();          // déclenchement d'une attaque (avant résolution)
void sfx_hit();             // un coup porte des dégâts
void sfx_unit_death();      // une unité meurt
void sfx_end_turn();        // fin de tour joueur
void sfx_castle_capture();  // capture d'un château (objectif spécial)
void sfx_river_blocked();   // tentative de franchissement de rivière impossible
void sfx_victory();         // écran de victoire
void sfx_defeat();          // écran de défaite
void sfx_menu_move();       // navigation dans un menu (langue, boutique...)
void sfx_recruit();         // recrutement / soin en boutique

} // namespace wg
