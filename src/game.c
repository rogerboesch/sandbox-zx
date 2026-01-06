#include <arch/zxn.h>
#include <z80.h>
#include <input.h>
#include <stdint.h>
#include <stdlib.h>
#include "game.h"
#include "sprites.h"
#include "layer2.h"
#include "tilemap.h"
#include "ula.h"

// Global game objects
Player player;
Entity bullets[MAX_BULLETS];
Entity enemies[MAX_ENEMIES];
GameData game;

// Random seed
static uint16_t rand_seed = 0x1234;

// Hole collision cooldown (prevents continuous point loss)
static uint8_t hole_cooldown = 0;

// Simple random number generator
static uint8_t fast_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (uint8_t)(rand_seed >> 8);
}

// Read keyboard/joystick input
uint8_t input_read(void) {
    uint8_t result = 0;
    uint8_t keys;

    // Read keyboard - using direct port reading
    // Q = up, A = down, O = left, P = right, Space = fire

    // Row Q-T (0xFBFE)
    keys = z80_inp(0xFBFE);
    if (!(keys & 0x01)) result |= INPUT_UP;     // Q

    // Row A-G (0xFDFE)
    keys = z80_inp(0xFDFE);
    if (!(keys & 0x01)) result |= INPUT_DOWN;   // A

    // Row Y-P (0xDFFE)
    keys = z80_inp(0xDFFE);
    if (!(keys & 0x02)) result |= INPUT_LEFT;   // O
    if (!(keys & 0x01)) result |= INPUT_RIGHT;  // P

    // Row Space-V (0x7FFE)
    keys = z80_inp(0x7FFE);
    if (!(keys & 0x01)) result |= INPUT_FIRE;   // Space

    // Row H-Enter (0xBFFE) - H for pause
    keys = z80_inp(0xBFFE);
    if (!(keys & 0x10)) result |= INPUT_PAUSE;  // H

    // Also support cursor keys via Kempston joystick port (0x1F)
    keys = z80_inp(0x1F);
    if (keys & 0x08) result |= INPUT_UP;
    if (keys & 0x04) result |= INPUT_DOWN;
    if (keys & 0x02) result |= INPUT_LEFT;
    if (keys & 0x01) result |= INPUT_RIGHT;
    if (keys & 0x10) result |= INPUT_FIRE;

    return result;
}

// Initialize game state
void game_init(void) {
    uint8_t i;

    // Initialize player
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    player.lives = PLAYER_MAX_LIVES;
    player.shield = 0;
    player.fire_cooldown = 0;
    player.invincible = 0;

    // Clear bullets
    for (i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }

    // Clear enemies
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }

    // Initialize game data
    game.state = STATE_PLAYING;
    game.score = 0;
    game.level = 1;
    game.wave = 0;
    game.enemies_killed = 0;
    game.frame_count = 0;
    game.shake_timer = 0;
    game.crash_timer = 0;
    game.crash_type = CRASH_NONE;
    game.survival_timer = 0;

    // Reset scroll positions
    scroll_y = 0;
    layer2_scroll(0);
    layer2_scroll_x(0);
    tilemap_scroll(0);

    // Reset hole collision cooldown
    hole_cooldown = 0;
}

// Fire a bullet from player position (fires upward)
void game_fire_bullet(void) {
    uint8_t i;

    if (player.fire_cooldown > 0) return;

    // Find inactive bullet slot
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = 1;
            // Bullet sprite is 16x16, center it on player center
            bullets[i].x = player.x + (PLAYER_WIDTH / 2) - 8;
            bullets[i].y = player.y - 16;
            bullets[i].dx = 0;
            bullets[i].dy = -BULLET_SPEED;  // Move upward
            player.fire_cooldown = 8;  // Cooldown frames
            break;
        }
    }
}

