/* -----------------------------------------------------------------------------
    Fichier : game.cpp
    Projet  : Wargame_Aka
    Auteur  : Jean-Charles (Aka)
    Objet   : Gestion du gameplay principal, boucle de jeu, rendu et logique
              des différents modes (Title, Briefing, Battle, Shop, etc.)

    Description :
        Ce fichier contient :
        - L'initialisation du jeu et du chargement de campagne
        - La boucle d'update selon le GameMode
        - Le rendu de la carte hexagonale et des HUD
        - Les écrans de victoire, défaite, briefing, boutique
        - Les helpers internes (déplacements, caméra, etc.)

    Notes :
        - Toute nouvelle valeur ajoutée à GameMode doit être gérée dans
          game_update() et game_draw(), sinon le compilateur génère un warning.
        - Les warnings sont traités comme des erreurs (-Werror), donc chaque
          case manquante doit être ajoutée explicitement.

    ----------------------------------------------------------------------------- */
	
#include "game.h"
#include "../core/input.h"
#include "../core/graphics.h"
#include "../core/audio.h"
#include "../core/sprites.h"
#include "../core/story_image.h"
#include "../language.h"
#include "../difficulty.h"
#include "combat.h"
#include "ai.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace wg {

// -----------------------------------------------------------------------------
//  Helpers internes
// -----------------------------------------------------------------------------
static int find_unit_index(const Map& map, const Hex& h) {
    for (size_t i = 0; i < map.units.size(); ++i)
        if (map.units[i].alive && map.units[i].pos == h) return (int)i;
    return -1;
}

// Cherche la prochaine unite du joueur pouvant encore agir, en partant de
// from_index (exclu) et en bouclant sur la liste. from_index = -1 si le
// curseur n'est sur aucune unite -> part du debut de la liste.
static int find_next_selectable_unit(const Map& map, int from_index) {
    int n = (int)map.units.size();
    if (n == 0) return -1;
    for (int step = 1; step <= n; ++step) {
        int idx = ((from_index + step) % n + n) % n;
        const Unit& u = map.units[idx];
        if (u.alive && u.faction == Faction::Player && u.can_act()) return idx;
    }
    return -1;
}

static void reset_faction_for_new_turn(Map& map, Faction f) {
    for (auto& u : map.units) {
        if (!u.alive || u.faction != f) continue;
        u.moves_left = unit_stats(u.type).move_range;
        u.action_used = false;
    }
}

static void start_battle(GameState& gs, int level_index) {
	    const LevelDef& def = level_get(level_index);
    level_build_map(def, gs.map);
    campaign_deploy_army(gs.army, def, gs.map);
    gs.current_level = level_index;
    gs.selected_index = -1;
    gs.phase = TurnPhase::Player;
    gs.turn_counter = 1;
    gs.mode = GameMode::Briefing; // l'histoire s'affiche avant que la bataille ne commence
    gs.cursor = def.player_deploy_zone.empty() ? Hex{0,0} : def.player_deploy_zone[0];
    gs.camera.zoom_fp = 224;
    gs.camera.offset_x = 20.0f;
    gs.camera.offset_y = 46.0f;
}

// Vérifie si la condition de victoire propre à la carte est remplie.
// Retourne true si la victoire doit être déclarée immédiatement.
static bool check_victory_condition(GameState& gs) {
    const LevelDef& def = level_get(gs.current_level);
    switch (def.condition) {
        case VictoryCondition::EliminateAll:
            return gs.map.count_alive(Faction::Enemy) == 0;
        case VictoryCondition::CaptureCastle:
            for (const auto& u : gs.map.units) {
                if (u.alive && u.faction == Faction::Player &&
                    gs.map.get_tile(u.pos).terrain == Terrain::Castle)
                    return true;
            }
            return false;
        case VictoryCondition::SurviveTurns:
            return gs.turn_counter > def.survive_turns;
    }
    return false;
}

static void end_player_turn(GameState& gs) {
    gs.selected_index = -1;

    // Condition "Capture du château" : vérifiée juste après les actions du joueur,
    // avant même que l'ennemi ne puisse réagir.
    const LevelDef& def_check = level_get(gs.current_level);
    if (def_check.condition == VictoryCondition::CaptureCastle && check_victory_condition(gs)) {
        campaign_collect_results(gs.army, gs.map, def_check);
        campaign_save(gs.army);
        gs.mode = GameMode::VictoryScreen;
        sfx_castle_capture();
        return;
    }

    gs.phase = TurnPhase::EnemyAnim;
    reset_faction_for_new_turn(gs.map, Faction::Enemy);
    ai_play_turn(gs.map);

    if (gs.map.count_alive(Faction::Player) == 0) {
        gs.mode = GameMode::DefeatScreen;
        sfx_defeat();
        return;
    }

    const LevelDef& def = level_get(gs.current_level);
    if (check_victory_condition(gs)) {
        campaign_collect_results(gs.army, gs.map, def);
        campaign_save(gs.army);
        gs.mode = GameMode::VictoryScreen;
        sfx_victory();
        return;
    }

    gs.turn_counter++;
    reset_faction_for_new_turn(gs.map, Faction::Player);
    gs.phase = TurnPhase::Player;
}

// Garde le curseur visible à l'écran en ajustant le pan de la caméra
static void keep_cursor_in_view(GameState& gs) {
    float px, py;
    hex_to_pixel(gs.cursor, gs.camera.hex_size(), gs.camera.offset_x, gs.camera.offset_y, &px, &py);
    const float margin = 24.0f;
    if (px < margin) gs.camera.offset_x += (margin - px);
    if (px > gfx_width() - margin) gs.camera.offset_x -= (px - (gfx_width() - margin));
    if (py < margin + 16) gs.camera.offset_y += (margin + 16 - py);
    // Bande HUD basse sur 4 lignes (jusqu'a 48px, cf draw_hud) -> marge dediee,
    // plus genereuse que la marge haute (38px de HUD fixe).
    if (py > gfx_height() - margin - 28) gs.camera.offset_y -= (py - (gfx_height() - margin - 28));
}

