// =============================================================================
//  campaign_sim.cpp — Simulation automatisée de la campagne complète
// -----------------------------------------------------------------------------
//  Joue les 30 niveaux, à chaque difficulté (Facile/Normal/Difficile), avec
//  un bot simple côté joueur (attaque si possible, sinon avance vers
//  l'ennemi ou le château) et la VRAIE IA ennemie (ai_play_turn, non
//  modifiée). But : détecter un niveau injouable (trop dur, objectif
//  inatteignable, carte cassée) plutôt que de juger la qualité du jeu.
//
//  Le bot est volontairement simple (pas d'optimisation fine, pas de retraite
//  tactique) : un niveau qui échoue contre CE bot n'est pas forcément
//  "impossible" pour un vrai joueur plus malin, mais un niveau qui réussit
//  très largement (peu de pertes, peu de tours) est un bon signe de
//  faisabilité. Voir le rapport texte en fin d'exécution pour l'interprétation.
// =============================================================================
#include "levels_data.h"
#include "combat.h"
#include "ai.h"
#include "campaign.h"
#include "../difficulty.h"
#include "../language.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <climits>
#include <string>
#include <vector>
#include <set>

using namespace wg;

// -----------------------------------------------------------------------------
//  Aides communes (réimplémentation légère : les originales sont `static`
//  dans game.cpp, donc non réutilisables telles quelles depuis cet outil).
// -----------------------------------------------------------------------------
static void reset_faction(Map& map, Faction f) {
    for (auto& u : map.units) {
        if (!u.alive || u.faction != f) continue;
        u.moves_left = unit_stats(u.type).move_range;
        u.action_used = false;
    }
}

static Hex castle_pos(const LevelDef& def) { return Hex{ def.width - 1, def.height / 2 }; }

static bool player_victory(const Map& map, const LevelDef& def, int turn) {
    switch (def.condition) {
        case VictoryCondition::EliminateAll:
            return map.count_alive(Faction::Enemy) == 0;
        case VictoryCondition::CaptureCastle: {
            const Unit* u = map.unit_at(castle_pos(def));
            return u && u->alive && u->faction == Faction::Player;
        }
        case VictoryCondition::SurviveTurns:
            return turn > def.survive_turns && map.count_alive(Faction::Player) > 0;
    }
    return false;
}
static bool player_defeat(const Map& map) { return map.count_alive(Faction::Player) == 0; }

// -----------------------------------------------------------------------------
//  Bot cote joueur : pour chaque unite pouvant agir, attaque si une cible est
//  a portee (privilegie l'ennemi le plus faible / achevable), sinon avance
//  vers l'ennemi vivant le plus proche (ou vers le chateau si CaptureCastle),
//  puis retente une attaque depuis la nouvelle position.
// -----------------------------------------------------------------------------
static Hex nearest_enemy_pos(const Map& map, const Hex& from) {
    Hex best{-1,-1};
    int best_d = std::numeric_limits<int>::max();
    for (const auto& u : map.units) {
        if (!u.alive || u.faction != Faction::Enemy) continue;
        int d = hex_distance(from, u.pos);
        if (d < best_d) { best_d = d; best = u.pos; }
    }
    return best;
}

static Hex pick_attack_target(Map& map, const std::vector<Hex>& atk) {
    Hex best = atk[0];
    int best_hp = std::numeric_limits<int>::max();
    for (const auto& h : atk) {
        Unit* t = map.unit_at(h);
        if (!t) continue;
        if (t->hp < best_hp) { best_hp = t->hp; best = h; }
    }
    return best;
}

// Repli : parmi les cases atteignables, choisit celle qui maximise la
// distance a l'ennemi le plus proche tout en profitant d'un bon bonus de
// terrain (meme logique de score, direction inversee).
static Hex pick_retreat_tile(const Map& map, const std::vector<Hex>& reach, const Hex& threat) {
    Hex best = reach[0];
    float best_score = -1e9f;
    for (const auto& h : reach) {
        int d = hex_distance(h, threat);
        int bonus = terrain_info(map.get_tile(h).terrain).def_bonus;
        float score = (float)d + (bonus / 50.0f);
        if (score > best_score) { best_score = score; best = h; }
    }
    return best;
}

