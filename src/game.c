#include <arch/zxn.h>
#include <z80.h>
#include <input.h>
#include <stdint.h>
#include <stdlib.h>
#include "game.h"
#include "player.h"
#include "bullet.h"
#include "enemy.h"
#include "collision.h"
#include "sprites.h"
#include "layer2.h"
#include "tilemap.h"
#include "ula.h"
#include "sound.h"
#include "level.h"
#include "level1.h"

// Global game data
GameData game;

// Hole collision cooldown (prevents continuous point loss)
static uint8_t hole_cooldown = 0;

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
    // Initialize level system
    level_init(&level1_def);

    // Refresh tilemap with level data
    tilemap_refresh();

    // Initialize player
    player_init();

    // Clear bullets
    bullets_init();

    // Clear enemies
    enemies_init();

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

// Main game update function
void game_update(void) {
    uint8_t input;
    uint8_t crash;
    CollisionResult coll_result;

    if (game.state != STATE_PLAYING) return;

    // Read input
    input = input_read();

    // Update player position and check for fire
    if (player_update(input)) {
        bullets_spawn(player.x, player.y);
        sound_fire();
    }

    // Update cooldowns
    player_update_cooldowns();

    // Check if player left the level
    crash = player_check_level();
    if (crash != CRASH_NONE) {
        if (player_hit()) {
            game.state = STATE_DYING;
            return;
        }
        player_reset_position();
        game.shake_timer = SHAKE_DURATION;
        game.crash_timer = CRASH_TEXT_DURATION;
        game.crash_type = crash;
    }

    // Check for hole collision - reduce score, shake screen, blue flash (no invincibility)
    if (hole_cooldown > 0) {
        hole_cooldown--;
    }
    else if (collision_check_hole(player.x, player.y, scroll_y)) {
        if (game.score >= 200) {
            game.score -= 200;
        }
        else {
            game.score = 0;
        }
        game.shake_timer = SHAKE_DURATION;
        game.crash_timer = CRASH_TEXT_DURATION;
        game.crash_type = CRASH_HOLE;
        hole_cooldown = 30;
        sound_hole();
    }

    // Update scrolling (vertical scroll - decrement to scroll downward)
    scroll_y -= SCROLL_SPEED;

    // Update level system (advances segments based on scroll)
    level_update(scroll_y);

    // Tilemap scrolls at full speed
    tilemap_scroll(scroll_y);

    // Layer 2 (background) scrolls at half speed for parallax
    layer2_scroll(scroll_y / 2);

    // Horizontal parallax: grid scrolls opposite to player movement
    {
        int16_t player_offset = (player.x - 120) / 4;
        layer2_scroll_x(player_offset);
    }

    // Update game objects
    bullets_update();
    enemies_update();

    // Check collisions
    coll_result = collision_bullets_enemies();
    if (coll_result.enemies_killed > 0) {
        game.enemies_killed += coll_result.enemies_killed;
        game.score += coll_result.score_gained;
        sound_explosion();
    }

    crash = collision_player_enemies();
    if (crash != CRASH_NONE) {
        game.crash_type = crash;
        game.shake_timer = SHAKE_DURATION;
        game.crash_timer = CRASH_TEXT_DURATION;
        sound_explosion();

        if (player_hit()) {
            game.state = STATE_DYING;
            return;
        }
    }

    // Spawn enemies periodically
    game.frame_count++;
    if (game.frame_count % 60 == 0) {
        enemies_spawn(game.level);
    }

    // Level progression
    if (game.enemies_killed >= 10 * game.level) {
        game.level++;
        game.enemies_killed = 0;
    }

    // Survival bonus (every 10 frames)
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

// Render HUD text on ULA
static void render_hud_text(void) {
    ula_print_at(0, 0, "SCORE", ATTR_WHITE_ON_BLACK);
    ula_print_num(6, 0, game.score, ATTR_YELLOW_ON_BLACK);
    ula_print_at(25, 0, "LIVES", ATTR_WHITE_ON_BLACK);
    ula_print_num(31, 0, player.lives, ATTR_YELLOW_ON_BLACK);
}

// Update during dying state - just move enemies, no scrolling
void game_update_dying(void) {
    uint8_t i;

    // Decrement timers
    if (game.crash_timer > 0) game.crash_timer--;
    if (game.shake_timer > 0) game.shake_timer--;

    // Move enemies (let them continue moving down)
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].y += enemies[i].dy;
            // Deactivate if off screen
            if (enemies[i].y > SCREEN_HEIGHT + 16) {
                enemies[i].active = 0;
            }
        }
    }

    // Update frame counter for animation
    game.frame_count++;

    // Spawn fewer enemies (every 120 frames instead of 60)
    if (game.frame_count % 120 == 0) {
        enemies_spawn(game.level);
    }
}

// Render during dying state - no player, just enemies
void game_render_dying(void) {
    uint8_t sprite_slot = 0;

    // Render HUD text overlay
    render_hud_text();

    // Hide player slots (player + shadow)
    sprite_slot = player_hide(sprite_slot);

    // Hide bullet slots
    sprite_slot = bullets_hide(sprite_slot);

    // Render enemy shadows
    sprite_slot = enemies_render_shadows(sprite_slot, game.frame_count);

    // Render enemies with animation
    sprite_slot = enemies_render(sprite_slot, game.frame_count);

    // Hide unused slots
    while (sprite_slot < 32) {
        sprite_hide(sprite_slot++);
    }
}

void game_render(void) {
    uint8_t sprite_slot = 0;

    // Render HUD text overlay
    render_hud_text();

    // Level is rendered by tilemap hardware (scrolled via tilemap_scroll)

    // Render player shadow and player
    sprite_slot = player_render(sprite_slot);

    // Render bullets
    sprite_slot = bullets_render(sprite_slot);

    // Render enemy shadows first (behind enemies)
    sprite_slot = enemies_render_shadows(sprite_slot, game.frame_count);

    // Render enemies with animation
    sprite_slot = enemies_render(sprite_slot, game.frame_count);

    // Hide unused sprite slots
    while (sprite_slot < 32) {
        sprite_hide(sprite_slot++);
    }
}
