# =============================================================================
#  Makefile — Simulateur de campagne (headless, sans SDL2)
# -----------------------------------------------------------------------------
#  make        -> compile bin/campaign_sim
#  make run    -> compile puis lance (imprime le rapport des 30 cartes x 3
#                 difficultes sur la sortie standard)
# =============================================================================
CXX      ?= g++
CXXFLAGS ?= -std=gnu++17 -O2 -Wall -funsigned-char

ROOT      := ../..
MAIN      := $(ROOT)/main
GAMEBUINO := $(ROOT)/components/gamebuino

INCLUDES := -I. -I$(MAIN) -I$(MAIN)/core -I$(MAIN)/game \
            -I$(GAMEBUINO)/include_lib -I$(GAMEBUINO)/include_ll

GAME_SRCS := \
    $(MAIN)/game/hex.cpp \
    $(MAIN)/game/units_db.cpp \
    $(MAIN)/game/map.cpp \
    $(MAIN)/game/combat.cpp \
    $(MAIN)/game/ai.cpp \
    $(MAIN)/game/levels_data.cpp \
    $(MAIN)/game/campaign.cpp \
    $(MAIN)/language.cpp \
    $(MAIN)/difficulty.cpp \
    $(MAIN)/core/filesystem.cpp

SIM_SRCS := campaign_sim.cpp audio_stub.cpp nvs_pc.cpp

SRCS := $(GAME_SRCS) $(SIM_SRCS)
OBJS := $(patsubst %.cpp,obj/%.o,$(notdir $(SRCS)))

BIN := bin/campaign_sim

.PHONY: all run clean
all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "-> $(BIN)"

VPATH := $(sort $(dir $(SRCS)))
obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf obj bin
