#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include "layer2.h"
#include "tileset.h"

// External references to banked data (forces linker to include)
extern uint8_t border_image_bank40;
extern uint8_t border_image_bank41;

// Use a dummy reference if necessary
void force_include(void) {
    volatile uint8_t *ptr = &border_image_bank40;
    ptr = &border_image_bank41;
    (void)ptr;
}

// Layer 2 uses 8K banks 16-21 (6 banks x 8K = 48K for 256x192x8bpp)
// We'll use MMU slot 2 (0x4000-0x5FFF) for writing
#define MMU_SLOT2_REG  0x52
#define MMU_SLOT3_REG  0x53

// Border image stored in 16K bank 20 (8K pages 40-41) loaded by NEX loader
// Image dimensions: 60x191 = 11460 bytes
// 16K bank 20 maps to 0xC000-0xFFFF when using MMU slots 6-7
#define BORDER_IMAGE_WIDTH   60
#define BORDER_IMAGE_HEIGHT  191
#define BORDER_IMAGE_BANK_16K  20  // 16K bank number for MMU slot 6

// Layer 2 background tiles (2x2 block = 16x16 pixels from tileset.h)
#define L2_TILE_TL  tile_O0  // top-left
#define L2_TILE_TR  tile_P0  // top-right
#define L2_TILE_BL  tile_O1  // bottom-left
#define L2_TILE_BR  tile_P1  // bottom-right

// Block coverage percentage (0-100)
// Each block is 16x16 = 256 pixels, screen is 256x192 = 49152 pixels
// 20% coverage = ~38 blocks
#define L2_BLOCK_COVERAGE  20

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

// Simple LCG random number generator
static uint16_t rand_seed = 12345;

static uint16_t rand16(void) {
    rand_seed = rand_seed * 25173 + 13849;
    return rand_seed;
}

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

// Draw a single 8x8 tile at position (x, y)
static void layer2_draw_tile(uint8_t x, uint8_t y, const uint8_t *tile) {
    uint8_t px, py;
    uint8_t pal_idx;

    for (py = 0; py < 8; py++) {
        if (y + py >= 192) continue;
        for (px = 0; px < 8; px++) {
            pal_idx = get_tile_pixel(tile, px, py);
            layer2_plot(x + px, y + py, zx_to_rgb332[pal_idx]);
        }
    }
}

