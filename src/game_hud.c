#include "game_hud.h"
#include "ula.h"
#include "game.h"
#include "player.h"

// Initialize game HUD
void game_hud_init(void) {
    // Nothing to init for now, but keeps API consistent
}

// Render game HUD (call every frame when playing)
void game_hud_render(void) {
    ula_print_at(0, 0, "SCORE", ATTR_WHITE_ON_BLACK);
    ula_print_num(6, 0, game.score, ATTR_YELLOW_ON_BLACK);
    ula_print_at(25, 0, "LIVES", ATTR_WHITE_ON_BLACK);
    ula_print_num(31, 0, player.lives, ATTR_YELLOW_ON_BLACK);
}
