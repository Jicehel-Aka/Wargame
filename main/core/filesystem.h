#pragma once
#include <string>

namespace wg {

// Chaque jeu AKA vit dans son propre repertoire a la racine de la carte SD
// (structure multi-cartouches geree par le loader : firmware.bin, meta.json,
// Picture.png, screen.bmp, et les assets propres au jeu, tous dans ce meme
// repertoire). Toute lecture/ecriture sur la SD propre a Wargame AKA doit
// passer par ce prefixe -> ne jamais coder "/sdcard/..." en dur ailleurs.
#define SD_GAME_DIR "/sdcard/Wargame_Aka"

bool fs_exists(const char* path);
bool fs_mkdir(const char* path);
bool fs_write_text(const char* path, const char* text);
bool fs_read_text(const char* path, std::string& out);

} // namespace wg
