#ifndef TILEMAP_H
#define TILEMAP_H

#include <stdint.h>

// Highway position (centered, 64 pixels wide)
#define HIGHWAY_WIDTH_PX  64
#define HIGHWAY_LEFT_PX   96
#define HIGHWAY_RIGHT_PX  160

// Scroll speed
#define SCROLL_SPEED       1
#define PARALLAX_DIVISOR   2

// Initialize tilemap with highway
void tilemap_init(void);

// Enable/disable tilemap display
void tilemap_enable(void);
void tilemap_disable(void);

// Scroll tilemap vertically
void tilemap_scroll(int16_t offset_y);

// Layer priority helpers
void set_layers_gameplay(void);
void set_layers_menu(void);

// Scroll state
extern int16_t scroll_y;

#endif // TILEMAP_H