// -----------------------------------------------------------------------------
//  Initialisation
// -----------------------------------------------------------------------------
void game_init(GameState& gs) {
    campaign_load(gs.army);
    if (gs.army.campaign_map_index >= level_count()) {
        gs.mode = GameMode::CampaignComplete;
    } else {
        gs.mode = GameMode::Title;
    }
}

// -----------------------------------------------------------------------------
//  Mise à jour : Battle / Player phase
// -----------------------------------------------------------------------------
static void update_battle_player(GameState& gs) {
    if (key_pressed(gb_buttons::KEY_RIGHT)) gs.cursor.q++;
    if (key_pressed(gb_buttons::KEY_LEFT))  gs.cursor.q--;
    if (key_pressed(gb_buttons::KEY_DOWN))  gs.cursor.r++;
    if (key_pressed(gb_buttons::KEY_UP))    gs.cursor.r--;
    gs.cursor.q = std::max(0, std::min(gs.map.width - 1, gs.cursor.q));
    gs.cursor.r = std::max(0, std::min(gs.map.height - 1, gs.cursor.r));

    if (key_pressed(gb_buttons::KEY_L1)) gs.camera.zoom_out();
    if (key_pressed(gb_buttons::KEY_R1)) gs.camera.zoom_in();

    keep_cursor_in_view(gs);

    if (key_pressed(gb_buttons::KEY_A)) {
        if (gs.selected_index < 0) {
            int idx = find_unit_index(gs.map, gs.cursor);
            if (idx >= 0 && gs.map.units[idx].faction == Faction::Player &&
                gs.map.units[idx].can_act()) {
                gs.selected_index = idx;
                sfx_select();
            } else {
                sfx_invalid();
            }
        } else {
            Unit& sel = gs.map.units[gs.selected_index];
            auto atk = gs.map.attackable_tiles(sel);
            bool is_attack = std::find(atk.begin(), atk.end(), gs.cursor) != atk.end();

            if (is_attack) {
                Unit* target = gs.map.unit_at(gs.cursor);
                if (target) resolve_attack(sel, *target, gs.map);
                gs.selected_index = -1;
            } else if (gs.cursor == sel.pos) {
                gs.selected_index = -1;
                sfx_cancel();
            } else {
                auto reach = gs.map.reachable_tiles(sel);
                bool is_reach = std::find(reach.begin(), reach.end(), gs.cursor) != reach.end();
                if (is_reach) {
                    sel.pos = gs.cursor;
                    sel.moves_left = 0; // mouvement = toute la jauge de PM pour cette v1
                    sfx_move();
                    if (sel.action_used) gs.selected_index = -1;
                } else {
                    sfx_invalid();
                }
            }
        }
    }

    if (key_pressed(gb_buttons::KEY_B)) {
        if (gs.selected_index >= 0) {
            sfx_cancel();
            gs.selected_index = -1;
        } else {
            int cur = find_unit_index(gs.map, gs.cursor);
            int next = find_next_selectable_unit(gs.map, cur);
            if (next >= 0) {
                gs.cursor = gs.map.units[next].pos;
                sfx_menu_move();
            } else {
                sfx_invalid();
            }
        }
    }

    gs.show_help = key_down(gb_buttons::KEY_MENU);

    if (key_pressed(gb_buttons::KEY_C)) {
        sfx_end_turn();
        end_player_turn(gs);
    }
}

// -----------------------------------------------------------------------------
//  Mise à jour : boutique de renforts (2 colonnes : Mon armee / Recrutement)
// -----------------------------------------------------------------------------
static void update_shop(GameState& gs) {
    int army_n = (int)gs.army.units.size();
    int recruit_n = (int)UnitType::COUNT;

    if (key_pressed(gb_buttons::KEY_LEFT)  && gs.shop_column != 0) { gs.shop_column = 0; sfx_menu_move(); }
    if (key_pressed(gb_buttons::KEY_RIGHT) && gs.shop_column != 1) { gs.shop_column = 1; sfx_menu_move(); }

    if (gs.shop_column == 0) {
        if (key_pressed(gb_buttons::KEY_UP))   { gs.shop_cursor--; sfx_menu_move(); }
        if (key_pressed(gb_buttons::KEY_DOWN)) { gs.shop_cursor++; sfx_menu_move(); }
        if (army_n > 0) {
            if (gs.shop_cursor < 0) gs.shop_cursor = army_n - 1;
            if (gs.shop_cursor >= army_n) gs.shop_cursor = 0;
        } else {
            gs.shop_cursor = 0;
        }
        if (key_pressed(gb_buttons::KEY_A) && army_n > 0) {
            if (campaign_heal(gs.army, gs.shop_cursor, 5)) sfx_recruit();
            else sfx_invalid();
        }
    } else {
        if (key_pressed(gb_buttons::KEY_UP))   { gs.shop_recruit_cursor--; sfx_menu_move(); }
        if (key_pressed(gb_buttons::KEY_DOWN)) { gs.shop_recruit_cursor++; sfx_menu_move(); }
        if (gs.shop_recruit_cursor < 0) gs.shop_recruit_cursor = recruit_n - 1;
        if (gs.shop_recruit_cursor >= recruit_n) gs.shop_recruit_cursor = 0;

        if (key_pressed(gb_buttons::KEY_A)) {
            if (campaign_recruit(gs.army, (UnitType)gs.shop_recruit_cursor)) sfx_recruit();
            else sfx_invalid();
        }
    }

    if (key_pressed(gb_buttons::KEY_C)) {
        campaign_save(gs.army);
        sfx_end_turn();
        int next = gs.army.campaign_map_index;
        if (next >= level_count()) {
            gs.mode = GameMode::CampaignComplete;
        } else {
            start_battle(gs, next);
        }
    }
}

