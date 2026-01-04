/*
 * Dark Nebula - ZX Spectrum Next
 * tiles.h - Layer 2 scrolling background definitions
 */

#ifndef TILES_H
#define TILES_H

#include <stdint.h>

// Screen dimensions
#define SCREEN_W  256
#define SCREEN_H  192

// Highway position (centered, 64 pixels wide)
#define HIGHWAY_WIDTH_PX  64
#define HIGHWAY_LEFT_PX   96   // (256-64)/2
#define HIGHWAY_RIGHT_PX  160  // 96 + 64

// Scroll speed
#define SCROLL_SPEED       1    // Pixels per frame
#define PARALLAX_DIVISOR   2    // Background scrolls at 1/2 speed

// Function prototypes
void tiles_init(void);
void tiles_upload_patterns(void);
void tilemap_init(void);
void tilemap_scroll(int16_t offset_y);
void layer2_init_background(void);
void layer2_scroll(int16_t offset_y);
void layer2_scroll_x(int16_t offset_x);
void layer2_enable(void);
void layer2_disable(void);
void highway_init(void);
void highway_scroll(int16_t offset_y);
void level_generate_row(uint8_t row);

// Layer 2 text drawing
void layer2_draw_char(uint8_t px, uint8_t py, char c, uint8_t color);
void layer2_draw_string(uint8_t px, uint8_t py, const char *str, uint8_t color);
void layer2_draw_num(uint8_t px, uint8_t py, uint16_t num, uint8_t color);
int16_t get_scroll_y(void);

// Scroll state
extern int16_t scroll_y;
extern int16_t scroll_y_sub;

#endif // TILES_H
