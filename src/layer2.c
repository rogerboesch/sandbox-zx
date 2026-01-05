#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include "layer2.h"

// Layer 2 uses 8K banks 16-21 (6 banks x 8K = 48K for 256x192x8bpp)
// We'll use MMU slot 2 (0x4000-0x5FFF) for writing
#define MMU_SLOT2_REG  0x52

// Draw blue grid to one 8K bank of Layer 2
static void layer2_draw_8k_bank(uint8_t l2_bank, uint8_t start_y) {
    uint8_t y;
    uint16_t x;
    uint8_t *row;
    uint8_t world_y, tile_y, tile_x;
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
        tile_y = world_y & 0x0F;

        for (x = 0; x < 256; x++) {
            tile_x = x & 0x0F;
            if (tile_x == 0 || tile_x == 15 || tile_y == 0 || tile_y == 15) {
                row[x] = 0x03;  // Blue border
            } else {
                row[x] = 0x00;  // Black inside
            }
        }
    }

    // Restore original bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = old_bank;
}

// Initialize Layer 2 with blue grid
void layer2_init(void) {
    // Set Layer 2 RAM to start at bank 16 (8K banks)
    // Register 0x12 uses 16K bank number, so 16K bank 8 = 8K banks 16-17
    IO_NEXTREG_REG = 0x12;
    IO_NEXTREG_DAT = 8;

    // Draw grid to all 6 8K banks (192 lines total)
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