// -----------------------------------------------------------------------------
//  Bot cote joueur : attaque toujours si une cible est a portee (refuser un
//  coup gratuit ne protege pas l'unite : l'ennemi l'attaquera de toute facon
//  a son tour), sinon avance vers l'ennemi le plus proche (ou le chateau),
//  en preferant le terrain defensif et en restant groupe avec le reste de
//  l'armee. Repli vers un bon terrain si gravement blessee -- une seule fois
//  par unite et par bataille (fuir indefiniment ne fait que retarder la mort
//  si l'ennemi finit toujours par rattraper ; mieux vaut se stabiliser une
//  fois puis reprendre le combat).
// -----------------------------------------------------------------------------
static void try_attack(Map& map, Unit& u) {
    auto atk = map.attackable_tiles(u);
    if (atk.empty()) return;
    Hex tgt_pos = pick_attack_target(map, atk);
    Unit* target = map.unit_at(tgt_pos);
    if (target) resolve_attack(u, *target, map);
}

static bool g_verbose_actions = false;

static void bot_player_turn(Map& map, const LevelDef& def, std::set<uint32_t>& retreated_once) {
    reset_faction(map, Faction::Player);
    Hex goal_castle = castle_pos(def);

    size_t n = map.units.size(); // snapshot : combat ne fait qu'alterer hp/alive, pas la taille du vector
    for (size_t i = 0; i < n; ++i) {
        Unit& u = map.units[i];
        if (!u.alive || u.faction != Faction::Player || !u.can_act()) continue;
        const UnitStats& ust = unit_stats(u.type);
        int hp_before = u.hp;

        auto atk0 = map.attackable_tiles(u);
        try_attack(map, u);
        if (g_verbose_actions && !atk0.empty())
            printf("      unite#%zu (%s, pv%d) attaque -> pv apres=%d\n", i, ust.name_fr, hp_before, u.alive ? u.hp : 0);
        if (!u.alive || u.moves_left <= 0) continue;

        bool critical = u.hp <= ust.hp_max * 0.3f;
        bool last_stand = map.count_alive(Faction::Player) <= 1;
        bool already_retreated = retreated_once.count(u.id) > 0;
        auto reach = map.reachable_tiles(u);
        if (reach.empty()) continue;

        if (critical && !last_stand && !already_retreated) {
            Hex threat = nearest_enemy_pos(map, u.pos);
            if (threat.q >= 0) {
                Hex candidate = pick_retreat_tile(map, reach, threat);
                if (hex_distance(candidate, threat) > hex_distance(u.pos, threat)) {
                    if (g_verbose_actions) printf("      unite#%zu (%s) replie (premiere fois)\n", i, ust.name_fr);
                    u.pos = candidate;
                    u.moves_left = 0;
                    retreated_once.insert(u.id);
                    continue;
                }
            }
        }

        Hex goal = (def.condition == VictoryCondition::CaptureCastle) ? goal_castle : nearest_enemy_pos(map, u.pos);
        if (goal.q < 0) continue; // pas d'ennemi vivant (ne devrait pas arriver ici)

        Hex best_tile = u.pos;
        float best_score = 1e9f;
        for (const auto& h : reach) {
            int d = hex_distance(h, goal);
            int bonus = terrain_info(map.get_tile(h).terrain).def_bonus;
            int ally_d = 99;
            for (const auto& al : map.units) {
                if (!al.alive || al.faction != Faction::Player || al.id == u.id) continue;
                ally_d = std::min(ally_d, hex_distance(h, al.pos));
            }
            if (ally_d == 99) ally_d = 0;
            float cohesion = (ally_d > 3) ? (ally_d - 3) * 0.6f : 0.0f;
            float score = (float)d - (bonus / 50.0f) + cohesion;
            if (score < best_score) { best_score = score; best_tile = h; }
        }
        if (g_verbose_actions && best_tile != u.pos)
            printf("      unite#%zu (%s) avance de (%d,%d) vers (%d,%d)\n", i, ust.name_fr, u.pos.q, u.pos.r, best_tile.q, best_tile.r);
        u.pos = best_tile;
        u.moves_left = 0;

        if (!u.action_used) try_attack(map, u);
    }
}


