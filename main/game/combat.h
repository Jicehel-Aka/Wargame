/*
===============================================================================
  combat.h — Résolution des attaques
===============================================================================
*/
#pragma once
#include "types.h"
#include "map.h"

namespace wg {

struct CombatResult {
    int damage_to_target = 0;
    int damage_to_attacker = 0; // riposte (uniquement en mêlée, si la cible survit)
    bool target_died = false;
    bool flanked = false;       // l'attaque vient d'un flanc/dos (bonus appliqué)
};

// Exécute une attaque de "attacker" sur "target" et applique les dégâts directement
// aux deux unités (HP modifiés in-place). Retourne le détail pour l'UI/log.
CombatResult resolve_attack(Unit& attacker, Unit& target, const Map& map);

} // namespace wg
