#include "story_image.h"
#include "filesystem.h"
#include <cstdio>
#include <cstring>

namespace wg {

static uint16_t s_buf[STORY_IMAGE_W * STORY_IMAGE_H];
static char s_loaded_name[32] = "";
static bool s_loaded_ok = false;

const uint16_t* story_image_load(const char* name) {
    if (!name || !name[0]) return nullptr;
    if (s_loaded_ok && strcmp(s_loaded_name, name) == 0) return s_buf;

    char path[96];
    snprintf(path, sizeof(path), SD_GAME_DIR "/story/%s.bin", name);
    FILE* f = fopen(path, "rb");
    if (!f) { s_loaded_ok = false; return nullptr; }
    size_t n = fread(s_buf, sizeof(uint16_t), STORY_IMAGE_W * STORY_IMAGE_H, f);
    fclose(f);
    if (n != (size_t)(STORY_IMAGE_W * STORY_IMAGE_H)) { s_loaded_ok = false; return nullptr; }

    strncpy(s_loaded_name, name, sizeof(s_loaded_name) - 1);
    s_loaded_name[sizeof(s_loaded_name) - 1] = 0;
    s_loaded_ok = true;
    return s_buf;
}

} // namespace wg
