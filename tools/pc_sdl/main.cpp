// =============================================================================
//  main.cpp — Port PC/SDL2 de Wargame AKA
// -----------------------------------------------------------------------------
//  Remplace app_main.cpp + tasks/task_game.cpp (spécifiques ESP-IDF/FreeRTOS)
//  par une boucle SDL2 classique. Le code du jeu lui-même (main/game/*,
//  main/core/*, main/language.cpp, main/difficulty.cpp) n'est PAS modifié :
//  seule cette couche "matériel" change.
//
//  Commandes clavier :
//    Flèches   = D-pad          Z = A     X = B     C = C     V = D
//    Q = L1    W = R1           Entrée = MENU        Tab = RUN
//    Échap ou fermer la fenêtre = quitter
//
//  Mode script (tests automatisés / rejouer un scénario) :
//    WARGAME_SCRIPT=chemin/vers/script.txt ./wargame_pc
//    Fichier texte, une instruction par ligne :
//      PRESS A|B|C|D|UP|DOWN|LEFT|RIGHT|L1|R1|MENU|RUN   (une frame de pression)
//      WAIT <ms>                                          (avance sans appui)
//      SCREENSHOT <chemin.bmp>                             (capture le framebuffer)
//    A la fin du script, le jeu continue en clavier interactif normal
//    (sauf si QUIT est la derniere ligne, qui ferme la fenetre).
// =============================================================================
#include <SDL.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "gb_core.h"
#include "gb_common.h"
#include "core/graphics.h"
#include "core/input.h"
#include "core/audio.h"
#include "core/sprites.h"
#include "language.h"
#include "difficulty.h"
#include "game/game.h"

gb_core g_core; // même variable globale que app_main.cpp (déclarée extern dans core/input.cpp)
extern uint16_t g_pc_key_state; // défini dans gb_core_pc.cpp
extern gb_pixel framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

static uint16_t sdl_scancode_to_gb_mask(const uint8_t* keys) {
    uint16_t m = 0;
    if (keys[SDL_SCANCODE_UP])     m |= GB_KEY_UP;
    if (keys[SDL_SCANCODE_DOWN])   m |= GB_KEY_DOWN;
    if (keys[SDL_SCANCODE_LEFT])   m |= GB_KEY_LEFT;
    if (keys[SDL_SCANCODE_RIGHT])  m |= GB_KEY_RIGHT;
    if (keys[SDL_SCANCODE_Z])      m |= GB_KEY_A;
    if (keys[SDL_SCANCODE_X])      m |= GB_KEY_B;
    if (keys[SDL_SCANCODE_C])      m |= GB_KEY_C;
    if (keys[SDL_SCANCODE_V])      m |= GB_KEY_D;
    if (keys[SDL_SCANCODE_Q])      m |= GB_KEY_L1;
    if (keys[SDL_SCANCODE_W])      m |= GB_KEY_R1;
    if (keys[SDL_SCANCODE_RETURN]) m |= GB_KEY_MENU;
    if (keys[SDL_SCANCODE_TAB])    m |= GB_KEY_RUN;
    return m;
}

// --------------------------------------------------------------------------
//  Mode script : instructions lues une fois au démarrage, rejouées frame par
//  frame sans dépendre du clavier X11 (utile sous Xvfb / CI, peu fiable pour
//  l'injection d'événements clavier réels).
// --------------------------------------------------------------------------
struct ScriptStep {
    enum Kind { Press, Wait, Screenshot, Quit, Hold } kind;
    uint16_t mask = 0;
    int ms = 0;
    std::string path;
};

static uint16_t name_to_mask(const std::string& n) {
    if (n == "A") return GB_KEY_A;
    if (n == "B") return GB_KEY_B;
    if (n == "C") return GB_KEY_C;
    if (n == "D") return GB_KEY_D;
    if (n == "UP") return GB_KEY_UP;
    if (n == "DOWN") return GB_KEY_DOWN;
    if (n == "LEFT") return GB_KEY_LEFT;
    if (n == "RIGHT") return GB_KEY_RIGHT;
    if (n == "L1") return GB_KEY_L1;
    if (n == "R1") return GB_KEY_R1;
    if (n == "MENU") return GB_KEY_MENU;
    if (n == "RUN") return GB_KEY_RUN;
    return 0;
}

