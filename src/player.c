#include <stdint.h>
#include "player.h"
#include "sprites.h"
#include "tilemap.h"
#include "level.h"

// Global player
Player player;

// Initialize player to starting position
void player_init(void) {
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.lives = PLAYER_MAX_LIVES;
    player.shield = 0;
    player.fire_cooldown = 0;
    player.invincible = 0;
}

// Check if a position would be on a valid tile
static uint8_t is_valid_position(int16_t x, int16_t y) {
    int16_t center_x = x + (PLAYER_WIDTH / 2);
    int16_t center_y = y + (PLAYER_HEIGHT / 2);
    uint8_t tile = tilemap_get_tile_at(center_x, center_y);
    return (tile != TILE_TRANS);
}

// Update player based on input
// Returns 1 if fire button pressed and cooldown allows
uint8_t player_update(uint8_t input) {
    uint8_t fire = 0;
    int16_t new_x, new_y;

    // Update player position - check tile before allowing move
    if (input & INPUT_UP) {
        if (player.y > GAME_TOP) {
            new_y = player.y - PLAYER_SPEED;
            if (is_valid_position(player.x, new_y)) {
                player.y = new_y;
            }
        }
    }
    if (input & INPUT_DOWN) {
        if (player.y < GAME_BOTTOM - PLAYER_HEIGHT) {
            new_y = player.y + PLAYER_SPEED;
            if (is_valid_position(player.x, new_y)) {
                player.y = new_y;
            }
        }
    }
    if (input & INPUT_LEFT) {
        if (player.x > GAME_LEFT) {
            new_x = player.x - PLAYER_SPEED;
            if (is_valid_position(new_x, player.y)) {
                player.x = new_x;
            }
        }
    }
    if (input & INPUT_RIGHT) {
        if (player.x < GAME_RIGHT - PLAYER_WIDTH) {
            new_x = player.x + PLAYER_SPEED;
            if (is_valid_position(new_x, player.y)) {
                player.x = new_x;
            }
        }
    }

    // Check fire
    if ((input & INPUT_FIRE) && player.fire_cooldown == 0) {
        fire = 1;
        player.fire_cooldown = 8;  // Cooldown frames
    }

    return fire;
}

// Update player cooldowns (call every frame)
void player_update_cooldowns(void) {
    if (player.fire_cooldown > 0) player.fire_cooldown--;
    if (player.invincible > 0) player.invincible--;
}

// Hole tile range
#define TILE_HOLE_TL 0x07
#define TILE_HOLE_BR 0x0A

// Check if player is outside level boundaries or in a hole
// Returns crash type (CRASH_LEVEL, CRASH_HOLE) or CRASH_NONE
uint8_t player_check_level(void) {
    int16_t player_center_x;
    int16_t player_center_y;
    uint8_t tile;

    // Only check if not invincible
    if (player.invincible != 0) {
        return CRASH_NONE;
    }

    // Check tile under player center
    player_center_x = player.x + (PLAYER_WIDTH / 2);
    player_center_y = player.y + (PLAYER_HEIGHT / 2);

    // Get tile at player center position
    tile = tilemap_get_tile_at(player_center_x, player_center_y);

    // If tile is transparent, player is off the road
    if (tile == TILE_TRANS) {
        return CRASH_LEVEL;
    }

    // If tile is a hole (tiles 0x07-0x0A), player fell in
    if (tile >= TILE_HOLE_TL && tile <= TILE_HOLE_BR) {
        return CRASH_HOLE;
    }

    return CRASH_NONE;
}

// Apply damage to player
// Returns 1 if player died
uint8_t player_hit(void) {
    player.lives--;
    player.invincible = 120;  // 2 seconds of invincibility

    return (player.lives == 0) ? 1 : 0;
}

// Reset player to center after crash
void player_reset_position(void) {
    int16_t left, right;

    // Reset to center of current lane(s)
    level_get_boundaries(&left, &right);
    player.x = (left + right) / 2 - (PLAYER_WIDTH / 2);
}

// Render player and shadow
// Returns next available sprite slot
uint8_t player_render(uint8_t sprite_slot) {
    // Render player shadow first (behind player)
    if (player.invincible == 0 || (player.invincible & 0x04)) {
        sprite_set(sprite_slot++, player.x + SHADOW_OFFSET_X, player.y + SHADOW_OFFSET_Y, SPRITE_SHADOW);
        sprite_set(sprite_slot++, player.x, player.y, SPRITE_PLAYER);
    }
    else {
        sprite_hide(sprite_slot++);
        sprite_hide(sprite_slot++);
    }

    return sprite_slot;
}

// Hide player sprites
// Returns next available sprite slot
uint8_t player_hide(uint8_t sprite_slot) {
    sprite_hide(sprite_slot++);
    sprite_hide(sprite_slot++);
    return sprite_slot;
}
