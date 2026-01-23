#include <stdint.h>
#include "enemy.h"
#include "sprites.h"

// Global enemies array
Entity enemies[MAX_ENEMIES];

// Random seed
static uint16_t rand_seed = 0x1234;

// Formation tracking
static uint8_t formation_leader[MAX_ENEMIES];  // Index of formation leader (255 = is leader or solo)
static uint8_t formation_type[MAX_ENEMIES];    // Formation type for each enemy
static uint8_t formation_phase[MAX_ENEMIES];   // Phase in movement pattern (for Galaga dive)
static int16_t formation_start_x[MAX_ENEMIES]; // Starting X for pattern calculations

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
        formation_leader[i] = 255;
        formation_type[i] = 0;
        formation_phase[i] = 0;
    }
}

// Sine table for smooth movement (quarter wave, scaled to max 48)
static const int8_t sine_table[16] = {
    0, 9, 18, 27, 33, 39, 44, 47, 48, 47, 44, 39, 33, 27, 18, 9
};

// Get sine value for angle (0-63 maps to 0-360 degrees)
static int8_t get_sine(uint8_t angle) {
    uint8_t quadrant = (angle >> 4) & 3;
    uint8_t idx = angle & 15;

    switch (quadrant) {
        case 0: return sine_table[idx];
        case 1: return sine_table[15 - idx];
        case 2: return -sine_table[idx];
        default: return -sine_table[15 - idx];
    }
}

// Update enemy positions
void enemies_update(void) {
    uint8_t i;
    uint8_t leader;
    int8_t wave_offset;

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {

            switch (formation_type[i]) {
                case FORMATION_SINGLE_PATROL:
                case FORMATION_GROUP_PATROL:
                    // Patrol movement: move down while oscillating left/right
                    // Slow horizontal movement - only every 2nd frame
                    if (enemies[i].frame & 1) {
                        enemies[i].x += enemies[i].dx;
                    }
                    enemies[i].y += enemies[i].dy;

                    // Bounce off edges
                    if (enemies[i].x < GAME_LEFT || enemies[i].x > GAME_RIGHT - ENEMY_WIDTH) {
                        enemies[i].dx = -enemies[i].dx;
                        // Sync group patrol direction
                        if (formation_type[i] == FORMATION_GROUP_PATROL && formation_leader[i] == 255) {
                            uint8_t j;
                            for (j = 0; j < MAX_ENEMIES; j++) {
                                if (formation_leader[j] == i) {
                                    enemies[j].dx = enemies[i].dx;
                                }
                            }
                        }
                    }
                    break;

                case FORMATION_ARROW:
                    // Arrow formation: follow leader with offset
                    leader = formation_leader[i];
                    if (leader == 255) {
                        // Leader: simple patrol (slow horizontal)
                        if (enemies[i].frame & 1) {
                            enemies[i].x += enemies[i].dx;
                        }
                        enemies[i].y += enemies[i].dy;
                        if (enemies[i].x < GAME_LEFT + 30 || enemies[i].x > GAME_RIGHT - ENEMY_WIDTH - 30) {
                            enemies[i].dx = -enemies[i].dx;
                        }
                    } else if (enemies[leader].active) {
                        // Follower: track leader position with offset
                        enemies[i].y += enemies[i].dy;
                        // Smoothly move toward leader x + offset
                        int16_t target_x = enemies[leader].x + (int8_t)(formation_start_x[i]);
                        if (enemies[i].frame & 1) {
                            if (enemies[i].x < target_x) enemies[i].x += 1;
                            else if (enemies[i].x > target_x) enemies[i].x -= 1;
                        }
                    } else {
                        // Leader dead, become independent
                        formation_type[i] = FORMATION_SINGLE_PATROL;
                        formation_leader[i] = 255;
                    }
                    break;

                case FORMATION_GALAGA_DIVE:
                    // Galaga dive: slow swooping curved pattern
                    // Only update phase every 2nd frame for slower movement
                    if (enemies[i].frame & 1) {
                        formation_phase[i] += 1;
                    }

                    wave_offset = get_sine(formation_phase[i]);
                    enemies[i].x = formation_start_x[i] + (enemies[i].dx > 0 ? wave_offset : -wave_offset);

                    // Slow vertical descent - only move every 2nd frame
                    if (enemies[i].frame & 1) {
                        enemies[i].y += 1;
                    }
                    break;
            }

            // Deactivate if off screen bottom
            if (enemies[i].y > SCREEN_HEIGHT) {
                enemies[i].active = 0;
                formation_leader[i] = 255;
                formation_type[i] = 0;
                formation_phase[i] = 0;
            }

            // Animation frame
            enemies[i].frame++;
        }
    }
}

// Count free enemy slots
static uint8_t count_free_slots(void) {
    uint8_t count = 0;
    uint8_t i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) count++;
    }
    return count;
}

// Find next free slot starting from index
static uint8_t find_free_slot(uint8_t start) {
    uint8_t i;
    for (i = start; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) return i;
    }
    return 255;  // No slot found
}

