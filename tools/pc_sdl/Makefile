# =============================================================================
#  Makefile — Port PC/SDL2 de Wargame AKA
# -----------------------------------------------------------------------------
#  Usage :
#    make          -> compile bin/wargame_pc
#    make run      -> compile puis lance
#    make clean
#
#  Nécessite SDL2 (dev headers) :
#    Debian/Ubuntu : sudo apt install libsdl2-dev
#    macOS (brew)  : brew install sdl2
#    Windows       : MSYS2/MinGW, package mingw-w64-x86_64-SDL2
# =============================================================================
CXX      ?= g++
CXXFLAGS ?= -std=gnu++17 -O1 -Wall -funsigned-char
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null || echo -I/usr/include/SDL2 -D_REENTRANT)
SDL_LIBS   := $(shell sdl2-config --libs   2>/dev/null || echo -lSDL2)

ROOT      := ../..
MAIN      := $(ROOT)/main
GAMEBUINO := $(ROOT)/components/gamebuino

INCLUDES := -I. -I$(MAIN) -I$(MAIN)/core -I$(MAIN)/game \
            -I$(GAMEBUINO)/include_lib -I$(GAMEBUINO)/include_ll \
            $(SDL_CFLAGS)

# Fichiers du jeu réel (non modifiés) + couche PC de ce dossier
GAME_SRCS := \
    $(MAIN)/game/hex.cpp \
    $(MAIN)/game/units_db.cpp \
    $(MAIN)/game/map.cpp \
    $(MAIN)/game/combat.cpp \
    $(MAIN)/game/ai.cpp \
    $(MAIN)/game/levels_data.cpp \
    $(MAIN)/game/campaign.cpp \
    $(MAIN)/game/game.cpp \
    $(MAIN)/language.cpp \
    $(MAIN)/difficulty.cpp \
    $(MAIN)/core/graphics.cpp \
    $(MAIN)/core/filesystem.cpp \
    $(MAIN)/core/sprites.cpp \
    $(MAIN)/core/story_image.cpp \
    $(MAIN)/core/input.cpp

PC_SRCS := \
    main.cpp \
    gb_core_pc.cpp \
    gb_lcd_pc.cpp \
    audio_pc.cpp \
    nvs_pc.cpp

SRCS := $(GAME_SRCS) $(PC_SRCS)
OBJS := $(patsubst %.cpp,obj/%.o,$(notdir $(SRCS)))

BIN := bin/wargame_pc

.PHONY: all run clean
all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(SDL_LIBS)
	@echo "-> $(BIN)"

# Règle générique : cherche chaque .cpp dans son dossier d'origine (VPATH)
VPATH := $(sort $(dir $(SRCS)))
obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf obj bin
