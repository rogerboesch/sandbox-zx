/*
 * Dark Nebula - ZX Spectrum Next
 * sprites.h - Sprite handling
 */

#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>

// Initialize sprite system (palette + patterns)
void sprites_init(void);

// Upload sprite patterns to pattern memory
void sprites_upload_patterns(void);

// Set sprite attributes
void sprite_set(uint8_t slot, int16_t x, int16_t y, uint8_t pattern, uint8_t flags);

// Hide a sprite
void sprite_hide(uint8_t slot);

#endif // SPRITES_H
