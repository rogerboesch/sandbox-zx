#include <stdint.h>
#include "player.h"
#include "sprites.h"

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

// Update player based on input
// Returns 1 if fire button pressed and cooldown allows
uint8_t player_update(uint8_t input) {
    uint8_t fire = 0;

    // Update player position
    if (input & INPUT_UP) {
        if (player.y > GAME_TOP) {
            player.y -= PLAYER_SPEED;
        }
    }
    if (input & INPUT_DOWN) {
        if (player.y < GAME_BOTTOM - PLAYER_HEIGHT) {
            player.y += PLAYER_SPEED;
        }
    }
    if (input & INPUT_LEFT) {
        if (player.x > GAME_LEFT) {
            player.x -= PLAYER_SPEED;
        }
    }
    if (input & INPUT_RIGHT) {
        if (player.x < GAME_RIGHT - PLAYER_WIDTH) {
            player.x += PLAYER_SPEED;
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

// Check if player is outside level boundaries
// Returns crash type (CRASH_LEVEL) or CRASH_NONE
uint8_t player_check_level(void) {
    int16_t player_center;

    // Only check if not invincible
    if (player.invincible != 0) {
        return CRASH_NONE;
    }

    player_center = player.x + (PLAYER_WIDTH / 2);

    if (player_center < LEVEL_LEFT || player_center > LEVEL_RIGHT) {
        return CRASH_LEVEL;
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
    player.x = PLAYER_START_X;
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
