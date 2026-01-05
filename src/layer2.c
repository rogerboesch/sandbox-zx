/*
 * Dark Nebula - ZX Spectrum Next
 * layer2.c - Layer 2 Background Graphics
 *
 * Layer 2 is the back layer showing a blue grid parallax background
 */

#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include "layer2.h"

#define LAYER2_PORT  0x123B

// Draw blue grid to Layer 2 (16x16 grid pattern)
static void layer2_draw_grid_bank(uint8_t bank) {
    uint8_t y, x;
    uint8_t *row;
    uint8_t world_y, tile_y, tile_x;

    // Enable write only (bit 1), not display (bit 0)
    // Bits 7-6 select bank (0, 1, or 2)
    z80_outp(LAYER2_PORT, 0x02 | (bank << 6));

    for (y = 0; y < 64; y++) {
        world_y = bank * 64 + y;
        row = (uint8_t *)(0x0000 + y * 256);
        tile_y = world_y & 0x0F;

        for (x = 0; x < 255; x++) {
            tile_x = x & 0x0F;
            if (tile_x == 0 || tile_x == 15 || tile_y == 0 || tile_y == 15) {
                row[x] = 0x03;  // Blue border
            } else {
                row[x] = 0x00;  // Black inside
            }
        }
        // Last pixel (x=255)
        row[255] = (tile_y == 0 || tile_y == 15) ? 0x03 : 0x00;
    }
}

// Initialize Layer 2 with blue grid
void layer2_init(void) {
    layer2_draw_grid_bank(0);
    layer2_draw_grid_bank(1);
    layer2_draw_grid_bank(2);
    z80_outp(LAYER2_PORT, 0x00);  // Disable write
}

// Enable Layer 2 display
void layer2_enable(void) {
    z80_outp(LAYER2_PORT, 0x01);
}

// Disable Layer 2 display
void layer2_disable(void) {
    z80_outp(LAYER2_PORT, 0x00);
}

// Scroll Layer 2 vertically (parallax background - half speed)
void layer2_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = 0x17;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

// Scroll Layer 2 horizontally
void layer2_scroll_x(int16_t offset_x) {
    IO_NEXTREG_REG = 0x16;
    IO_NEXTREG_DAT = (uint8_t)(offset_x & 0xFF);
}

// Clear Layer 2 with a color
void layer2_clear(uint8_t color) {
    uint8_t bank;
    uint16_t i;
    uint8_t *ptr;

    // Layer 2 is in banks 8, 9, 10 (256x192 @ 8bpp = 48KB)
    for (bank = 0; bank < 3; bank++) {
        // Map Layer 2 bank to slot (0x0000-0x3FFF)
        z80_outp(LAYER2_PORT, 0x03 | (bank << 6));

        ptr = (uint8_t *)0x0000;
        for (i = 0; i < 16384; i++) {
            *ptr++ = color;
        }
    }

    // Reset to normal operation
    z80_outp(LAYER2_PORT, 0x02);
}

// Plot a pixel on Layer 2
void layer2_plot(uint8_t x, uint8_t y, uint8_t color) {
    uint8_t bank;
    uint8_t *ptr;

    if (y >= 192) return;

    // Determine which bank (each bank = 64 lines)
    bank = y / 64;

    // Map the appropriate Layer 2 bank
    z80_outp(LAYER2_PORT, 0x03 | (bank << 6));

    // Calculate address within the bank
    ptr = (uint8_t *)0x0000 + ((y % 64) * 256) + x;
    *ptr = color;
}

// Draw horizontal line
void layer2_hline(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color) {
    uint8_t x;
    for (x = x1; x <= x2; x++) {
        layer2_plot(x, y, color);
    }
}

// Draw vertical line
void layer2_vline(uint8_t x, uint8_t y1, uint8_t y2, uint8_t color) {
    uint8_t y;
    for (y = y1; y <= y2; y++) {
        layer2_plot(x, y, color);
    }
}

// Fill rectangle
void layer2_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    uint8_t i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            layer2_plot(x + i, y + j, color);
        }
    }
}