// -----------------------------------------------------------------------------
//  Bataille complete
// -----------------------------------------------------------------------------
struct BattleResult { bool won=false, lost=false, timeout=false; int turns=0; int survivors=0; };

static BattleResult simulate_battle(PlayerArmy& army, int level_index, bool verbose = false) {
    const LevelDef& def = level_get(level_index);
    Map map;
    level_build_map(def, map);
    campaign_deploy_army(army, def, map);

    BattleResult res;
    const int MAX_TURNS = 60;
    std::set<uint32_t> retreated_once;
    for (int turn = 1; turn <= MAX_TURNS; ++turn) {
        bot_player_turn(map, def, retreated_once);
        if (verbose) {
            printf("    [T%02d apres joueur] joueur=%d ennemis=%d\n", turn,
                   map.count_alive(Faction::Player), map.count_alive(Faction::Enemy));
        }
        if (player_victory(map, def, turn)) { res.won = true; res.turns = turn; break; }
        if (player_defeat(map)) { res.lost = true; res.turns = turn; break; }

        reset_faction(map, Faction::Enemy);
        ai_play_turn(map);
        if (verbose) {
            printf("    [T%02d apres ennemi] joueur=%d ennemis=%d\n", turn,
                   map.count_alive(Faction::Player), map.count_alive(Faction::Enemy));
        }
        if (player_defeat(map)) { res.lost = true; res.turns = turn; break; }
        if (player_victory(map, def, turn)) { res.won = true; res.turns = turn; break; }
    }
    if (!res.won && !res.lost) res.timeout = true, res.turns = MAX_TURNS;
    res.survivors = map.count_alive(Faction::Player);

    if (res.won) campaign_collect_results(army, map, def);
    return res;
}

// -----------------------------------------------------------------------------
//  Boutique (bot) entre deux cartes : soigne d'abord, recrute de l'infanterie
//  (la moins chere) avec le reste, jusqu'a 10 unites dans l'effectif.
// -----------------------------------------------------------------------------
// Recrute en alternant Infanterie (encaisse, pas cher) et Archer (degats a
// distance, sans riposte) -> composition robuste plutot que de tout miser
// sur des unites a fort potentiel mais fragiles (Mage 6 PV, etc.), qui ont
// fait chuter la survie generale lors d'un premier essai.
static bool bot_recruit_best(PlayerArmy& army) {
    int infantry_n = 0, archer_n = 0;
    for (const auto& u : army.units) {
        if (u.type == UnitType::Infantry) infantry_n++;
        if (u.type == UnitType::Archer) archer_n++;
    }
    UnitType want = (infantry_n <= archer_n) ? UnitType::Infantry : UnitType::Archer;
    if (campaign_recruit(army, want)) return true;
    // repli si le type voulu n'est plus abordable
    static const UnitType fallback[] = {
        UnitType::Infantry, UnitType::Archer, UnitType::Cavalry, UnitType::Sapper,
        UnitType::Mage, UnitType::Griffon, UnitType::Hero, UnitType::SiegeEngine,
    };
    for (UnitType t : fallback)
        if (campaign_recruit(army, t)) return true;
    return false;
}

static void bot_shop(PlayerArmy& army) {
    const int MIN_ROSTER = 4; // remonter a l'effectif de depart avant de peaufiner les PV
    while ((int)army.units.size() < MIN_ROSTER) {
        if (!bot_recruit_best(army)) break;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i < army.units.size(); ++i) {
            const UnitStats& st = unit_stats(army.units[i].type);
            int missing = st.hp_max - army.units[i].hp;
            if (missing <= 0) continue;
            int heal = std::min(5, missing);
            if (campaign_heal(army, (int)i, heal)) changed = true;
        }
    }
    while ((int)army.units.size() < 10) {
        if (!bot_recruit_best(army)) break;
    }
}

