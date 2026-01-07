#ifndef BULLET_H
#define BULLET_H

#include <stdint.h>
#include "game.h"

// Initialize bullets (clear all)
void bullets_init(void);

// Update bullet positions
void bullets_update(void);

// Spawn a bullet from player position
void bullets_spawn(int16_t player_x, int16_t player_y);

// Render bullets using sprites
// Returns next available sprite slot
uint8_t bullets_render(uint8_t sprite_slot);

// Hide all bullet sprites
// Returns next available sprite slot
uint8_t bullets_hide(uint8_t sprite_slot);

// Global bullets array
extern Entity bullets[MAX_BULLETS];

#endif // BULLET_H
