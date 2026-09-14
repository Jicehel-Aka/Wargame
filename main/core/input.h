/*
===============================================================================
  input.h — Entrées utilisateur (boutons AKA) + retour au loader
===============================================================================
*/
#pragma once
#include <cstdint>
#include "gb_core.h"

namespace wg {

void input_init();
void input_poll();   // à appeler une fois par frame

bool key_down(gb_buttons::gb_key k);      // état maintenu
bool key_pressed(gb_buttons::gb_key k);   // front montant (cette frame)
bool key_released(gb_buttons::gb_key k);  // front descendant (cette frame)

// Vérifie la combo RUN+MENU maintenue ~500ms : redémarre sur la partition loader.
// À appeler à chaque frame depuis la boucle principale.
void check_return_to_loader();

} // namespace wg
