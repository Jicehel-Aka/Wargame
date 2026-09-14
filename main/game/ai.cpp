#include "ai.h"
#include "combat.h"
#include <limits>
#include <algorithm>

namespace wg {

static bool is_fragile(UnitType t) {
    return t == UnitType::Archer || t == UnitType::Mage || t == UnitType::SiegeEngine;
}

// Score d'une attaque possible : privilégie les coups qui tuent, qui flanquent,
// et qui visent les unités fragiles adverses.
static int score_attack(const Unit& attacker, const Unit& target, const Map& map) {
    const UnitStats& a = unit_stats(attacker.type);
    int score = 0;

    int allies_adjacent_to_target = 0;
    for (const auto& n : hex_neighbors(target.pos)) {
        const Unit* u = map.unit_at(n);
        if (u && u->alive && u->faction == attacker.faction) allies_adjacent_to_target++;
    }
    // après cette attaque, le flanc serait constitué si on ajoute l'attaquant
    if (allies_adjacent_to_target >= 1) score += 40; // flanquement en cours/à venir

    if (is_fragile(target.type)) score += 50;

    // estimation simplifiée des dégâts pour savoir si le coup tue
    int rough_dmg = a.attack - unit_stats(target.type).defense;
    if (rough_dmg < 1) rough_dmg = 1;
    if (rough_dmg >= target.hp) score += 100; // coup fatal

    score += (100 - target.hp); // préfère finir les unités déjà blessées
    return score;
}

// Évalue à quel point une case candidate est "sûre" pour une unité fragile
// (moins d'ennemis adjacents potentiels = mieux), et bonifie si un allié
// costaud est adjacent (protection).
static int score_safety(const Unit& u, const Hex& candidate, const Map& map) {
    int score = 0;
    int enemy_threats = 0;
    int sturdy_ally_adjacent = 0;

    for (const auto& n : hex_neighbors(candidate)) {
        const Unit* nu = map.unit_at(n);
        if (!nu || !nu->alive) continue;
        if (nu->faction != u.faction) enemy_threats++;
        else if (!is_fragile(nu->type)) sturdy_ally_adjacent++;
    }
    score -= enemy_threats * 30;
    score += sturdy_ally_adjacent * 15;
    return score;
}

void ai_play_turn(Map& map) {
    for (auto& unit : map.units) {
        if (!unit.alive || unit.faction != Faction::Enemy) continue;
        if (!unit.can_act()) continue;

        // 1) Tenter une attaque directe depuis la position actuelle
        auto targets = map.attackable_tiles(unit);
        if (!targets.empty()) {
            const Unit* best = nullptr;
            int best_score = std::numeric_limits<int>::min();
            for (const auto& h : targets) {
                const Unit* t = map.unit_at(h);
                if (!t) continue;
                int s = score_attack(unit, *t, map);
                if (s > best_score) { best_score = s; best = t; }
            }
            if (best) {
                Unit* target_mut = map.unit_at(best->pos);
                if (target_mut) resolve_attack(unit, *target_mut, map);
                unit.moves_left = 0;
                continue;
            }
        }

        // 2) Pas de cible immédiate : se repositionner.
        auto reachable = map.reachable_tiles(unit);
        reachable.push_back(unit.pos); // rester sur place reste une option

        Hex best_tile = unit.pos;
        int best_score = std::numeric_limits<int>::min();

        // cherche l'ennemi le plus faible / le plus proche pour s'en approcher
        const Unit* weak_enemy = nullptr;
        int weak_score = std::numeric_limits<int>::min();
        for (const auto& e : map.units) {
            if (!e.alive || e.faction == unit.faction) continue;
            int s = (is_fragile(e.type) ? 50 : 0) + (100 - e.hp);
            if (s > weak_score) { weak_score = s; weak_enemy = &e; }
        }

        for (const auto& cand : reachable) {
            int s = 0;
            if (weak_enemy) {
                int d = hex_distance(cand, weak_enemy->pos);
                s += (50 - std::min(50, d * 5)); // se rapprocher
            }
            if (is_fragile(unit.type)) {
                s += score_safety(unit, cand, map); // unités fragiles : prudence
            } else {
                // unités robustes : valoriser le fait de couvrir un allié fragile
                for (const auto& n : hex_neighbors(cand)) {
                    const Unit* nu = map.unit_at(n);
                    if (nu && nu->alive && nu->faction == unit.faction && is_fragile(nu->type))
                        s += 20;
                }
            }
            if (s > best_score) { best_score = s; best_tile = cand; }
        }

        if (best_tile != unit.pos) {
            // coût réel parcouru = on borne simplement moves_left à 0 (mouvement "tout en un")
            unit.pos = best_tile;
        }
        unit.moves_left = 0;

        // tentative d'attaque après repositionnement
        auto post_targets = map.attackable_tiles(unit);
        if (!post_targets.empty()) {
            const Unit* best = nullptr;
            int bs = std::numeric_limits<int>::min();
            for (const auto& h : post_targets) {
                const Unit* t = map.unit_at(h);
                if (!t) continue;
                int s = score_attack(unit, *t, map);
                if (s > bs) { bs = s; best = t; }
            }
            if (best) {
                Unit* target_mut = map.unit_at(best->pos);
                if (target_mut) resolve_attack(unit, *target_mut, map);
            }
        }
    }
}

} // namespace wg