// Spawn a new enemy (from top of screen)
void game_spawn_enemy(void) {
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
            if (type < 3 || game.level < 2) {
                enemies[i].type = 0;  // Basic enemy
                enemies[i].health = 1;
                enemies[i].dy = ENEMY_SPEED;  // Move downward
            } else {
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

// Update bullet positions (moving upward)
void game_update_bullets(void) {
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

// Update enemy positions (moving downward)
void game_update_enemies(void) {
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

// Simple AABB collision detection
static uint8_t check_collision(int16_t x1, int16_t y1, uint8_t w1, uint8_t h1,
                               int16_t x2, int16_t y2, uint8_t w2, uint8_t h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2);
}

// Tilemap constants for reading tile data
#define TILEMAP_ADDR    0x6000
#define TILEMAP_WIDTH   40
#define TILE_TRANS      0x07

// Check if player is over a hole in the highway
// Returns 1 if player center is over a hole, 0 otherwise
static uint8_t check_hole_collision(void) {
    int16_t player_center_x = player.x + (PLAYER_WIDTH / 2);
    int16_t player_center_y = player.y + (PLAYER_HEIGHT / 2);
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

    // Check if it's a transparent/hole tile
    return (tile == TILE_TRANS) ? 1 : 0;
}

// Check all collisions
void game_check_collisions(void) {
    uint8_t i, j;

    // Bullet vs Enemy collisions
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        for (j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;

            if (check_collision(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT,
                               enemies[j].x, enemies[j].y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
                bullets[i].active = 0;
                enemies[j].health--;

                if (enemies[j].health <= 0) {
                    enemies[j].active = 0;
                    game.enemies_killed++;
                    // Score based on enemy type
                    game.score += (enemies[j].type == 0) ? SCORE_ENEMY_NORMAL : SCORE_ENEMY_FAST;
                }
                break;
            }
        }
    }

    // Player vs Enemy collisions
    if (player.invincible == 0) {
        for (i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;

            if (check_collision(player.x, player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
                               enemies[i].x, enemies[i].y, ENEMY_WIDTH, ENEMY_HEIGHT)) {
                // Set crash type based on enemy type (yellow=normal, red=fast)
                game.crash_type = (enemies[i].type == 0) ? CRASH_ENEMY : CRASH_ENEMY_FAST;
                enemies[i].active = 0;
                player.lives--;
                player.invincible = 120;  // 2 seconds of invincibility

                // Trigger screen shake and CRASH text
                game.shake_timer = SHAKE_DURATION;
                game.crash_timer = CRASH_TEXT_DURATION;

                if (player.lives == 0) {
                    game.state = STATE_GAMEOVER;
                }
                break;
            }
        }
    }
}

// Main game update function
void game_update(void) {
    uint8_t input;

    if (game.state != STATE_PLAYING) return;

    // Read input
    input = input_read();

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
    if (input & INPUT_FIRE) {
        game_fire_bullet();
    }

    // Update cooldowns
    if (player.fire_cooldown > 0) player.fire_cooldown--;
    if (player.invincible > 0) player.invincible--;

    // Check if player left the highway (only if not invincible)
    // Crash only when more than half the player is outside
    if (player.invincible == 0) {
        int16_t player_center = player.x + (PLAYER_WIDTH / 2);
        if (player_center < HIGHWAY_LEFT || player_center > HIGHWAY_RIGHT) {
            // Player crashed off highway
            player.lives--;
            player.x = PLAYER_START_X;  // Reset to center
            player.invincible = 120;    // 2 seconds invincibility
            game.shake_timer = SHAKE_DURATION;
            game.crash_timer = CRASH_TEXT_DURATION;
            game.crash_type = CRASH_HIGHWAY;

            if (player.lives == 0) {
                game.state = STATE_GAMEOVER;
                return;
            }
        }
    }

    // Check for hole collision - reduce score, shake screen, blue flash (no invincibility)
    if (hole_cooldown > 0) {
        hole_cooldown--;
    } else if (check_hole_collision()) {
        if (game.score >= 200) {
            game.score -= 200;
        } else {
            game.score = 0;
        }
        game.shake_timer = SHAKE_DURATION;
        game.crash_timer = CRASH_TEXT_DURATION;
        game.crash_type = CRASH_HOLE;
        hole_cooldown = 30;
    }

    // Update scrolling (vertical scroll - decrement to scroll downward)
    scroll_y -= SCROLL_SPEED;

    // Tilemap (highway) scrolls at full speed
    tilemap_scroll(scroll_y);

    // Layer 2 (background) scrolls at half speed for parallax
    layer2_scroll(scroll_y / 2);

    // Horizontal parallax: grid scrolls opposite to player movement
    // Player moves right -> grid scrolls left, player moves left -> grid scrolls right
    {
        int16_t player_offset = (player.x - 120) / 4;  // Divide by 4 for subtle effect
        layer2_scroll_x(player_offset);  // Positive offset = grid scrolls left
    }

    // Update game objects
    game_update_bullets();
    game_update_enemies();
    game_check_collisions();

    // Spawn enemies periodically
    game.frame_count++;
    if (game.frame_count % 60 == 0) {
        game_spawn_enemy();
    }

    // Level progression
    if (game.enemies_killed >= 10 * game.level) {
        game.level++;
        game.enemies_killed = 0;
    }

    // Survival bonus (every 50 frames = ~1 second at 50Hz)
    game.survival_timer++;
    if (game.survival_timer >= 10) {
        game.survival_timer = 0;
        game.score += 1;
    }

    // Update effect timers
    if (game.shake_timer > 0) game.shake_timer--;
    if (game.crash_timer > 0) game.crash_timer--;
}

// Get screen shake offset (for visual effect)
int8_t game_get_shake_offset(void) {
    if (game.shake_timer == 0) return 0;
    // Alternating offset based on timer
    return (game.shake_timer & 0x02) ? 2 : -2;
}

// Render game
// Render HUD text on ULA
static void render_hud_text(void) {
    ula_print_at(0, 0, "SCORE", ATTR_WHITE_ON_BLACK);
    ula_print_num(6, 0, game.score, ATTR_YELLOW_ON_BLACK);
    ula_print_at(25, 0, "LIVES", ATTR_WHITE_ON_BLACK);
    ula_print_num(31, 0, player.lives, ATTR_YELLOW_ON_BLACK);
}

void game_render(void) {
    uint8_t i;
    uint8_t sprite_slot = 0;

    // Render HUD text overlay
    render_hud_text();

    // Highway is rendered by tilemap hardware (scrolled via tilemap_scroll)

    // Render player shadow first (behind player)
    if (player.invincible == 0 || (player.invincible & 0x04)) {
        sprite_set(sprite_slot++, player.x + SHADOW_OFFSET_X, player.y + SHADOW_OFFSET_Y, SPRITE_SHADOW);
        sprite_set(sprite_slot++, player.x, player.y, SPRITE_PLAYER);
    } else {
        sprite_hide(sprite_slot++);
        sprite_hide(sprite_slot++);
    }

    // Render bullets
    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            sprite_set(sprite_slot++, bullets[i].x, bullets[i].y, SPRITE_BULLET);
        }
    }

    // Render enemy shadows first (behind enemies)
    // Shadow offset is 3x larger when not over highway
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            int16_t enemy_center = enemies[i].x + (ENEMY_WIDTH / 2);
            uint8_t shadow_mult = (enemy_center >= HIGHWAY_LEFT && enemy_center <= HIGHWAY_RIGHT) ? 1 : 3;
            sprite_set(sprite_slot++, enemies[i].x + SHADOW_OFFSET_X * shadow_mult,
                       enemies[i].y + SHADOW_OFFSET_Y * shadow_mult, SPRITE_ENEMY_SHADOW);
        }
    }

    // Render enemies
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            uint8_t pattern = (enemies[i].type == 0) ? SPRITE_ENEMY1 : SPRITE_ENEMY2;
            sprite_set(sprite_slot++, enemies[i].x, enemies[i].y, pattern);
        }
    }

    // Hide unused sprite slots
    while (sprite_slot < 32) {
        sprite_hide(sprite_slot++);
    }
}
