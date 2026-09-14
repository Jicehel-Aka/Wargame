#include "campaign.h"
#include "../core/filesystem.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>

namespace wg {

static const char* kSavePath = SD_GAME_DIR "/wargame_army.sav";

void campaign_new(PlayerArmy& army) {
    army.units.clear();
    army.victory_points = 0;
    army.campaign_map_index = 0;
    army.next_unit_id = 1;

    // Armée de départ : un noyau équilibré
    UnitType starters[] = { UnitType::Infantry, UnitType::Infantry,
                             UnitType::Archer, UnitType::Cavalry };
    for (auto t : starters) {
        ArmyUnit u;
        u.type = t;
        u.hp = unit_stats(t).hp_max;
        u.id = army.next_unit_id++;
        army.units.push_back(u);
    }
}

void campaign_save(const PlayerArmy& army) {
    std::ostringstream out;
    out << army.victory_points << " " << army.campaign_map_index << " " << army.next_unit_id << "\n";
    for (const auto& u : army.units)
        out << (int)u.type << " " << u.hp << " " << u.id << "\n";
    fs_write_text(kSavePath, out.str().c_str());
}

void campaign_load(PlayerArmy& army) {
    std::string content;
    if (!fs_exists(kSavePath) || !fs_read_text(kSavePath, content)) {
        campaign_new(army);
        return;
    }
    std::istringstream in(content);
    in >> army.victory_points >> army.campaign_map_index >> army.next_unit_id;
    army.units.clear();
    int type_i, hp; uint32_t id;
    while (in >> type_i >> hp >> id) {
        ArmyUnit u;
        u.type = (UnitType)type_i;
        u.hp = hp;
        u.id = id;
        army.units.push_back(u);
    }
    if (army.units.empty()) campaign_new(army);
}

void campaign_deploy_army(const PlayerArmy& army, const LevelDef& level, Map& map) {
    size_t zone_i = 0;
    for (const auto& au : army.units) {
        if (au.hp <= 0) continue; // unité tombée définitivement (plus de PV restants)
        if (zone_i >= level.player_deploy_zone.size()) break; // zone de déploiement pleine

        Unit u;
        u.type = au.type;
        u.faction = Faction::Player;
        u.pos = level.player_deploy_zone[zone_i++];
        u.hp = au.hp;
        u.moves_left = unit_stats(au.type).move_range;
        u.action_used = false;
        u.alive = true;
        u.id = au.id;
        map.units.push_back(u);
    }
}

void campaign_collect_results(PlayerArmy& army, const Map& finished_map, const LevelDef& level) {
    // Met à jour les PV des unités survivantes, retire les unités mortes définitivement
    for (auto& au : army.units) {
        bool found_alive = false;
        for (const auto& mu : finished_map.units) {
            if (mu.faction == Faction::Player && mu.id == au.id) {
                au.hp = mu.alive ? mu.hp : 0;
                found_alive = mu.alive;
                break;
            }
        }
        if (!found_alive) au.hp = 0; // morte au combat -> retirée du roster (hp=0)
    }
    // nettoyage : on retire les unités à 0 PV de l'armée persistante
    army.units.erase(
        std::remove_if(army.units.begin(), army.units.end(),
                        [](const ArmyUnit& u) { return u.hp <= 0; }),
        army.units.end());

    army.victory_points += level.victory_points_reward;
    army.campaign_map_index++;
}

int campaign_recruit_cost(UnitType type) {
    return unit_stats(type).cost;
}

int campaign_heal_cost_per_hp() {
    return 2; // 2 PV de victoire pour soigner 1 PV de vie
}

bool campaign_recruit(PlayerArmy& army, UnitType type) {
    int cost = campaign_recruit_cost(type);
    if (army.victory_points < cost) return false;
    army.victory_points -= cost;
    ArmyUnit u;
    u.type = type;
    u.hp = unit_stats(type).hp_max;
    u.id = army.next_unit_id++;
    army.units.push_back(u);
    return true;
}

bool campaign_heal(PlayerArmy& army, int unit_index, int hp_amount) {
    if (unit_index < 0 || unit_index >= (int)army.units.size()) return false;
    ArmyUnit& u = army.units[unit_index];
    int max_hp = unit_stats(u.type).hp_max;
    int missing = max_hp - u.hp;
    if (missing <= 0) return false;
    int heal = (hp_amount < missing) ? hp_amount : missing;
    int cost = heal * campaign_heal_cost_per_hp();
    if (army.victory_points < cost) return false;
    army.victory_points -= cost;
    u.hp += heal;
    return true;
}

} // namespace wg