// Draw a single 16x16 block at position (bx, by)
static void layer2_draw_block(uint8_t bx, uint8_t by) {
    uint8_t px, py;
    uint8_t tile_px, tile_py;
    const uint8_t *tile;
    uint8_t pal_idx;
    uint8_t screen_x, screen_y;

    for (py = 0; py < 16; py++) {
        screen_y = by + py;
        if (screen_y >= 192) continue;

        tile_py = py & 0x07;

        for (px = 0; px < 16; px++) {
            screen_x = bx + px;
            // screen_x wraps naturally with uint8_t

            tile_px = px & 0x07;

            // Select tile from 2x2 block
            if (py < 8) {
                tile = (px < 8) ? L2_TILE_TL : L2_TILE_TR;
            } else {
                tile = (px < 8) ? L2_TILE_BL : L2_TILE_BR;
            }

            pal_idx = get_tile_pixel(tile, tile_px, tile_py);
            layer2_plot(screen_x, screen_y, zx_to_rgb332[pal_idx]);
        }
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

// Clear Layer 2 in 256x256 mode (8 banks instead of 6)
static void layer2_clear_256(uint8_t color) {
    uint8_t l2_bank;
    uint16_t i;
    uint8_t *ptr;
    uint8_t old_bank;

    // Save current MMU slot 2 bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    old_bank = IO_NEXTREG_DAT;

    // Clear all 8 8K banks (256 lines)
    for (l2_bank = 16; l2_bank < 24; l2_bank++) {
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

// Draw a single 16x16 block at position (bx, by) - supports 256 height
static void layer2_draw_block_256(uint8_t bx, uint8_t by) {
    uint8_t px, py;
    uint8_t tile_px, tile_py;
    const uint8_t *tile;
    uint8_t pal_idx;
    uint8_t screen_x, screen_y;
    uint8_t l2_bank;
    uint8_t *ptr;
    uint8_t old_bank;

    // Save current MMU slot 2 bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    old_bank = IO_NEXTREG_DAT;

    for (py = 0; py < 16; py++) {
        screen_y = by + py;
        tile_py = py & 0x07;

        // Determine which 8K bank (each bank = 32 lines)
        l2_bank = 16 + (screen_y / 32);

        // Map the appropriate Layer 2 bank
        IO_NEXTREG_REG = MMU_SLOT2_REG;
        IO_NEXTREG_DAT = l2_bank;

        for (px = 0; px < 16; px++) {
            screen_x = bx + px;
            tile_px = px & 0x07;

            // Select tile from 2x2 block
            if (py < 8) {
                tile = (px < 8) ? L2_TILE_TL : L2_TILE_TR;
            } else {
                tile = (px < 8) ? L2_TILE_BL : L2_TILE_BR;
            }

            pal_idx = get_tile_pixel(tile, tile_px, tile_py);
            ptr = (uint8_t *)0x4000 + ((screen_y % 32) * 256) + screen_x;
            *ptr = zx_to_rgb332[pal_idx];
        }
    }

    // Restore original bank
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = old_bank;
}

// Draw border image from bank to Layer 2
// Source data is in pages 40-41, we map one at a time to slot 3 (0x6000)
// Destination is Layer 2 banks 16-21 mapped to slot 2 (0x4000)
// mirror: 0 = normal, 1 = horizontally mirrored
static void layer2_draw_border_from_bank(uint8_t x, uint8_t y, uint8_t mirror) {
    uint8_t row, col;
    uint8_t screen_y;
    uint8_t l2_bank, last_l2_bank;
    uint8_t src_page, last_src_page;
    uint8_t *dst;
    const uint8_t *src;
    uint8_t old_slot2, old_slot3;
    uint16_t src_offset;

    // Save current MMU banks
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    old_slot2 = IO_NEXTREG_DAT;
    IO_NEXTREG_REG = MMU_SLOT3_REG;
    old_slot3 = IO_NEXTREG_DAT;

    last_l2_bank = 0xFF;
    last_src_page = 0xFF;
    src_offset = 0;

    for (row = 0; row < BORDER_IMAGE_HEIGHT; row++) {
        screen_y = y + row;
        if (screen_y >= 192) break;

        // Determine destination Layer 2 bank (each bank = 32 lines)
        l2_bank = 16 + (screen_y / 32);

        // Determine source page (page 40 or 41 depending on offset)
        // Each 8K page holds 8192 bytes
        src_page = 40 + (src_offset / 8192);

        // Remap destination bank if changed
        if (l2_bank != last_l2_bank) {
            IO_NEXTREG_REG = MMU_SLOT2_REG;
            IO_NEXTREG_DAT = l2_bank;
            last_l2_bank = l2_bank;
        }

        // Remap source page if changed
        if (src_page != last_src_page) {
            IO_NEXTREG_REG = MMU_SLOT3_REG;
            IO_NEXTREG_DAT = src_page;
            last_src_page = src_page;
        }

        // Calculate addresses
        dst = (uint8_t *)0x4000 + ((screen_y % 32) * 256) + x;
        src = (const uint8_t *)0x6000 + (src_offset % 8192);

        // Copy row (mirrored or normal)
        if (mirror) {
            for (col = 0; col < BORDER_IMAGE_WIDTH; col++) {
                dst[col] = src[BORDER_IMAGE_WIDTH - 1 - col];
            }
        } else {
            for (col = 0; col < BORDER_IMAGE_WIDTH; col++) {
                dst[col] = src[col];
            }
        }
        src_offset += BORDER_IMAGE_WIDTH;
    }

    // Restore original banks
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = old_slot2;
    IO_NEXTREG_REG = MMU_SLOT3_REG;
    IO_NEXTREG_DAT = old_slot3;
}

// Initialize Layer 2 with white background and border images (256x192 mode)
void layer2_init(void) {
    // Disable interrupts during bank manipulation to prevent
    // IM1 handler from corrupting Layer 2 memory at 0x4000
    intrinsic_di();

    // Set Layer 2 RAM to start at bank 16 (8K banks)
    // Register 0x12 uses 16K bank number, so 16K bank 8 = 8K banks 16-17
    IO_NEXTREG_REG = 0x12;
    IO_NEXTREG_DAT = 8;

    // Fill with white background (256x192 = 6 banks)
    layer2_clear(0xFF);

    // Draw left border image from bank 40
    layer2_draw_border_from_bank(0, 0, 0);

    // Draw right border image mirrored
    layer2_draw_border_from_bank(256 - BORDER_IMAGE_WIDTH, 0, 1);

    // Restore MMU slot 2 before enabling interrupts
    IO_NEXTREG_REG = MMU_SLOT2_REG;
    IO_NEXTREG_DAT = 10;

    // Now safe to enable interrupts
    intrinsic_ei();
}

// Enable Layer 2 display
void layer2_enable(void) {
    // Register 0x69: Layer 2 Control
    // Bit 7: Layer 2 enable
    // Bit 6: Layer 2 shadow (0 = use bank in reg 0x12)
    // Bits 5-4: Resolution (00 = 256x192, 01 = 320x256, 10 = 640x256, 11 = 256x256)
    // Bits 3-0: Palette offset
    ZXN_NEXTREG(0x69, 0x80);  // Enable Layer 2, 256x192 mode (bits 5-4 = 00)
}

// Disable Layer 2 display
void layer2_disable(void) {
    // Use ZXN_NEXTREG to ensure proper register write
    ZXN_NEXTREG(0x69, 0x00);  // Disable Layer 2
}

// Scroll Layer 2 vertically (parallax background - half speed)
// Wraps within 0-191 range to avoid jump in 256x192 mode
void layer2_scroll(int16_t offset_y) {
    // Wrap to 0-191 range for 256x192 mode
    // Use 192 - (offset % 192) to match tilemap scroll direction
    int16_t wrapped = offset_y % 192;
    if (wrapped < 0) wrapped += 192;
    uint8_t scroll = (uint8_t)((192 - wrapped) % 192);
    IO_NEXTREG_REG = 0x17;
    IO_NEXTREG_DAT = scroll;
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
