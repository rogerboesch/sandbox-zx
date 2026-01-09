#include "debug_hud.h"
#include "ula.h"
#include "player.h"
#include "tilemap.h"

// Debug HUD state
static uint8_t debug_enabled = 0;
static uint8_t debug_visible = 0;  // Track if currently drawn (for clearing)

// Initialize debug HUD (does not reset enabled state)
void debug_hud_init(void) {
    // Don't reset debug_enabled - keep user preference across game restarts
    debug_visible = 0;
}

// Toggle debug display on/off
void debug_hud_toggle(void) {
    debug_enabled = !debug_enabled;

    // Clear when turning off
    if (!debug_enabled && debug_visible) {
        debug_hud_clear();
    }
}

// Check if debug is enabled
uint8_t debug_hud_is_enabled(void) {
    return debug_enabled;
}

// Clear debug area (transparent)
void debug_hud_clear(void) {
    ula_print_at(0, 15, "          ", 0x00);
    ula_print_at(0, 16, "          ", 0x00);
    ula_print_at(0, 17, "          ", 0x00);
    ula_print_at(0, 18, "          ", 0x00);
    debug_visible = 0;
}

// Render debug HUD (call every frame when playing)
void debug_hud_render(void) {
    int16_t player_center_x;
    int16_t player_center_y;
    uint8_t tile;
    uint8_t collision;

    if (!debug_enabled) return;

    // Get player center position (same as collision check)
    player_center_x = player.x + (PLAYER_WIDTH / 2);
    player_center_y = player.y + (PLAYER_HEIGHT / 2);

    // Get tile under player
    tile = tilemap_get_tile_at(player_center_x, player_center_y);

    // Collision = on transparent tile
    collision = (tile == TILE_TRANS) ? 1 : 0;

    ula_print_at(0, 15, "X:        ", ATTR_WHITE_ON_RED);
    ula_print_num(2, 15, player.x, ATTR_YELLOW_ON_RED);
    ula_print_at(0, 16, "Y:        ", ATTR_WHITE_ON_RED);
    ula_print_num(2, 16, player.y, ATTR_YELLOW_ON_RED);
    ula_print_at(0, 17, "TILE:     ", ATTR_WHITE_ON_RED);
    ula_print_num(5, 17, tile, ATTR_YELLOW_ON_RED);
    ula_print_at(0, 18, "COL:      ", ATTR_WHITE_ON_RED);
    ula_print_num(4, 18, collision, ATTR_YELLOW_ON_RED);

    debug_visible = 1;
}
