#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include "layer2.h"
#include "tileset.h"

// Layer 2 uses 8K banks 16-21 (6 banks x 8K = 48K for 256x192x8bpp)
// We'll use MMU slot 2 (0x4000-0x5FFF) for writing
#define MMU_SLOT2_REG  0x52

// Layer 2 background tiles (2x2 pattern from tileset.h)
#define L2_TILE_TL  tile_G2  // top-left
#define L2_TILE_TR  tile_H2  // top-right
#define L2_TILE_BL  tile_G3  // bottom-left
#define L2_TILE_BR  tile_H3  // bottom-right

// ZX Spectrum colors in RGB332 for Layer 2
static const uint8_t zx_to_rgb332[16] = {
    0x00,  // 0: Black
    0x02,  // 1: Blue
    0xC0,  // 2: Red
    0xC2,  // 3: Magenta
    0x18,  // 4: Green
    0x1A,  // 5: Cyan
    0xD8,  // 6: Yellow
    0xDA,  // 7: White
    0x00,  // 8: Bright Black
    0x03,  // 9: Bright Blue
    0xE0,  // 10: Bright Red
    0xE3,  // 11: Bright Magenta
    0x1C,  // 12: Bright Green
    0x1F,  // 13: Bright Cyan
    0xFC,  // 14: Bright Yellow
    0xFF   // 15: Bright White
};

// Get pixel from 4-bit tile data
static uint8_t get_tile_pixel(const uint8_t *tile, uint8_t px, uint8_t py) {
    uint8_t byte = tile[py * 4 + (px >> 1)];
    if (px & 1) {
        return byte & 0x0F;  // Low nibble (right pixel)
    }
    else {
        return byte >> 4;    // High nibble (left pixel)
    }
}

// Draw 2x2 tile pattern to one 8K bank of Layer 2
static void layer2_draw_8k_bank(uint8_t l2_bank, uint8_t start_y) {
    uint8_t y;
    uint16_t x;
    uint8_t *row;
    uint8_t world_y, block_y, block_x;
    uint8_t tile_px, tile_py;
    const uint8_t *tile;
    uint8_t pal_idx;
    uint8_t old_bank;

    // Save current MMU slot 2 bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    old_bank = IO_NEXTREG_DAT;

    // Map Layer 2 bank to slot 2 (0x4000)
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = l2_bank;

    // Each 8K bank = 32 lines (256 bytes per line, 32 * 256 = 8192)
    for (y = 0; y < 32; y++) {
        world_y = start_y + y;
        row = (uint8_t *)(0x4000 + y * 256);

        // Which row in the 16x16 block (0-15)?
        block_y = world_y & 0x0F;
        tile_py = block_y & 0x07;  // Pixel row within 8x8 tile

        for (x = 0; x < 256; x++) {
            // Which column in the 16x16 block (0-15)?
            block_x = x & 0x0F;
            tile_px = block_x & 0x07;  // Pixel col within 8x8 tile

            // Select tile from 2x2 block
            if (block_y < 8) {
                tile = (block_x < 8) ? L2_TILE_TL : L2_TILE_TR;
            }
            else {
                tile = (block_x < 8) ? L2_TILE_BL : L2_TILE_BR;
            }

            // Get palette index and convert to RGB332
            pal_idx = get_tile_pixel(tile, tile_px, tile_py);
            row[x] = zx_to_rgb332[pal_idx];
        }
    }

    // Restore original bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = old_bank;
}

// Initialize Layer 2 with pattern
void layer2_init(void) {
    // Set Layer 2 RAM to start at bank 16 (8K banks)
    // Register 0x12 uses 16K bank number, so 16K bank 8 = 8K banks 16-17
    IO_NEXTREG_REG = 0x12;
    IO_NEXTREG_DAT = 8;

    // Draw tiles to all 6 8K banks (192 lines total)
    // Bank 16: lines 0-31, Bank 17: lines 32-63
    // Bank 18: lines 64-95, Bank 19: lines 96-127
    // Bank 20: lines 128-159, Bank 21: lines 160-191
    layer2_draw_8k_bank(16, 0);
    layer2_draw_8k_bank(17, 32);
    layer2_draw_8k_bank(18, 64);
    layer2_draw_8k_bank(19, 96);
    layer2_draw_8k_bank(20, 128);
    layer2_draw_8k_bank(21, 160);
}

// Enable Layer 2 display
void layer2_enable(void) {
    // Register 0x69: Layer 2 Control
    // Bit 7: Layer 2 enable
    // Bit 6: Layer 2 shadow (0 = use bank in reg 0x12)
    // Bits 5-4: Resolution (00 = 256x192)
    // Bits 3-0: Palette offset
    ZXN_NEXTREG(0x69, 0x80);  // Enable Layer 2, 256x192
}

// Disable Layer 2 display
void layer2_disable(void) {
    // Use ZXN_NEXTREG to ensure proper register write
    ZXN_NEXTREG(0x69, 0x00);  // Disable Layer 2
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
    uint8_t l2_bank;
    uint16_t i;
    uint8_t *ptr;
    uint8_t old_bank;

    // Save current MMU slot 2 bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    old_bank = IO_NEXTREG_DAT;

    // Clear all 6 8K banks
    for (l2_bank = 16; l2_bank < 22; l2_bank++) {
        IO_NEXTREG_REG = MMU_SLOT2_REG;
        IO_NEXTREG_DAT = l2_bank;

        ptr = (uint8_t *)0x4000;
        for (i = 0; i < 8192; i++) {
            *ptr++ = color;
        }
    }

    // Restore original bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = old_bank;
}

// Plot a pixel on Layer 2
void layer2_plot(uint8_t x, uint8_t y, uint8_t color) {
    uint8_t l2_bank;
    uint8_t *ptr;
    uint8_t old_bank;

    if (y >= 192) return;

    // Save current MMU slot 2 bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    old_bank = IO_NEXTREG_DAT;

    // Determine which 8K bank (each bank = 32 lines)
    l2_bank = 16 + (y / 32);

    // Map the appropriate Layer 2 bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = l2_bank;

    // Calculate address within the bank
    ptr = (uint8_t *)0x4000 + ((y % 32) * 256) + x;
    *ptr = color;

    // Restore original bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = old_bank;
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
