#include <stdint.h>
#include "bullet.h"
#include "sprites.h"

// Global bullets array
Entity bullets[MAX_BULLETS];

// Initialize bullets (clear all)
void bullets_init(void) {
    uint8_t i;
    for (i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }
}

// Update bullet positions
void bullets_update(void) {
    uint8_t i;

    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].y += bullets[i].dy;

            // Deactivate if off screen (top)
            if (bullets[i].y < -BULLET_HEIGHT) {
                bullets[i].active = 0;
            }
        }
    }
}

// Spawn a bullet from player position
void bullets_spawn(int16_t player_x, int16_t player_y) {
    uint8_t i;

    // Find inactive bullet slot
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = 1;
            // Bullet sprite is 16x16, center it on player center
            bullets[i].x = player_x + (PLAYER_WIDTH / 2) - 8;
            bullets[i].y = player_y - 16;
            bullets[i].dx = 0;
            bullets[i].dy = -BULLET_SPEED;  // Move upward
            break;
        }
    }
}

// Render bullets using sprites
// Returns next available sprite slot
uint8_t bullets_render(uint8_t sprite_slot) {
    uint8_t i;

    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            sprite_set(sprite_slot++, bullets[i].x, bullets[i].y, SPRITE_BULLET);
        }
    }

    return sprite_slot;
}

// Hide all bullet sprites
// Returns next available sprite slot
uint8_t bullets_hide(uint8_t sprite_slot) {
    uint8_t i;

    for (i = 0; i < MAX_BULLETS; i++) {
        sprite_hide(sprite_slot++);
    }

    return sprite_slot;
}