// -----------------------------------------------------------------------------
int main(int argc, char** argv) {
    if (argc >= 5 && std::string(argv[1]) == "trials") {
        // trials <nb_essais> <niveau 1-based> <easy|normal|hard>
        // Rejoue la meme carte plusieurs fois de suite (RNG qui avance
        // naturellement entre essais, contrairement a relancer le process --
        // srand(time(...)) redonnerait la meme graine sur des lancements trop
        // rapproches). Utile pour evaluer un mecanisme a alea (cf. attaque
        // Difficile aleatoire dans difficulty.cpp).
        int trials = atoi(argv[2]);
        int level_1based = atoi(argv[3]);
        std::string diff_s = argv[4];
        Difficulty d = diff_s == "easy" ? Difficulty::Easy : diff_s == "hard" ? Difficulty::Hard : Difficulty::Normal;
        difficulty_set(d);
        int wins = 0, losses = 0, timeouts = 0;
        int total_turns = 0, total_kills = 0;
        for (int k = 0; k < trials; ++k) {
            PlayerArmy army;
            campaign_new(army);
            for (int i = 0; i < level_1based - 1; ++i) {
                BattleResult r0 = simulate_battle(army, i);
                if (!r0.won) break;
                bot_shop(army);
            }
            BattleResult r = simulate_battle(army, level_1based - 1);
            if (r.won) wins++; else if (r.lost) losses++; else timeouts++;
            total_turns += r.turns;
        }
        printf("Carte %d, %s, %d essais : %d gagnes, %d perdus, %d timeout (tours moy. %.1f)\n",
               level_1based, difficulty_name(d), trials, wins, losses, timeouts, (double)total_turns / trials);
        (void)total_kills;
        return 0;
    }

    if (argc >= 4 && std::string(argv[1]) == "verbose") {
        // Diagnostic cible : verbose <niveau 1-based> <easy|normal|hard>
        int level_1based = atoi(argv[2]);
        std::string diff_s = argv[3];
        Difficulty d = diff_s == "easy" ? Difficulty::Easy : diff_s == "hard" ? Difficulty::Hard : Difficulty::Normal;
        difficulty_set(d);
        PlayerArmy army;
        campaign_new(army);
        // Rejoue les niveaux precedents "normalement" (silencieux) pour arriver
        // avec une armee/PV de campagne coherents, puis detail le niveau cible.
        for (int i = 0; i < level_1based - 1; ++i) {
            BattleResult r = simulate_battle(army, i);
            if (!r.won) { printf("(campagne deja bloquee avant la carte %d)\n", level_1based); return 1; }
            bot_shop(army);
        }
        printf("Diagnostic : Carte %d en %s\n", level_1based, difficulty_name(d));
        g_verbose_actions = true;
        BattleResult r = simulate_battle(army, level_1based - 1, true);
        printf("Resultat : %s (tour %d)\n", r.won ? "GAGNE" : r.lost ? "PERDU" : "TIMEOUT", r.turns);
        return 0;
    }

    Difficulty diffs[] = { Difficulty::Easy, Difficulty::Normal, Difficulty::Hard };
    int global_first_failure = -1;

    for (Difficulty d : diffs) {
        difficulty_set(d);
        PlayerArmy army;
        campaign_new(army);
        int total = level_count();

        printf("\n================ %s (%d cartes) ================\n", difficulty_name(d), total);
        for (int i = 0; i < total; ++i) {
            const LevelDef& def = level_get(i);
            BattleResult r = simulate_battle(army, i);

            const char* status = r.won ? "GAGNE " : r.lost ? "PERDU " : "TIMEOUT";
            printf("  Carte %2d/%-2d [%-34s] %s  (tour %2d, survivants %d, effectif %2zu, PV camp %4d)\n",
                   i + 1, total, def.name_fr, status, r.turns, r.survivors,
                   army.units.size(), army.victory_points);

            if (!r.won) {
                printf("  --> Campagne bloquee a la carte %d en %s.\n", i + 1, difficulty_name(d));
                if (global_first_failure < 0 || i < global_first_failure) global_first_failure = i;
                break;
            }
            bot_shop(army);
        }
    }

    printf("\n================ Resume ================\n");
    printf("(voir le detail ci-dessus par difficulte)\n");
    return 0;
}