// -----------------------------------------------------------------------------
//  Mise à jour générale
// -----------------------------------------------------------------------------
void game_update(GameState& gs, float dt) {
    check_return_to_loader();

    switch (gs.mode) {
        case GameMode::Title: {
            if (key_pressed(gb_buttons::KEY_LEFT))  { language_prev(); sfx_menu_move(); }
            if (key_pressed(gb_buttons::KEY_RIGHT)) { language_next(); sfx_menu_move(); }

            // La difficulte ne se choisit qu'en debut de campagne (aucune carte
            // encore jouee) -> evite de la changer en cours de route pour
            // esquiver un passage difficile. Combo L1+R1 maintenu = mode debug,
            // pour pouvoir tester le contenu Difficile sans tout recommencer.
            bool campaign_started = gs.army.campaign_map_index > 0;
            bool debug_override = key_down(gb_buttons::KEY_L1) && key_down(gb_buttons::KEY_R1);
            if (!campaign_started || debug_override) {
                if (key_pressed(gb_buttons::KEY_UP))   { difficulty_next(); sfx_menu_move(); }
                if (key_pressed(gb_buttons::KEY_DOWN)) { difficulty_prev(); sfx_menu_move(); }
            } else if (key_pressed(gb_buttons::KEY_UP) || key_pressed(gb_buttons::KEY_DOWN)) {
                sfx_invalid();
            }

            if (key_pressed(gb_buttons::KEY_A))
                start_battle(gs, gs.army.campaign_map_index);
            break;
        }

        case GameMode::Briefing:
		    // Le joueur lit le texte d'introduction du niveau.
			// Une simple pression sur A lance la bataille.
            if (key_pressed(gb_buttons::KEY_A))
                gs.mode = GameMode::Battle;
            break;

        case GameMode::Battle:
            if (gs.phase == TurnPhase::Player)
                update_battle_player(gs);
            break;

        case GameMode::VictoryScreen:
            if (key_pressed(gb_buttons::KEY_A)) {
                gs.shop_cursor = 0;
                gs.shop_recruit_cursor = 0;
                gs.shop_column = 0;
                gs.mode = GameMode::Shop;
            }
            break;

        case GameMode::DefeatScreen:
            if (key_pressed(gb_buttons::KEY_A))
                start_battle(gs, gs.current_level);
            break;

        case GameMode::Shop:
            update_shop(gs);
            break;

        case GameMode::CampaignComplete:
            break;
    }
}

// -----------------------------------------------------------------------------
//  Rendu : carte hexagonale
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//  Echelle d'affichage des sprites (terrain + unites)
//  -> a ajuster ici si les sprites paraissent trop petits/illisibles a l'ecran.
//     1.0 = taille "juste couvrante" d'origine.
//     Terrain : rester proche de 1.0-1.2 (au-dela, les tuiles carrees
//     debordent trop sur les hexagones voisins et cassent la grille -- teste
//     visuellement jusqu'a 2.0, illisible).
//     Unites : peut monter beaucoup plus haut (1.5-2.0+) sans ce probleme,
//     puisqu'elles se dessinent par-dessus, centrees sur leur propre case.
// -----------------------------------------------------------------------------
// Terrain : desormais decoupe a la forme hexagonale exacte (cf.
// gfx_drawBitmapScaledHexClip plus bas) -> plus besoin d'un facteur de
// surcouverture approximatif comme avant.
static constexpr float kUnitSpriteScale    = 2.2f;  // taille des icones d'unite

static void hex_corners(float cx, float cy, float size, float* xs, float* ys) {
    for (int i = 0; i < 6; ++i) {
        float angle = (float)(M_PI / 180.0 * (60.0 * i));
        xs[i] = cx + size * cosf(angle);
        ys[i] = cy + size * sinf(angle);
    }
}

static void draw_filled_hex(float cx, float cy, float size, Color color) {
    float xs[6], ys[6];
    hex_corners(cx, cy, size, xs, ys);
    for (int i = 0; i < 6; ++i) {
        int j = (i + 1) % 6;
        gfx_fillTriangle((int)cx, (int)cy, (int)xs[i], (int)ys[i], (int)xs[j], (int)ys[j], color);
    }
}

static void draw_hex_outline(float cx, float cy, float size, Color color) {
    float xs[6], ys[6];
    hex_corners(cx, cy, size, xs, ys);
    for (int i = 0; i < 6; ++i) {
        int j = (i + 1) % 6;
        gfx_drawLine((int)xs[i], (int)ys[i], (int)xs[j], (int)ys[j], color);
    }
}

// -----------------------------------------------------------------------------
//  Connectivite des routes : plutot qu'un sprite de route a motif fixe (qui ne
//  s'alignait pas avec les cases voisines), on dessine un segment procedural
//  du centre de la case vers chaque arete partagee avec un voisin "route-like"
//  (Route/Pont/Chateau). Chaque bord etant trace depuis les DEUX cases qui le
//  partagent, les segments se rejoignent exactement au milieu de l'arete ->
//  chemin continu plutot que des bouts de route paralleles disjoints.
// -----------------------------------------------------------------------------
// hex_neighbors() renvoie l'ordre E,NE,NW,W,SW,SE (cf hex.cpp) ; cette table
// convertit cet ordre vers l'indice d'arete correspondant dans hex_corners()
// (arete i = segment entre le coin i et le coin i+1).
static const int kDirToEdge[6] = {0, 5, 4, 3, 2, 1};

