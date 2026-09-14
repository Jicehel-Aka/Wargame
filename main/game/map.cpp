#include "map.h"
#include <queue>
#include <algorithm>

namespace wg {

bool Map::in_bounds(const Hex& h) const {
    // Stockage en coordonnées "offset" simplifié : on borne juste q/r dans la zone allouée
    return h.q >= 0 && h.q < width && h.r >= 0 && h.r < height;
}

bool Map::is_walkable(const Hex& h) const {
    if (!in_bounds(h)) return false;
    auto it = tiles.find(h);
    Terrain t = (it != tiles.end()) ? it->second.terrain : Terrain::Plain;
    return terrain_info(t).move_cost < 999;
}

Tile Map::get_tile(const Hex& h) const {
    auto it = tiles.find(h);
    if (it != tiles.end()) return it->second;
    return Tile{};
}

void Map::set_tile(const Hex& h, Terrain t) {
    tiles[h] = Tile{ t };
}

Unit* Map::unit_at(const Hex& h) {
    for (auto& u : units)
        if (u.alive && u.pos == h) return &u;
    return nullptr;
}

const Unit* Map::unit_at(const Hex& h) const {
    for (const auto& u : units)
        if (u.alive && u.pos == h) return &u;
    return nullptr;
}

// Dijkstra simple borné par le nombre de PM restants (coûts entiers >=1).
// Les unités volantes (UnitStats::flying) ignorent le coût de terrain ET
// l'infranchissabilité (elles survolent les rivières, forêts, etc.).
std::vector<Hex> Map::reachable_tiles(const Unit& u) const {
    bool flying = unit_stats(u.type).flying;
    std::vector<Hex> result;
    std::unordered_map<Hex, int, HexHash> cost_so_far;
    cost_so_far[u.pos] = 0;

    using Item = std::pair<int, Hex>; // (cout, hex)
    auto cmp = [](const Item& a, const Item& b) { return a.first > b.first; };
    std::priority_queue<Item, std::vector<Item>, decltype(cmp)> frontier(cmp);
    frontier.push({0, u.pos});

    while (!frontier.empty()) {
        auto [cur_cost, cur] = frontier.top();
        frontier.pop();
        if (cur_cost > cost_so_far[cur]) continue;

        for (const auto& n : hex_neighbors(cur)) {
            if (!in_bounds(n)) continue;
            if (!flying && !is_walkable(n)) continue; // un vol passe partout
            if (unit_at(n) != nullptr) continue; // case occupée = bloquante pour le déplacement
            int step_cost = flying ? 1 : terrain_info(get_tile(n).terrain).move_cost;
            int new_cost = cur_cost + step_cost;
            if (new_cost > u.moves_left) continue;

            auto it = cost_so_far.find(n);
            if (it == cost_so_far.end() || new_cost < it->second) {
                cost_so_far[n] = new_cost;
                frontier.push({new_cost, n});
            }
        }
    }

    for (auto& kv : cost_so_far)
        if (kv.first != u.pos) result.push_back(kv.first);
    return result;
}

std::vector<Hex> Map::attackable_tiles(const Unit& u) const {
    std::vector<Hex> result;
    if (u.action_used) return result;
    const auto& stats = unit_stats(u.type);
    for (const auto& h : hex_range(u.pos, stats.atk_range)) {
        if (h == u.pos) continue;
        const Unit* target = unit_at(h);
        if (target && target->faction != u.faction && target->alive)
            result.push_back(h);
    }
    return result;
}

int Map::count_alive(Faction f) const {
    int n = 0;
    for (const auto& u : units)
        if (u.alive && u.faction == f) n++;
    return n;
}

} // namespace wg
