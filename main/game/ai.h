/*
===============================================================================
  ai.h — IA adverse (niveau "tactique basique")
-------------------------------------------------------------------------------
  Comportement :
    1) Pour chaque unité ennemie jouable, évalue les cibles atteignables.
    2) Préfère les attaques qui terminent en flanquement (2+ attaquants sur
       la même cible) -> cible en priorité les unités déjà au contact d'un allié.
    3) Si une unité ennemie fragile (PV bas, faible défense type Mage/Archer)
       est à portée et vulnérable, elle est priorisée comme cible "facile".
    4) Si aucune cible n'est atteignable, l'unité se repositionne pour :
         - soutenir/flanquer une cible potentielle au prochain tour, ou
         - se replier derrière une unité plus résistante si elle est fragile
           elle-même (Archer, Mage, Catapulte) et qu'un ennemi menace sa case.
===============================================================================
*/
#pragma once
#include "map.h"

namespace wg {

// Joue le tour complet de l'IA (faction Enemy) sur la carte donnée.
void ai_play_turn(Map& map);

} // namespace wg
