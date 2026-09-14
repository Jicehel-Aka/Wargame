/*
===============================================================================
  campaign.h — Progression de campagne, armée persistante, renforts
-------------------------------------------------------------------------------
  - L'armée du joueur (PlayerArmy) est sauvegardée sur SD entre les cartes.
  - Après chaque victoire, le joueur gagne des points de victoire et peut
    les dépenser pour acheter de nouvelles unités ou renforcer (soigner)
    les unités existantes avant la carte suivante.
===============================================================================
*/
#pragma once
#include "types.h"
#include "map.h"
#include "levels_data.h"

namespace wg {

// Charge l'armée sauvegardée (ou crée une armée de départ si aucune sauvegarde)
void campaign_load(PlayerArmy& army);

// Sauvegarde l'armée courante sur SD
void campaign_save(const PlayerArmy& army);

// Démarre une nouvelle campagne (réinitialise l'armée de départ)
void campaign_new(PlayerArmy& army);

// Place l'armée persistante du joueur sur la carte (zone de déploiement de la carte)
// en respectant les PV restants de chaque unité.
void campaign_deploy_army(const PlayerArmy& army, const LevelDef& level, Map& map);

// Après une bataille gagnée : récupère les unités survivantes (et leurs PV) dans
// l'armée persistante, ajoute les points de victoire de la carte.
void campaign_collect_results(PlayerArmy& army, const Map& finished_map, const LevelDef& level);

// Coût pour recruter une nouvelle unité d'un type donné
int campaign_recruit_cost(UnitType type);

// Coût pour soigner intégralement une unité de l'armée (par point de vie manquant)
int campaign_heal_cost_per_hp();

// Achète une nouvelle unité si assez de points de victoire (retourne false sinon)
bool campaign_recruit(PlayerArmy& army, UnitType type);

// Soigne une unité existante de l'armée (par son index) si assez de points (retourne false sinon)
bool campaign_heal(PlayerArmy& army, int unit_index, int hp_amount);

} // namespace wg
