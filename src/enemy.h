#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include "game.h"

// Initialize enemies (clear all)
void enemies_init(void);

// Update enemy positions
void enemies_update(void);

// Spawn a new enemy
void enemies_spawn(uint8_t level);

// Render enemy shadows
// Returns next available sprite slot
uint8_t enemies_render_shadows(uint8_t sprite_slot, uint8_t frame_count);

// Render enemies
// Returns next available sprite slot
uint8_t enemies_render(uint8_t sprite_slot, uint8_t frame_count);

// Global enemies array
extern Entity enemies[MAX_ENEMIES];

#endif // ENEMY_H
