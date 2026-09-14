#include "task_game.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../core/input.h"
#include "../core/graphics.h"
#include "../game/game.h"

namespace wg {

void task_game(void* arg) {
    gfx_init();
    input_init();

    GameState gs;
    game_init(gs);

    const TickType_t frame_ticks = pdMS_TO_TICKS(25); // ~40 FPS
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        input_poll();
        game_update(gs, 0.025f);
        game_draw(gs);
        vTaskDelayUntil(&last_wake, frame_ticks);
    }
}

} // namespace wg
