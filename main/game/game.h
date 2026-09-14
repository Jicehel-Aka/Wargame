/*
===============================================================================
  game.h — État de jeu principal (machine à états du wargame)
===============================================================================
*/
#pragma once
#include "map.h"
#include "camera.h"
#include "campaign.h"
#include "levels_data.h"

namespace wg {

enum class GameMode {
    Title,
    Briefing,        // écran d'histoire avant la bataille
    Battle,
    VictoryScreen,
    DefeatScreen,
    Shop,            // boutique de renforts entre deux cartes
    CampaignComplete
};

enum class TurnPhase { Player, EnemyAnim };

struct GameState {
    GameMode mode = GameMode::Title;
    TurnPhase phase = TurnPhase::Player;

    Map map;
    Camera camera;
    PlayerArmy army;

    Hex cursor {0, 0};
    int selected_index = -1; // index dans map.units, -1 = aucune sélection
    bool moved_this_selection = false;

    int current_level = 0;
    int turn_counter = 1; // numéro du tour courant (pour la condition de victoire "survivre N tours")
    int shop_cursor = 0;         // index sélectionné dans la colonne "Mon armée"
    int shop_recruit_cursor = 0; // index sélectionné dans la colonne "Recrutement" (UnitType)
    int shop_column = 0;         // 0 = Mon armée (soin), 1 = Recrutement (achat)
    bool show_help = false; // overlay d'aide affiché tant que MENU est maintenu
};

void game_init(GameState& gs);
void game_update(GameState& gs, float dt);
void game_draw(GameState& gs);

} // namespace wg