static bool connects_as_road(Terrain t) {
    return t == Terrain::Road || t == Terrain::Bridge || t == Terrain::Castle;
}

static void draw_road_segment(float cx, float cy, float ex, float ey, Color color, int width) {
    float dx = ex - cx, dy = ey - cy;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.5f) return;
    float nx = -dy / len, ny = dx / len; // normale unitaire, pour simuler une largeur de trait
    int half = width / 2;
    for (int w = -half; w <= half; ++w) {
        float ox = nx * (float)w, oy = ny * (float)w;
        gfx_drawLine((int)(cx + ox), (int)(cy + oy), (int)(ex + ox), (int)(ey + oy), color);
    }
}

static void draw_road_connections(Map& map, const Hex& h, float cx, float cy, float size) {
    Terrain t = map.get_tile(h).terrain;
    if (t != Terrain::Road && t != Terrain::Bridge) return;

    Color road_color = make_color(230, 217, 160); // chemin clair, coherent avec l'ancien sprite route
    int width = (int)(size * 0.22f);
    if (width < 1) width = 1;
    float apothem = size * 0.8660254f;

    auto neighbors = hex_neighbors(h);
    bool any_connection = false;
    for (int dir = 0; dir < 6; ++dir) {
        const Hex& nb = neighbors[dir];
        if (!map.in_bounds(nb) || !connects_as_road(map.get_tile(nb).terrain)) continue;
        int edge = kDirToEdge[dir];
        float angle = (float)(M_PI / 180.0 * (60.0 * edge + 30.0));
        float ex = cx + apothem * cosf(angle);
        float ey = cy + apothem * sinf(angle);
        draw_road_segment(cx, cy, ex, ey, road_color, width);
        any_connection = true;
    }
    if (!any_connection) {
        // Tronçon isole (aucun voisin route/pont/chateau) : simple renflement
        // au centre plutot qu'un chemin qui ne mènerait nulle part.
        gfx_fillCircle((int)cx, (int)cy, (int)(size * 0.22f), road_color);
    }
}

static void draw_multiline(int x, int y, int line_height, const char* text, Color color) {
    char line[64];
    int li = 0;
    while (*text) {
        if (*text == '\n' || li >= (int)sizeof(line) - 1) {
            line[li] = 0;
            gfx_text(x, y, line, color);
            y += line_height;
            li = 0;
            if (*text == '\n') text++;
        } else {
            line[li++] = *text++;
        }
    }
    if (li > 0) {
        line[li] = 0;
        gfx_text(x, y, line, color);
    }
}

static void objective_text(const LevelDef& def, GameState& gs, char* buf, size_t bufsize) {
    switch (def.condition) {
        case VictoryCondition::EliminateAll:
            snprintf(buf, bufsize, "%s", tr(TextID::HUD_OBJ_ELIMINATE));
            break;
        case VictoryCondition::CaptureCastle:
            snprintf(buf, bufsize, "%s", tr(TextID::HUD_OBJ_CASTLE));
            break;
        case VictoryCondition::SurviveTurns:
            snprintf(buf, bufsize, tr(TextID::HUD_OBJ_SURVIVE),
                      def.survive_turns, gs.turn_counter);
            break;
    }
}

