#include "sprites.h"
#include "filesystem.h"
#include <cstdio>

namespace wg {

static uint16_t s_terrain[(int)Terrain::COUNT][SPRITE_PIXELS];
static bool     s_terrain_loaded[(int)Terrain::COUNT];
static uint16_t s_unit[(int)UnitType::COUNT][SPRITE_PIXELS];
static bool     s_unit_loaded[(int)UnitType::COUNT];

// Noms longs (LFN activé dans sdkconfig, cf. CONFIG_FATFS_LFN_HEAP) :
// un fichier par terrain / unité, dans le repertoire propre au jeu sur la
// carte SD (SD_GAME_DIR, cf. core/filesystem.h) — pas a la racine de la SD,
// puisque chaque jeu AKA a son propre repertoire (structure multi-cartouches
// du loader : firmware.bin, meta.json, Picture.png, screen.bmp, sprites/...).
static const char* kTerrainFile[(int)Terrain::COUNT] = {
    SD_GAME_DIR "/sprites/terrain_plain.bin",
    SD_GAME_DIR "/sprites/terrain_forest.bin",
    SD_GAME_DIR "/sprites/terrain_hill.bin",
    SD_GAME_DIR "/sprites/terrain_water.bin",
    SD_GAME_DIR "/sprites/terrain_road.bin",
    SD_GAME_DIR "/sprites/terrain_castle.bin",
    SD_GAME_DIR "/sprites/terrain_bridge.bin",
    SD_GAME_DIR "/sprites/terrain_wall.bin",
};

static const char* kUnitFile[(int)UnitType::COUNT] = {
    SD_GAME_DIR "/sprites/unit_infantry.bin",
    SD_GAME_DIR "/sprites/unit_archer.bin",
    SD_GAME_DIR "/sprites/unit_cavalry.bin",
    SD_GAME_DIR "/sprites/unit_mage.bin",
    SD_GAME_DIR "/sprites/unit_siege_engine.bin",
    SD_GAME_DIR "/sprites/unit_hero.bin",
    SD_GAME_DIR "/sprites/unit_sapper.bin",
    SD_GAME_DIR "/sprites/unit_griffon.bin",
};

static bool load_raw16(const char* path, uint16_t* dst) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    size_t n = fread(dst, sizeof(uint16_t), SPRITE_PIXELS, f);
    fclose(f);
    return n == (size_t)SPRITE_PIXELS;
}

void sprites_init() {
    int nt = 0, nu = 0;
    for (int i = 0; i < (int)Terrain::COUNT; ++i) {
        s_terrain_loaded[i] = load_raw16(kTerrainFile[i], s_terrain[i]);
        if (s_terrain_loaded[i]) nt++;
    }
    for (int i = 0; i < (int)UnitType::COUNT; ++i) {
        s_unit_loaded[i] = load_raw16(kUnitFile[i], s_unit[i]);
        if (s_unit_loaded[i]) nu++;
    }
    printf("[Sprites] terrain %d/%d, unites %d/%d charges depuis %s\n",
           nt, (int)Terrain::COUNT, nu, (int)UnitType::COUNT, SD_GAME_DIR "/sprites/");
    if (nt < (int)Terrain::COUNT || nu < (int)UnitType::COUNT)
        printf("[Sprites] fichiers manquants -> repli sur le rendu couleur/glyphe pour ceux-la.\n");
}

const uint16_t* sprite_for_terrain(Terrain t) {
    int i = (int)t;
    if (i < 0 || i >= (int)Terrain::COUNT || !s_terrain_loaded[i]) return nullptr;
    return s_terrain[i];
}

const uint16_t* sprite_for_unit(UnitType t) {
    int i = (int)t;
    if (i < 0 || i >= (int)UnitType::COUNT || !s_unit_loaded[i]) return nullptr;
    return s_unit[i];
}

} // namespace wg
