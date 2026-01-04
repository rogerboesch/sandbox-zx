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

#endif // LAYER2_H
