#ifndef COLLISION_H
#define COLLISION_H

#include <stdint.h>
#include "game.h"

// Collision result structure
typedef struct {
    uint8_t enemies_killed;
    uint16_t score_gained;
    uint8_t player_hit;
    uint8_t crash_type;
} CollisionResult;

// Check bullet vs enemy collisions
// Deactivates bullets and damages enemies
// Returns number of enemies killed and score gained
CollisionResult collision_bullets_enemies(void);

// Check player vs enemy collisions
// Returns crash type if collision occurred, CRASH_NONE otherwise
uint8_t collision_player_enemies(void);

// Check if player is over a hole
// Returns 1 if player center is over a hole, 0 otherwise
uint8_t collision_check_hole(int16_t player_x, int16_t player_y, int16_t scroll_y);

#endif // COLLISION_H
