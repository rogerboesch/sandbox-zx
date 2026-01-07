#include <stdint.h>
#include "enemy.h"
#include "sprites.h"

// Global enemies array
Entity enemies[MAX_ENEMIES];

// Random seed
static uint16_t rand_seed = 0x1234;

// Simple random number generator
static uint8_t fast_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (uint8_t)(rand_seed >> 8);
}

// Initialize enemies (clear all)
void enemies_init(void) {
    uint8_t i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }
}

// Update enemy positions
void enemies_update(void) {
    uint8_t i;

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].x += enemies[i].dx;
            enemies[i].y += enemies[i].dy;

            // Bounce off left/right edges
            if (enemies[i].x < GAME_LEFT || enemies[i].x > GAME_RIGHT - ENEMY_WIDTH) {
                enemies[i].dx = -enemies[i].dx;
            }

            // Deactivate if off screen bottom
            if (enemies[i].y > SCREEN_HEIGHT) {
                enemies[i].active = 0;
            }

            // Animation frame
            enemies[i].frame++;
        }
    }
}

// Spawn a new enemy
void enemies_spawn(uint8_t level) {
    uint8_t i;
    uint8_t type;

    // Find inactive enemy slot
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = 1;
            enemies[i].x = (fast_rand() % (GAME_RIGHT - GAME_LEFT - ENEMY_WIDTH)) + GAME_LEFT;
            enemies[i].y = -ENEMY_HEIGHT;  // Start above screen

            // Enemy type based on level
            type = fast_rand() % 4;
            if (type < 3 || level < 2) {
                enemies[i].type = 0;  // Basic enemy
                enemies[i].health = 1;
                enemies[i].dy = ENEMY_SPEED;  // Move downward
            }
            else {
                enemies[i].type = 1;  // Tougher enemy
                enemies[i].health = 2;
                enemies[i].dy = ENEMY_SPEED + 1;  // Slightly faster
            }

            // Add some horizontal movement variation
            enemies[i].dx = (fast_rand() % 3) - 1;  // -1, 0, or 1
            enemies[i].frame = 0;
            break;
        }
    }
}

// Render enemy shadows
// Returns next available sprite slot
uint8_t enemies_render_shadows(uint8_t sprite_slot, uint8_t frame_count) {
    uint8_t i;
    (void)frame_count;  // Unused for shadows

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int16_t enemy_center = enemies[i].x + (ENEMY_WIDTH / 2);
            uint8_t shadow_mult = (enemy_center >= LEVEL_LEFT && enemy_center <= LEVEL_RIGHT) ? 1 : 2;
            sprite_set(sprite_slot++, enemies[i].x + SHADOW_OFFSET_X * shadow_mult,
                       enemies[i].y + SHADOW_OFFSET_Y * shadow_mult, SPRITE_ENEMY_SHADOW);
        }
    }

    return sprite_slot;
}

// Render enemies
// Returns next available sprite slot
uint8_t enemies_render(uint8_t sprite_slot, uint8_t frame_count) {
    uint8_t i;

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            // Animate through frames A0-G0, offset by enemy index for variety
            uint8_t frame = ((frame_count >> 3) + i) % ENEMY_ANIM_FRAMES;
            uint8_t pattern = SPRITE_ENEMY_BASE + frame;
            sprite_set(sprite_slot++, enemies[i].x, enemies[i].y, pattern);
        }
    }

    return sprite_slot;
}