static void draw_map(GameState& gs) {
    const Camera& cam = gs.camera;
    float size = cam.hex_size();

    for (int q = 0; q < gs.map.width; ++q) {
        for (int r = 0; r < gs.map.height; ++r) {
            Hex h{q, r};
            float cx, cy;
            hex_to_pixel(h, size, cam.offset_x, cam.offset_y, &cx, &cy);
            if (cx < -size || cx > gfx_width() + size || cy < -size || cy > gfx_height() + size)
                continue;
            Terrain t = gs.map.get_tile(h).terrain;
            const uint16_t* tspr = sprite_for_terrain(t);
            if (tspr) {
                // Le sprite source est un carre 16x16 ; on le met a l'echelle
                // pour couvrir tout juste le diametre de l'hexagone (2*size),
                // avec une legere marge de securite anti-arrondi (+5%), puis
                // on le decoupe a la forme hexagonale exacte (gfx_...HexClip)
                // -> bords nets entre deux terrains differents, plus de carre
                // qui deborde sur les cases voisines.
                int tile_zoom_fp = (int)(size * 2.05f * 256.0f / (float)SPRITE_SIZE);
                gfx_drawBitmapScaledHexClip((int)cx, (int)cy, SPRITE_SIZE, SPRITE_SIZE,
                                             tspr, COLOR_CHROMA_KEY, tile_zoom_fp, size);
            } else {
                draw_filled_hex(cx, cy, size, terrain_info(t).color);
            }
            draw_road_connections(gs.map, h, cx, cy, size);
            draw_hex_outline(cx, cy, size, COLOR_DARKGREY);
        }
    }

    if (gs.selected_index >= 0) {
        Unit& sel = gs.map.units[gs.selected_index];
        for (const auto& h : gs.map.reachable_tiles(sel)) {
            float cx, cy;
            hex_to_pixel(h, size, cam.offset_x, cam.offset_y, &cx, &cy);
            draw_hex_outline(cx, cy, size, COLOR_CYAN);
        }
        for (const auto& h : gs.map.attackable_tiles(sel)) {
            float cx, cy;
            hex_to_pixel(h, size, cam.offset_x, cam.offset_y, &cx, &cy);
            draw_hex_outline(cx, cy, size, COLOR_RED);
            draw_hex_outline(cx, cy, size - 1, COLOR_RED);
        }
    }

    for (const auto& u : gs.map.units) {
        if (!u.alive) continue;
        float cx, cy;
        hex_to_pixel(u.pos, size, cam.offset_x, cam.offset_y, &cx, &cy);
        if (cx < -size || cx > gfx_width() + size) continue;

        bool has_move = (u.moves_left > 0);
        bool has_action = !u.action_used;
        bool exhausted = !has_move && !has_action;

        Color body = (u.faction == Faction::Player) ? COLOR_BLUE : COLOR_RED;
        if (exhausted) body = (u.faction == Faction::Player) ? 0x0010 /*bleu sombre*/ : 0x4000 /*rouge sombre*/;

        float r = size * 0.55f;
        gfx_fillCircle((int)cx, (int)cy, (int)r, body);
        gfx_drawCircle((int)cx, (int)cy, (int)r, exhausted ? COLOR_GREY : COLOR_WHITE);

        const uint16_t* uspr = sprite_for_unit(u.type);
        if (uspr) {
            // Le disque de couleur (bleu/rouge selon le camp) reste visible en
            // halo derriere le sprite -> repere de camp immediat, sans avoir a
            // dessiner deux variantes de chaque unite.
            int unit_zoom_fp = (int)(r * 1.7f * kUnitSpriteScale * 256.0f / (float)SPRITE_SIZE);
            gfx_drawBitmapScaledCentered((int)cx, (int)cy, SPRITE_SIZE, SPRITE_SIZE,
                                          uspr, COLOR_CHROMA_KEY, unit_zoom_fp);
        } else {
            char glyph[2] = { unit_stats(u.type).glyph, 0 };
            gfx_text((int)cx - 4, (int)cy - 4, glyph, COLOR_WHITE);
        }

        // Pastille d'état (uniquement nos unités, en haut à droite du pion) :
        //   vert = mouvement ET action disponibles, jaune = un seul des deux, rien = épuisée
        if (u.faction == Faction::Player) {
            Color dot = exhausted ? COLOR_DARKGREY : ((has_move && has_action) ? COLOR_GREEN : COLOR_YELLOW);
            gfx_fillCircle((int)(cx + r * 0.7f), (int)(cy - r * 0.7f), 3, dot);
            gfx_drawCircle((int)(cx + r * 0.7f), (int)(cy - r * 0.7f), 3, COLOR_BLACK);
        }

        int max_hp = unit_stats(u.type).hp_max;
        int bar_w = (int)(size * 1.2f);
        int hp_w = (max_hp > 0) ? (bar_w * u.hp / max_hp) : 0;
        int bx = (int)(cx - bar_w / 2);
        int by = (int)(cy + r + 2);
        gfx_fillRect(bx, by, bar_w, 3, COLOR_DARKGREY);
        gfx_fillRect(bx, by, hp_w, 3, COLOR_GREEN);
    }

    {
        float cx, cy;
        hex_to_pixel(gs.cursor, size, cam.offset_x, cam.offset_y, &cx, &cy);
        draw_hex_outline(cx, cy, size, COLOR_YELLOW);
        draw_hex_outline(cx, cy, size - 1, COLOR_YELLOW);
    }
}

static void draw_hud(GameState& gs) {
    const LevelDef& def = level_get(gs.current_level);
    gfx_fillRect(0, 0, gfx_width(), 14, COLOR_BLACK);
    gfx_text(2, 3, level_name(def), COLOR_WHITE);

    char buf[48];
    Terrain cursor_t = gs.map.get_tile(gs.cursor).terrain;
    const TerrainInfo& ti = terrain_info(cursor_t);
    if (ti.move_cost >= 999)
        snprintf(buf, sizeof(buf), tr(TextID::HUD_TILE_IMPASSABLE), terrain_name(cursor_t));
    else
        snprintf(buf, sizeof(buf), tr(TextID::HUD_TILE_INFO),
                  terrain_name(cursor_t), ti.move_cost, ti.def_bonus);
    gfx_fillRect(0, 14, gfx_width(), 12, COLOR_BLACK);
    gfx_text(2, 16, buf, COLOR_WHITE);

    char obj_buf[48];
    objective_text(def, gs, obj_buf, sizeof(obj_buf));
    gfx_fillRect(0, 26, gfx_width(), 12, COLOR_BLACK);
    gfx_text(2, 28, obj_buf, COLOR_ORANGE);

    snprintf(buf, sizeof(buf), tr(TextID::HUD_VP_PLAYERS_ENEMIES),
              gs.army.victory_points,
              gs.map.count_alive(Faction::Player),
              gs.map.count_alive(Faction::Enemy));
    gfx_fillRect(0, gfx_height() - 12, gfx_width(), 12, COLOR_BLACK);
    gfx_text(2, gfx_height() - 10, buf, COLOR_WHITE);

    // Ligne 2 (bas) : rappel contextuel A/B — toujours visible, change selon l'etat.
    gfx_fillRect(0, gfx_height() - 24, gfx_width(), 12, COLOR_BLACK);
    // Ligne 3 (bas) : stats de l'unite sous le curseur (ci-dessous), sinon vide
    // (la barre reste dessinee pour ne pas faire "sauter" la vue de la carte).
    gfx_fillRect(0, gfx_height() - 36, gfx_width(), 12, COLOR_BLACK);
    // Ligne 4 (bas) : "C: fin du tour" — toujours visible tant que c'est le tour du joueur.
    gfx_fillRect(0, gfx_height() - 48, gfx_width(), 12, COLOR_BLACK);
    if (gs.phase == TurnPhase::Player)
        gfx_text_center(gfx_height() - 46, tr(TextID::HUD_HINT_END_TURN), COLOR_GREEN);

    // Ligne 3 (bas) : stats de l'unite SOUS LE CURSEUR (alliee ou ennemie -- pas
    // seulement l'unite activement selectionnee) : avec des sprites parfois
    // difficiles a distinguer d'un coup d'oeil, c'est la facon fiable
    // d'identifier une unite, y compris pour inspecter un ennemi (jusque-la
    // impossible : aucune info ennemie ne s'affichait avant cet ajout).
    Unit* cursor_unit = gs.map.unit_at(gs.cursor);
    if (cursor_unit) {
        const UnitStats& cst = unit_stats(cursor_unit->type);
        snprintf(buf, sizeof(buf), tr(TextID::HUD_UNIT_STATS),
                  unit_name(cursor_unit->type), cursor_unit->hp, cst.hp_max,
                  cst.attack, cst.defense, cst.atk_range);
        Color name_color = (cursor_unit->faction == Faction::Player) ? COLOR_YELLOW : COLOR_RED;
        gfx_text_center(gfx_height() - 34, buf, name_color);
    }

    if (gs.selected_index >= 0) {
        Unit& sel = gs.map.units[gs.selected_index];
        auto atk = gs.map.attackable_tiles(sel);
        if (!atk.empty())
            gfx_text_center(gfx_height() - 22, tr(TextID::HUD_HINT_ATTACK), COLOR_RED);
        else if (sel.moves_left > 0)
            gfx_text_center(gfx_height() - 22, tr(TextID::HUD_HINT_MOVE), COLOR_CYAN);
        else
            gfx_text_center(gfx_height() - 22, tr(TextID::HUD_HINT_NO_ACTION), COLOR_GREY);
    } else {
        gfx_text_center(gfx_height() - 22, tr(TextID::HUD_HINT_IDLE), COLOR_GREY);
    }
}

