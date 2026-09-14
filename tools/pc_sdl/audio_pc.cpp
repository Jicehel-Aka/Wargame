// Le port PC/SDL2 ne fait pas de son (pas dans le périmètre de cette
// vérification) : toutes les fonctions audio sont des no-op.
#include "audio.h"
namespace wg {
void audio_init() {}
void audio_set_sfx_volume(int) {}
int  audio_get_sfx_volume() { return 0; }
void sfx_select() {}
void sfx_cancel() {}
void sfx_move() {}
void sfx_invalid() {}
void sfx_attack() {}
void sfx_hit() {}
void sfx_unit_death() {}
void sfx_end_turn() {}
void sfx_castle_capture() {}
void sfx_river_blocked() {}
void sfx_victory() {}
void sfx_defeat() {}
void sfx_menu_move() {}
void sfx_recruit() {}
} // namespace wg
