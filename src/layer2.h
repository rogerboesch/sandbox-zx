/*
 * Dark Nebula - ZX Spectrum Next
 * layer2.h - Layer 2 Background Graphics
 */

#ifndef LAYER2_H
#define LAYER2_H

#include <stdint.h>

// Initialize Layer 2 with blue grid background
void layer2_init(void);

// Enable/disable Layer 2 display
void layer2_enable(void);
void layer2_disable(void);

// Scroll Layer 2
void layer2_scroll(int16_t offset_y);
void layer2_scroll_x(int16_t offset_x);

// Drawing functions
void layer2_clear(uint8_t color);
void layer2_plot(uint8_t x, uint8_t y, uint8_t color);
void layer2_hline(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color);
void layer2_vline(uint8_t x, uint8_t y1, uint8_t y2, uint8_t color);
void layer2_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

#endif // LAYER2_H
