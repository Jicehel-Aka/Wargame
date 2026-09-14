idf_component_register(
    SRCS
        app_main.cpp
        language.cpp
        difficulty.cpp

        core/graphics.cpp
        core/input.cpp
        core/filesystem.cpp
        core/audio.cpp
        core/sprites.cpp
        core/story_image.cpp

        game/hex.cpp
        game/units_db.cpp
        game/map.cpp
        game/combat.cpp
        game/ai.cpp
        game/levels_data.cpp
        game/campaign.cpp
        game/game.cpp

        tasks/task_game.cpp

    INCLUDE_DIRS
        .
        core
        game
        tasks

    REQUIRES
        gamebuino
        app_update
        esp_partition
        spi_flash
        esp_system
        nvs_flash
)
