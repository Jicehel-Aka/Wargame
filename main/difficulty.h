/*
===============================================================================
  difficulty.h — Gestion de la difficulté (Wargame AKA)
-------------------------------------------------------------------------------
  Même pattern que language.h : enum + persistance NVS.
  La difficulté a deux effets :
    1) Elle module la force des unités ennemies en combat (cf. combat.cpp) :
       pourcentages appliqués à l'attaque/défense effective des ennemis.
    2) Elle détermine combien de cartes de la campagne sont accessibles
       (cf. levels_data.cpp / level_count()) : le contenu complet (les 10
       dernières cartes, l'arc "Hard" et sa fin héroïque) n'est visible qu'en
       Difficile. En Facile/Normal, la campagne se termine à la carte 20.
===============================================================================
*/
#pragma once
#include <cstdint>

namespace wg {

enum class Difficulty : uint8_t {
    Easy = 0,
    Normal,
    Hard,
    COUNT
};

Difficulty difficulty_get();
void difficulty_set(Difficulty d);
void difficulty_next();
void difficulty_prev();
void difficulty_load_from_nvs();
void difficulty_save_to_nvs();

// Nom localisé (FR/EN selon language_get()).
const char* difficulty_name(Difficulty d);

// Pourcentages appliqués aux unités ENNEMIES en résolution de combat
// (100 = neutre, cf. combat.cpp). Les unités du joueur ne sont jamais modifiées.
//
// NOTE : remplacé par un ajustement en valeur fixe (cf. ci-dessous) — les
// petites valeurs d'attaque/défense (3 à 8 en général) rendaient un scaling
// en pourcentage imprévisible : +30% d'attaque sur une valeur de 4 tronquait
// souvent a l'identique OU doublait le résultat net une fois soustrait a la
// défense, selon des seuils entiers peu lisibles. Des valeurs a plat, elles,
// se traduisent immédiatement et proportionnellement en dégâts en plus/moins.
int difficulty_enemy_attack_delta();  // ajouté à l'attaque des ennemis (négatif possible ;
                                       // en Difficile, valeur aléatoire redessinée à chaque
                                       // appel plutôt qu'une constante fixe, cf. .cpp)
int difficulty_enemy_defense_delta(); // ajouté à la défense des ennemis (peut être négatif)

} // namespace wg
