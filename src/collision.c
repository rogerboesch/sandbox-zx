#include <stdint.h>
#include "collision.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "level.h"

// Tilemap constants for reading tile data
#define TILEMAP_ADDR    0x6000
#define TILEMAP_WIDTH   40
#define TILE_HOLE_TL    0x04
#define TILE_HOLE_TR    0x05
#define TILE_HOLE_BL    0x06
#define TILE_HOLE_BR    0x07

// Simple AABB collision detection
static uint8_t check_aabb(int16_t x1, int16_t y1, uint8_t w1, uint8_t h1,
                          int16_t x2, int16_t y2, uint8_t w2, uint8_t h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2);
}

// Check bullet vs enemy collisions
CollisionResult collision_bullets_enemies(void) {
    CollisionResult result = {0, 0, 0, CRASH_NONE};
    uint8_t i, j;

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        for (j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;

            if (check_aabb(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                           enemies[j].x, enemies[j].y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
                bullets[i].active = 0;
                enemies[j].health--;

                if (enemies[j].health <= 0) {
                    enemies[j].active = 0;
                    result.enemies_killed++;
                    // Score based on enemy type
                    result.score_gained += (enemies[j].type == 0) ? SCORE_ENEMY_NORMAL : SCORE_ENEMY_FAST;
                }
                break;
            }
        }
    }

    return result;
}

// Check player vs enemy collisions
uint8_t collision_player_enemies(void) {
    uint8_t i;

    // Skip if player is invincible
    if (player.invincible != 0) {
        return CRASH_NONE;
    }

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        if (check_aabb(player.x, player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
                       enemies[i].x, enemies[i].y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
            // Return crash type based on enemy type (yellow=normal, red=fast)
            uint8_t crash = (enemies[i].type == 0) ? CRASH_ENEMY : CRASH_ENEMY_FAST;
            enemies[i].active = 0;
            return crash;
        }
    }

    return CRASH_NONE;
}

// Check if player is over a hole
uint8_t collision_check_hole(int16_t player_x, int16_t player_y, int16_t scroll_y) {
    int16_t player_center_x = player_x + (PLAYER_WIDTH / 2);
    int16_t player_center_y = player_y + (PLAYER_HEIGHT / 2);
    uint8_t tile_x, tile_y, tile;
    uint8_t *tilemap = (uint8_t *)TILEMAP_ADDR;
    uint8_t scroll_val;
    int16_t tilemap_y_px;

    // Sprites have 32-pixel offset from tilemap origin on ZX Next
    // Convert sprite coords to tilemap coords
    int16_t tm_x = player_center_x + 32;
    int16_t tm_y = player_center_y + 32;

    // Convert to tilemap tile X (tilemap is 40 tiles wide, screen shows middle 32)
    tile_x = tm_x / 8;
    if (tile_x >= TILEMAP_WIDTH) return 0;

    // Convert to tilemap tile Y (accounting for scroll)
    scroll_val = (uint8_t)(scroll_y & 0xFF);
    tilemap_y_px = tm_y + scroll_val;
    tile_y = (tilemap_y_px / 8) & 0x1F;  // mod 32

    // Read tile from tilemap
    tile = tilemap[tile_y * TILEMAP_WIDTH + tile_x];

    // Check if it's a hole tile (indices 4-7)
    return (tile >= TILE_HOLE_TL && tile <= TILE_HOLE_BR) ? 1 : 0;
}