static std::vector<ScriptStep> load_script(const char* path) {
    std::vector<ScriptStep> steps;
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        if (cmd == "PRESS") { std::string n; iss >> n; steps.push_back({ScriptStep::Press, name_to_mask(n), 0, ""}); }
        else if (cmd == "HOLD") { std::string n; int ms; iss >> n >> ms; steps.push_back({ScriptStep::Hold, name_to_mask(n), ms, ""}); }
        else if (cmd == "WAIT") { int ms; iss >> ms; steps.push_back({ScriptStep::Wait, 0, ms, ""}); }
        else if (cmd == "SCREENSHOT") { std::string p; iss >> p; steps.push_back({ScriptStep::Screenshot, 0, 0, p}); }
        else if (cmd == "QUIT") { steps.push_back({ScriptStep::Quit, 0, 0, ""}); }
    }
    return steps;
}

static void save_screenshot(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_PIXELFORMAT_BGR565);
    if (!surf) return;
    memcpy(surf->pixels, framebuffer, sizeof(gb_pixel) * SCREEN_WIDTH * SCREEN_HEIGHT);
    SDL_SaveBMP(surf, path.c_str());
    SDL_FreeSurface(surf);
    (void)renderer;
}

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init a échoué : %s\n", SDL_GetError());
        return 1;
    }

    const int SCALE = 2; // fenêtre 640x480 pour un pixel art 320x240 lisible
    SDL_Window* window = SDL_CreateWindow(
        "Wargame AKA - port PC/SDL2 (Z/X/C/V=A/B/C/D  Q/W=L1/R1  Entree=MENU)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow a échoué : %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Le framebuffer est déjà packé BGR565 (écran réel AKA, cf. CHANGES.md) ;
    // SDL_PIXELFORMAT_BGR565 lit exactement le même agencement de bits, donc
    // aucune conversion manuelle n'est nécessaire.
    SDL_Texture* texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);

    printf("\n=============================================\n");
    printf("  Wargame Medieval Fantastique - PORT PC/SDL2\n");
    printf("=============================================\n\n");

    g_core.init();
    wg::gfx_init();
    wg::input_init();
    wg::audio_init();
    wg::sprites_init();
    wg::language_load_from_nvs();
    wg::difficulty_load_from_nvs();

    wg::GameState gs;
    wg::game_init(gs);

    std::vector<ScriptStep> script;
    size_t script_idx = 0;
    int script_wait_remaining = 0;
    uint16_t script_hold_mask = 0;
    bool script_press_active = false;
    const char* script_path = getenv("WARGAME_SCRIPT");
    if (script_path) {
        script = load_script(script_path);
        printf("[PC] Mode script : %zu instruction(s) chargees depuis %s\n", script.size(), script_path);
    }

    bool running = true;
    Uint32 last_tick = SDL_GetTicks();

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                running = false;
        }

        if (!script.empty() && script_idx < script.size()) {
            // Une instruction "PRESS" dure exactement 1 frame (front montant
            // puis relachement), pour que key_pressed() la voie proprement.
            if (script_press_active) {
                script_press_active = false;
                g_pc_key_state = 0;
            } else if (script_wait_remaining > 0) {
                script_wait_remaining -= 25;
                g_pc_key_state = script_hold_mask; // 0 pour WAIT, mask pour HOLD
            } else {
                script_hold_mask = 0;
                while (script_idx < script.size()) {
                    ScriptStep& st = script[script_idx++];
                    if (st.kind == ScriptStep::Press) {
                        script_press_active = true;
                        g_pc_key_state = st.mask;
                        break;
                    } else if (st.kind == ScriptStep::Wait) {
                        script_wait_remaining = st.ms;
                        script_hold_mask = 0;
                        g_pc_key_state = 0;
                        break;
                    } else if (st.kind == ScriptStep::Hold) {
                        script_wait_remaining = st.ms;
                        script_hold_mask = st.mask;
                        g_pc_key_state = st.mask;
                        break;
                    } else if (st.kind == ScriptStep::Screenshot) {
                        save_screenshot(renderer, st.path);
                        printf("[PC] Capture ecrite : %s\n", st.path.c_str());
                    } else if (st.kind == ScriptStep::Quit) {
                        running = false;
                        break;
                    }
                }
            }
        } else {
            const uint8_t* keys = SDL_GetKeyboardState(nullptr);
            g_pc_key_state = sdl_scancode_to_gb_mask(keys);
        }

        wg::input_poll();
        wg::game_update(gs, 0.025f);
        wg::game_draw(gs);

        SDL_UpdateTexture(texture, nullptr, framebuffer, SCREEN_WIDTH * sizeof(gb_pixel));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // ~40 FPS, comme task_game.cpp (pdMS_TO_TICKS(25))
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - last_tick;
        if (elapsed < 25) SDL_Delay(25 - elapsed);
        last_tick = SDL_GetTicks();
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