static void draw_help_overlay(GameState& gs) {
    int x = 18, y = 18, w = gfx_width() - 36, h = gfx_height() - 36;
    gfx_fillRect(x, y, w, h, COLOR_BLACK);
    gfx_drawRect(x, y, w, h, COLOR_WHITE);
    gfx_text_center(y + 8,  tr(TextID::HELP_TITLE), COLOR_YELLOW);
    gfx_text(x + 12, y + 26, tr(TextID::HELP_DPAD), COLOR_WHITE);
    gfx_text(x + 12, y + 38, tr(TextID::HELP_A), COLOR_WHITE);
    gfx_text(x + 12, y + 50, tr(TextID::HELP_A2), COLOR_WHITE);
    gfx_text(x + 12, y + 62, tr(TextID::HELP_A3), COLOR_WHITE);
    gfx_text(x + 12, y + 74, tr(TextID::HELP_B), COLOR_WHITE);
    gfx_text(x + 12, y + 86, tr(TextID::HELP_C), COLOR_WHITE);
    gfx_text(x + 12, y + 98, tr(TextID::HELP_ZOOM), COLOR_WHITE);
    gfx_text(x + 12, y + 112, tr(TextID::HELP_DOT_GREEN), COLOR_GREEN);
    gfx_text(x + 12, y + 124, tr(TextID::HELP_DOT_YELLOW), COLOR_YELLOW);

    // Légende des terrains
    gfx_text(x + 12, y + 138, tr(TextID::HELP_TERRAINS), COLOR_WHITE);
    int ly = y + 150;
    int lx = x + 12;
    for (int i = 0; i < (int)Terrain::COUNT; ++i) {
        const TerrainInfo& ti = terrain_info((Terrain)i);
        gfx_fillRect(lx, ly + 1, 8, 8, ti.color);
        gfx_drawRect(lx, ly + 1, 8, 8, COLOR_WHITE);
        gfx_text(lx + 12, ly, terrain_name((Terrain)i), COLOR_WHITE);
        lx += 90;
        if (lx > x + w - 80) { lx = x + 12; ly += 12; }
    }
}

static void draw_briefing(GameState& gs) {
	// Affiche le texte d'introduction du niveau (narration / contexte)
    const LevelDef& def = level_get(gs.current_level);
    gfx_clear(COLOR_BLACK);
    gfx_text_center(14, level_name(def), COLOR_YELLOW);

    const uint16_t* img = story_image_load(def.story_image);
    int text_y = 44;
    if (img) {
        int ix = (gfx_width() - STORY_IMAGE_W) / 2;
        gfx_drawBitmap(ix, 26, STORY_IMAGE_W, STORY_IMAGE_H, img, COLOR_CHROMA_KEY);
        gfx_drawRect(ix, 26, STORY_IMAGE_W, STORY_IMAGE_H, COLOR_DARKGREY);
        text_y = 26 + STORY_IMAGE_H + 10;
    } else {
        gfx_drawLine(20, 28, gfx_width() - 20, 28, COLOR_DARKGREY);
    }
    draw_multiline(20, text_y, 13, level_briefing(def), COLOR_WHITE);
    gfx_text_center(gfx_height() - 20, tr(TextID::BRIEFING_HINT_START), COLOR_GREEN);
}

