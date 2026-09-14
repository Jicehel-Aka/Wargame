#include "types.h"
#include "../language.h"

namespace wg {

// Couleurs RGB565 approximatives
// Le panneau ST7789V de l'AKA est configure en BGR (cf. meme remarque dans
// core/graphics.h) -> packing blue-high/red-low, pas le RGB565 standard.
static inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)((r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11));
}

const TerrainInfo& terrain_info(Terrain t) {
    static const TerrainInfo table[(int)Terrain::COUNT] = {
        /* Plain  */ { "Plaine",   "Plain",   1,   0, rgb(120, 180, 80)  },
        /* Forest */ { "Foret",    "Forest",  2,  20, rgb(40, 110, 50)   },
        /* Hill   */ { "Colline",  "Hill",    2,  30, rgb(150, 130, 90)  },
        /* Water  */ { "Eau",      "Water", 999,   0, rgb(40, 90, 200)   },
        /* Road   */ { "Route",    "Road",    1,   0, rgb(180, 160, 120) },
        /* Castle */ { "Chateau",  "Castle",  1,  50, rgb(140, 140, 150) },
        /* Bridge */ { "Pont",     "Bridge",  1, -20, rgb(150, 110, 70)  },
        /* Wall   */ { "Remparts", "Walls",   3,  60, rgb(100, 100, 115) },
    };
    return table[(int)t];
}

const char* terrain_name(Terrain t) {
    const TerrainInfo& ti = terrain_info(t);
    return (language_get() == Language::FR) ? ti.name_fr : ti.name_en;
}

const UnitStats& unit_stats(UnitType t) {
    static const UnitStats table[(int)UnitType::COUNT] = {
        /* Infantry    */ { "Infanterie", "Infantry",     'I', 10, 4, 3, 3, 1, 10 },
        /* Archer      */ { "Archers",    "Archers",      'A',  7, 4, 1, 3, 3, 12 },
        /* Cavalry     */ { "Cavalerie",  "Cavalry",      'C', 10, 5, 2, 5, 1, 16 },
        /* Mage        */ { "Mage",       "Mage",         'M',  6, 6, 1, 3, 2, 18 },
        /* SiegeEngine */ { "Catapulte",  "Siege Engine", 'S',  8, 8, 1, 2, 3, 20 },
        /* Hero        */ { "Heros",      "Hero",         'H', 16, 7, 4, 4, 1, 30 },
        /* Sapper      */ { "Sapeur",     "Sapper",       'P',  9, 5, 3, 2, 1, 14, false, true },
        /* Griffon     */ { "Griffon",    "Griffon",      'V',  7, 5, 2, 6, 1, 24, true,  false },
    };
    return table[(int)t];
}

const char* unit_name(UnitType t) {
    const UnitStats& st = unit_stats(t);
    return (language_get() == Language::FR) ? st.name_fr : st.name_en;
}

} // namespace wg
