#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdint.h>

// Scroll speed
#define SCROLL_SPEED       1
#define PARALLAX_DIVISOR   2

// Initialize tilemap with level
void tilemap_init(void);

// Enable/disable tilemap display
void tilemap_enable(void);
void tilemap_disable(void);

// Scroll tilemap vertically
void tilemap_scroll(int16_t offset_y);

// Refresh entire tilemap from level data
void tilemap_refresh(void);

// Layer priority helpers
void set_layers_gameplay(void);
void set_layers_menu(void);

// Scroll state
extern int16_t scroll_y;

// Tile constants for collision checking
#define TILE_TRANS 0x06

// Get tile index at screen position (returns tile index, or TILE_TRANS if off-screen)
uint8_t tilemap_get_tile_at(int16_t screen_x, int16_t screen_y);

#endif // TILEMAP_H