static void draw_title(GameState& gs) {
    gfx_clear(COLOR_BLACK);
    gfx_text_center(70, tr(TextID::TITLE_GAME_NAME), COLOR_YELLOW);
    gfx_text_center(100, tr(TextID::TITLE_SUBTITLE), COLOR_WHITE);
    char buf[48];
    snprintf(buf, sizeof(buf), tr(TextID::TITLE_MAP_PROGRESS),
              gs.army.campaign_map_index + 1, level_count(), gs.army.victory_points);
    gfx_text_center(130, buf, COLOR_CYAN);

    snprintf(buf, sizeof(buf), "%s : %s", tr(TextID::TITLE_LANGUAGE_LABEL), language_name(language_get()));
    gfx_text_center(160, buf, COLOR_GREY);
    gfx_text_center(176, tr(TextID::TITLE_HINT_CHANGE_LANG), COLOR_GREY);

    snprintf(buf, sizeof(buf), "%s : %s", tr(TextID::TITLE_DIFFICULTY_LABEL), difficulty_name(difficulty_get()));
    gfx_text_center(190, buf, COLOR_ORANGE);
    bool campaign_started = gs.army.campaign_map_index > 0;
    bool debug_held = key_down(gb_buttons::KEY_L1) && key_down(gb_buttons::KEY_R1);
    if (!campaign_started || debug_held) {
        gfx_text_center(202, tr(TextID::TITLE_HINT_CHANGE_DIFF), COLOR_GREY);
    } else {
        gfx_text_center(202, tr(TextID::TITLE_DIFFICULTY_LOCKED), COLOR_GREY);
        gfx_text_center(214, tr(TextID::TITLE_HINT_DEBUG_UNLOCK), COLOR_GREY);
    }

    gfx_text_center(228, tr(TextID::TITLE_PRESS_A), COLOR_GREEN);
}

static void draw_victory(GameState& gs) {
    const LevelDef& def = level_get(gs.current_level);
    gfx_clear(COLOR_BLACK);
    gfx_text_center(20, tr(TextID::VICTORY_TITLE), COLOR_GREEN);
    draw_multiline(20, 42, 13, level_victory_text(def), COLOR_WHITE);
    char buf[48];
    snprintf(buf, sizeof(buf), tr(TextID::VICTORY_VP), gs.army.victory_points);
    gfx_text_center(gfx_height() - 36, buf, COLOR_CYAN);
    gfx_text_center(gfx_height() - 18, tr(TextID::VICTORY_CONTINUE), COLOR_YELLOW);
}

static void draw_defeat(GameState& gs) {
    gfx_clear(COLOR_BLACK);
    gfx_text_center(100, tr(TextID::DEFEAT_TITLE), COLOR_RED);
    gfx_text_center(130, tr(TextID::DEFEAT_TEXT), COLOR_WHITE);
    gfx_text_center(160, tr(TextID::DEFEAT_RETRY), COLOR_YELLOW);
}

static void draw_campaign_complete(GameState& gs) {
    gfx_clear(COLOR_BLACK);
    gfx_text_center(100, tr(TextID::CAMPAIGN_DONE_TITLE), COLOR_YELLOW);
    char buf[48];
    snprintf(buf, sizeof(buf), tr(TextID::CAMPAIGN_DONE_SCORE), gs.army.victory_points);
    gfx_text_center(130, buf, COLOR_WHITE);
}

static void draw_bar(int x, int y, int w, int h, int cur, int max_v, Color bg, Color fg) {
    gfx_fillRect(x, y, w, h, bg);
    if (max_v > 0) {
        int fw = (cur * w) / max_v;
        if (fw > 0) gfx_fillRect(x, y, fw, h, fg);
    }
}

// Colonne "Mon armee" : une ligne par unite possedee (sprite + nom + barre de PV).
static void draw_shop_army_column(GameState& gs, int x, int y0, int y1, int row_h) {
    bool active = (gs.shop_column == 0);
    int n = (int)gs.army.units.size();
    int visible = (y1 - y0) / row_h;
    if (visible < 1) visible = 1;

    int scroll = 0;
    if (n > visible) {
        scroll = gs.shop_cursor - visible / 2;
        if (scroll < 0) scroll = 0;
        if (scroll > n - visible) scroll = n - visible;
    }

    if (n == 0) {
        gfx_text(x + 4, y0 + 4, tr(TextID::SHOP_EMPTY_ARMY), COLOR_GREY);
        return;
    }

    char buf[40];
    int y = y0;
    for (int i = scroll; i < n && i < scroll + visible; ++i) {
        const ArmyUnit& au = gs.army.units[i];
        const UnitStats& st = unit_stats(au.type);
        bool sel = active && (i == gs.shop_cursor);

        if (sel) gfx_fillRect(x, y, 150, row_h - 1, 0x2104 /* bleu tres sombre */);

        const uint16_t* spr = sprite_for_unit(au.type);
        if (spr) gfx_drawBitmap(x + 2, y + 1, SPRITE_SIZE, SPRITE_SIZE, spr, COLOR_CHROMA_KEY);
        else { char g[2] = { st.glyph, 0 }; gfx_text(x + 6, y + 4, g, COLOR_WHITE); }

        Color name_c = sel ? COLOR_YELLOW : COLOR_WHITE;
        gfx_text(x + 20, y + 1, unit_name(au.type), name_c);

        draw_bar(x + 20, y + 11, 70, 4, au.hp, st.hp_max, COLOR_DARKGREY, COLOR_GREEN);
        snprintf(buf, sizeof(buf), "%2d/%2d", au.hp, st.hp_max);
        gfx_text(x + 96, y + 8, buf, COLOR_GREY);

        y += row_h;
    }
}

