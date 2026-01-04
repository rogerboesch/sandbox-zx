/*
 * Dark Nebula - ZX Spectrum Next
 * tiles.h - 3-Layer Graphics System
 *
 * BACK:   Layer 2 - Blue grid parallax background (half speed)
 * MIDDLE: Tilemap - Highway/level (full speed, 16x16 tiles)
 * FRONT:  Sprites - Player, enemies, HUD
 */

#ifndef TILES_H
#define TILES_H

#include <stdint.h>

// Screen dimensions
#define SCREEN_W  256
#define SCREEN_H  192

// Highway position (centered, 64 pixels wide)
#define HIGHWAY_WIDTH_PX  64
#define HIGHWAY_LEFT_PX   96
#define HIGHWAY_RIGHT_PX  160

// Scroll speed
#define SCROLL_SPEED       1
#define PARALLAX_DIVISOR   2

// Initialize all graphics layers
void tiles_init(void);

// Enable/disable graphics layers
void layer2_enable(void);
void layer2_disable(void);
void tilemap_enable(void);
void tilemap_disable(void);

// Scroll Layer 2 (background - half speed)
void layer2_scroll(int16_t offset_y);
void layer2_scroll_x(int16_t offset_x);

// Scroll tilemap (highway - full speed)
void tilemap_scroll(int16_t offset_y);

// Get scroll position
int16_t get_scroll_y(void);

// Scroll state
extern int16_t scroll_y;
extern int16_t scroll_y_sub;

#endif // TILES_H
