#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include "game.h"

// Formation types
#define FORMATION_SINGLE_PATROL   0  // Single enemy patrol left/right
#define FORMATION_GROUP_PATROL    1  // 2-4 enemies patrol together
#define FORMATION_ARROW           2  // Arrow/V formation
#define FORMATION_GALAGA_DIVE     3  // Galaga-style diving pattern
#define NUM_FORMATION_TYPES       4

// Formation constants
#define FORMATION_SPACING         20 // Spacing between enemies in formation

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
