#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "game.h"

// Initialize player to starting position
void player_init(void);

// Update player based on input
// Returns 1 if fire button pressed and cooldown allows
uint8_t player_update(uint8_t input);

// Update player cooldowns (call every frame)
void player_update_cooldowns(void);

// Check if player is outside level boundaries
// Returns crash type (CRASH_LEVEL) or CRASH_NONE
uint8_t player_check_level(void);

// Apply damage to player
// Returns 1 if player died
uint8_t player_hit(void);

// Reset player to center after crash
void player_reset_position(void);

// Render player and shadow
// Returns next available sprite slot
uint8_t player_render(uint8_t sprite_slot);

// Hide player sprites
// Returns next available sprite slot
uint8_t player_hide(uint8_t sprite_slot);

// Global player
extern Player player;

#endif // PLAYER_H