// Colonne "Recrutement" : une ligne par type d'unite disponible (sprite + nom + cout).
static void draw_shop_recruit_column(GameState& gs, int x, int y0, int y1, int row_h) {
    bool active = (gs.shop_column == 1);
    int n = (int)UnitType::COUNT;
    int visible = (y1 - y0) / row_h;
    if (visible < 1) visible = 1;

    int scroll = 0;
    if (n > visible) {
        scroll = gs.shop_recruit_cursor - visible / 2;
        if (scroll < 0) scroll = 0;
        if (scroll > n - visible) scroll = n - visible;
    }

    char buf[24];
    int y = y0;
    for (int i = scroll; i < n && i < scroll + visible; ++i) {
        UnitType type = (UnitType)i;
        const UnitStats& st = unit_stats(type);
        int cost = campaign_recruit_cost(type);
        bool affordable = gs.army.victory_points >= cost;
        bool sel = active && (i == gs.shop_recruit_cursor);

        if (sel) gfx_fillRect(x, y, 150, row_h - 1, 0x2104);

        const uint16_t* spr = sprite_for_unit(type);
        if (spr) gfx_drawBitmap(x + 2, y + 1, SPRITE_SIZE, SPRITE_SIZE, spr, COLOR_CHROMA_KEY);
        else { char g[2] = { st.glyph, 0 }; gfx_text(x + 6, y + 4, g, COLOR_WHITE); }

        Color name_c = sel ? COLOR_YELLOW : (affordable ? COLOR_WHITE : COLOR_GREY);
        gfx_text(x + 20, y + 1, unit_name(type), name_c);

        snprintf(buf, sizeof(buf), "%d PV", cost);
        Color cost_c = affordable ? COLOR_GREEN : COLOR_RED;
        gfx_text(x + 20, y + 11, buf, cost_c);

        y += row_h;
    }
}

static void draw_shop(GameState& gs) {
    gfx_clear(COLOR_BLACK);

    // --- Barre de titre + PV disponibles -----------------------------------
    gfx_text_center(3, tr(TextID::SHOP_TITLE), COLOR_YELLOW);
    char buf[48];
    gfx_fillRect(0, 14, gfx_width(), 14, COLOR_DARKGREY);
    snprintf(buf, sizeof(buf), tr(TextID::SHOP_VP_AVAILABLE), gs.army.victory_points);
    gfx_text_center(17, buf, COLOR_WHITE);

    // --- Deux colonnes -------------------------------------------------------
    const int col_left_x  = 4;
    const int col_right_x = 164;
    const int header_y = 32, header_h = 12;
    const int row_h = 18;
    const int hint_h = 26;   // 2 lignes d'aide en bas
    const int detail_h = 12; // 1 ligne de detail contextuel
    const int list_y0 = header_y + header_h + 2;
    const int list_y1 = gfx_height() - hint_h - detail_h - 2;

    bool col0_active = (gs.shop_column == 0);
    gfx_fillRect(col_left_x,  header_y, 150, header_h, col0_active ? COLOR_BLUE : COLOR_DARKGREY);
    gfx_fillRect(col_right_x, header_y, 150, header_h, !col0_active ? COLOR_BLUE : COLOR_DARKGREY);
    gfx_text(col_left_x + 4,  header_y + 2, tr(TextID::SHOP_COLUMN_ARMY),    COLOR_WHITE);
    gfx_text(col_right_x + 4, header_y + 2, tr(TextID::SHOP_COLUMN_RECRUIT), COLOR_WHITE);

    draw_shop_army_column(gs, col_left_x, list_y0, list_y1, row_h);
    draw_shop_recruit_column(gs, col_right_x, list_y0, list_y1, row_h);

    gfx_drawLine(160, header_y, 160, list_y1, COLOR_DARKGREY);

    // --- Ligne de detail contextuel (action possible sur la selection) -----
    int detail_y = list_y1 + 2;
    if (gs.shop_column == 0) {
        int n = (int)gs.army.units.size();
        if (n == 0) {
            // rien a afficher, deja indique dans la colonne
        } else {
            const ArmyUnit& au = gs.army.units[gs.shop_cursor];
            const UnitStats& st = unit_stats(au.type);
            int missing = st.hp_max - au.hp;
            if (missing <= 0) {
                gfx_text(4, detail_y, tr(TextID::SHOP_HEAL_FULL), COLOR_GREY);
            } else {
                int heal = (missing < 5) ? missing : 5;
                int cost = heal * campaign_heal_cost_per_hp();
                bool affordable = gs.army.victory_points >= cost;
                snprintf(buf, sizeof(buf), tr(TextID::SHOP_HEAL_ACTION), heal, cost);
                gfx_text(4, detail_y, buf, affordable ? COLOR_CYAN : COLOR_RED);
            }
        }
    } else {
        UnitType type = (UnitType)gs.shop_recruit_cursor;
        int cost = campaign_recruit_cost(type);
        bool affordable = gs.army.victory_points >= cost;
        snprintf(buf, sizeof(buf), tr(TextID::SHOP_RECRUIT_ACTION), cost);
        gfx_text(4, detail_y, buf, affordable ? COLOR_CYAN : COLOR_RED);
    }

    // --- Aide bas d'ecran ----------------------------------------------------
    gfx_text(4, gfx_height() - 22, tr(TextID::SHOP_SWITCH_HINT), COLOR_GREY);
    gfx_text(4, gfx_height() - 11, tr(TextID::SHOP_NEXT_HINT), COLOR_GREEN);
}

// -----------------------------------------------------------------------------
//  Rendu général
// -----------------------------------------------------------------------------
void game_draw(GameState& gs) {
    switch (gs.mode) {

        case GameMode::Title:
            draw_title(gs);
            break;

        case GameMode::Briefing:     // <-- AJOUT OBLIGATOIRE
            draw_briefing(gs);
            break;

        case GameMode::Battle:
            gfx_clear(COLOR_BLACK);
            draw_map(gs);
            draw_hud(gs);
            if (gs.show_help) draw_help_overlay(gs);
            break;

        case GameMode::VictoryScreen:
            draw_victory(gs);
            break;

        case GameMode::DefeatScreen:
            draw_defeat(gs);
            break;

        case GameMode::Shop:
            draw_shop(gs);
            break;

        case GameMode::CampaignComplete:
            draw_campaign_complete(gs);
            break;
    }

    gfx_present();
}


} // namespace wg
