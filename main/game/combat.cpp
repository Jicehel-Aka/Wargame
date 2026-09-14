#include "combat.h"
#include "../core/audio.h"
#include "../difficulty.h"
#include <cstdlib>
#include <cmath>

namespace wg {

static int apply_terrain_defense(int base_def, const Map& map, const Hex& pos) {
    Terrain t = map.get_tile(pos).terrain;
    int bonus = terrain_info(t).def_bonus; // en %
    return base_def + (base_def * bonus) / 100;
}

// Une cible est "flanquée" si au moins 2 unités ennemies (de son point de vue)
// sont adjacentes à elle au moment de l'attaque -> elle ne peut pas se concentrer
// sur l'attaquant principal : -25% de défense effective.
static bool is_flanked(const Unit& target, const Map& map) {
    int enemy_neighbors = 0;
    for (const auto& n : hex_neighbors(target.pos)) {
        const Unit* u = map.unit_at(n);
        if (u && u->alive && u->faction != target.faction) enemy_neighbors++;
    }
    return enemy_neighbors >= 2;
}

CombatResult resolve_attack(Unit& attacker, Unit& target, const Map& map) {
    CombatResult res;
    sfx_attack();
    const UnitStats& a_stats = unit_stats(attacker.type);
    const UnitStats& t_stats = unit_stats(target.type);

    int base_def = t_stats.defense;
    if (target.faction == Faction::Enemy)
        base_def += difficulty_enemy_defense_delta();
    if (base_def < 0) base_def = 0;
    int target_def = apply_terrain_defense(base_def, map, target.pos);

    res.flanked = is_flanked(target, map);
    if (res.flanked)
        target_def = (target_def * 75) / 100; // -25%

    // Le mage ignore une partie de la défense (dégâts magiques)
    int effective_def = target_def;
    if (attacker.type == UnitType::Mage)
        effective_def = target_def / 2;

    int attack_value = a_stats.attack;
    if (attacker.faction == Faction::Enemy)
        attack_value += difficulty_enemy_attack_delta();
    if (attack_value < 1) attack_value = 1;

    int dmg = attack_value - effective_def;
    if (dmg < 1) dmg = 1; // toujours au moins 1 point de dégâts

    // Le sapeur est spécialisé dans la prise de fortifications
    if (a_stats.siege_bonus) {
        Terrain target_terrain = map.get_tile(target.pos).terrain;
        if (target_terrain == Terrain::Castle || target_terrain == Terrain::Wall)
            dmg += dmg / 2; // +50% contre les défenseurs retranchés
    }

    res.damage_to_target = dmg;

    target.hp -= dmg;
    sfx_hit();
    if (target.hp <= 0) {
        target.hp = 0;
        target.alive = false;
        res.target_died = true;
        sfx_unit_death();
    }

    // Riposte uniquement si l'attaque est au contact (range==1), que la cible
    // a un type de combat à portée 1, et qu'elle a survécu.
    attacker.action_used = true;

    if (!res.target_died && a_stats.atk_range == 1 && t_stats.atk_range == 1) {
        int atk_def = apply_terrain_defense(a_stats.defense, map, attacker.pos);
        int counter_value = t_stats.attack;
        if (target.faction == Faction::Enemy)
            counter_value += difficulty_enemy_attack_delta();
        if (counter_value < 1) counter_value = 1;
        int counter = counter_value - atk_def;
        if (counter < 1) counter = 1;
        res.damage_to_attacker = counter;
        attacker.hp -= counter;
        if (attacker.hp <= 0) {
            attacker.hp = 0;
            attacker.alive = false;
            sfx_unit_death();
        }
    }

    return res;
}

} // namespace wg