// Initialize a single enemy
static void init_enemy(uint8_t idx, int16_t x, int16_t y, int8_t dx, int8_t dy, uint8_t level) {
    uint8_t type = fast_rand() % 4;

    enemies[idx].active = 1;
    enemies[idx].x = x;
    enemies[idx].y = y;
    enemies[idx].dx = dx;
    enemies[idx].dy = dy;
    enemies[idx].frame = 0;

    if (type < 3 || level < 2) {
        enemies[idx].type = 0;
        enemies[idx].health = 1;
    } else {
        enemies[idx].type = 1;
        enemies[idx].health = 2;
    }
}

// Spawn a formation of enemies
void enemies_spawn(uint8_t level) {
    uint8_t free_slots = count_free_slots();
    uint8_t form_type;
    uint8_t form_size;
    uint8_t i, slot, leader_slot;
    int16_t start_x;
    int8_t direction;

    if (free_slots == 0) return;

    // Choose formation based on available slots
    form_type = fast_rand() % NUM_FORMATION_TYPES;

    // Ensure we have enough slots for the formation
    if (form_type == FORMATION_GROUP_PATROL && free_slots < 2) {
        form_type = FORMATION_SINGLE_PATROL;
    }
    if (form_type == FORMATION_ARROW && free_slots < 3) {
        form_type = (free_slots >= 2) ? FORMATION_GROUP_PATROL : FORMATION_SINGLE_PATROL;
    }
    if (form_type == FORMATION_GALAGA_DIVE && free_slots < 2) {
        form_type = FORMATION_SINGLE_PATROL;
    }

    // Random start side (0 = from left, 1 = from right)
    direction = (fast_rand() & 1) ? 1 : -1;

    switch (form_type) {
        case FORMATION_SINGLE_PATROL:
            // Single enemy patrolling
            slot = find_free_slot(0);
            if (slot == 255) return;

            start_x = (direction > 0) ? GAME_LEFT + 10 : GAME_RIGHT - ENEMY_WIDTH - 10;
            init_enemy(slot, start_x, -ENEMY_HEIGHT, direction, ENEMY_SPEED, level);
            formation_type[slot] = FORMATION_SINGLE_PATROL;
            formation_leader[slot] = 255;
            formation_start_x[slot] = start_x;
            break;

        case FORMATION_GROUP_PATROL:
            // 2-4 enemies patrolling together
            form_size = 2 + (fast_rand() % 3);  // 2, 3, or 4
            if (form_size > free_slots) form_size = free_slots;

            start_x = (direction > 0) ?
                      GAME_LEFT + 20 :
                      GAME_RIGHT - ENEMY_WIDTH - 20 - (form_size - 1) * FORMATION_SPACING;

            leader_slot = 255;
            for (i = 0; i < form_size; i++) {
                slot = find_free_slot(0);
                if (slot == 255) break;

                init_enemy(slot,
                          start_x + i * FORMATION_SPACING,
                          -ENEMY_HEIGHT - (i * 8),  // Stagger entry
                          direction, ENEMY_SPEED, level);
                formation_type[slot] = FORMATION_GROUP_PATROL;

                if (i == 0) {
                    leader_slot = slot;
                    formation_leader[slot] = 255;  // First is leader
                } else {
                    formation_leader[slot] = leader_slot;
                }
                formation_start_x[slot] = enemies[slot].x;
            }
            break;

        case FORMATION_ARROW:
            // Arrow/V formation (1 leader + 2 wings)
            form_size = 3;
            if (form_size > free_slots) form_size = free_slots;

            start_x = GAME_LEFT + 60 + (fast_rand() % 100);

            // Spawn leader at front
            leader_slot = find_free_slot(0);
            if (leader_slot == 255) return;

            init_enemy(leader_slot, start_x, -ENEMY_HEIGHT, direction, ENEMY_SPEED, level);
            formation_type[leader_slot] = FORMATION_ARROW;
            formation_leader[leader_slot] = 255;
            formation_start_x[leader_slot] = start_x;

            // Spawn wing enemies
            for (i = 1; i < form_size; i++) {
                slot = find_free_slot(0);
                if (slot == 255) break;

                int8_t x_offset = (i == 1) ? -FORMATION_SPACING : FORMATION_SPACING;
                init_enemy(slot,
                          start_x + x_offset,
                          -ENEMY_HEIGHT - FORMATION_SPACING,  // Behind leader
                          direction, ENEMY_SPEED, level);
                formation_type[slot] = FORMATION_ARROW;
                formation_leader[slot] = leader_slot;
                formation_start_x[slot] = x_offset;  // Store offset for tracking
            }
            break;

        case FORMATION_GALAGA_DIVE:
            // Galaga-style diving enemies
            form_size = 2 + (fast_rand() % 2);  // 2 or 3
            if (form_size > free_slots) form_size = free_slots;

            start_x = GAME_LEFT + 40 + (fast_rand() % 140);

            for (i = 0; i < form_size; i++) {
                slot = find_free_slot(0);
                if (slot == 255) break;

                init_enemy(slot,
                          start_x + (i * 24) - ((form_size - 1) * 12),
                          -ENEMY_HEIGHT - (i * 16),
                          direction, 0, level);  // dy=0, movement handled by pattern
                formation_type[slot] = FORMATION_GALAGA_DIVE;
                formation_leader[slot] = 255;
                formation_phase[slot] = i * 16;  // Stagger phases
                formation_start_x[slot] = enemies[slot].x;
            }
            break;
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
