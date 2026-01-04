/*
 * Dark Nebula - ZX Spectrum Next
 * tiles.c - 3-Layer Graphics System
 *
 * BACK:   Layer 2 - Blue grid parallax background (half speed)
 * MIDDLE: Tilemap - Highway/level (full speed, 16x16 tiles)
 * FRONT:  Sprites - Player, enemies, HUD
 */

#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <string.h>
#include "tiles.h"
#include "game.h"

// Scroll state
int16_t scroll_y = 0;
int16_t scroll_y_sub = 0;

// ============================================================
// LAYER 2 - Back Layer (Parallax Background)
// ============================================================
#define LAYER2_PORT  0x123B

// Draw blue grid to Layer 2 (16x16 grid pattern)
static void layer2_draw_grid_bank(uint8_t bank) {
    uint8_t y, x;
    uint8_t *row;
    uint8_t world_y, tile_y, tile_x;

    z80_outp(LAYER2_PORT, 0x03 | (bank << 6));

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
static void layer2_init(void) {
    layer2_draw_grid_bank(0);
    layer2_draw_grid_bank(1);
    layer2_draw_grid_bank(2);
    z80_outp(LAYER2_PORT, 0x00);  // Disable write
}

// ============================================================
// TILEMAP - Middle Layer (Highway/Level)
// ============================================================
#define REG_TILEMAP_CTRL     0x6B
#define REG_TILEMAP_ATTR     0x6C
#define REG_TILEMAP_BASE     0x6E
#define REG_TILEMAP_TILES    0x6F
#define REG_TILEMAP_TRANS    0x4C
#define REG_TILEMAP_XSCROLL  0x2F
#define REG_TILEMAP_YSCROLL  0x31

// Tilemap: 40x32 tiles, each tile = 1 byte
// At 0x6000 = 1280 bytes
#define TILEMAP_BASE_ADDR    0x6000

// Tile patterns: 16x16 4-bit = 128 bytes each
// At 0x6C00
#define TILES_BASE_ADDR      0x6C00

// Tile indices
#define TILE_TRANS    0
#define TILE_MAGENTA  1
#define TILE_DASH     2

// Define 16x16 tile patterns (4-bit = 128 bytes each)
static void tilemap_define_tiles(void) {
    uint8_t *tiles = (uint8_t *)TILES_BASE_ADDR;
    uint8_t i;

    // Tile 0: Transparent (palette index 0)
    for (i = 0; i < 128; i++) {
        tiles[i] = 0x00;
    }
    tiles += 128;

    // Tile 1: Solid magenta (palette index 1)
    for (i = 0; i < 128; i++) {
        tiles[i] = 0x11;  // Two pixels of palette index 1
    }
    tiles += 128;

    // Tile 2: White dash on magenta (for center line)
    for (i = 0; i < 128; i++) {
        // Rows 6-9 are white (palette 2), rest magenta (palette 1)
        uint8_t row = i / 8;
        if (row >= 6 && row <= 9) {
            tiles[i] = 0x22;  // White
        } else {
            tiles[i] = 0x11;  // Magenta
        }
    }
}

// Set up tilemap palette
static void tilemap_setup_palette(void) {
    // Select tilemap palette, auto-increment
    IO_NEXTREG_REG = 0x43;
    IO_NEXTREG_DAT = 0x30;

    // Start at index 0
    IO_NEXTREG_REG = 0x40;
    IO_NEXTREG_DAT = 0;

    // Write palette colors (RGB332)
    IO_NEXTREG_REG = 0x41;
    IO_NEXTREG_DAT = 0xE0;  // 0: Red (will be transparent)
    IO_NEXTREG_DAT = 0xE3;  // 1: Magenta
    IO_NEXTREG_DAT = 0xFF;  // 2: White
    IO_NEXTREG_DAT = 0xFC;  // 3: Yellow
}

// Fill tilemap - highway in center (tiles 12-19), transparent elsewhere
static void tilemap_fill(void) {
    uint8_t *tmap = (uint8_t *)TILEMAP_BASE_ADDR;
    uint8_t x, y;

    // 40 columns x 32 rows
    // Highway at pixels 96-159 = tiles 6-9 (16x16 tiles, so 96/16=6)
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 40; x++) {
            uint8_t tile;

            if (x >= 6 && x <= 9) {
                // Highway area
                if ((x == 7 || x == 8) && (y % 2 == 0)) {
                    tile = TILE_DASH;  // Center dashes
                } else {
                    tile = TILE_MAGENTA;
                }
            } else {
                tile = TILE_TRANS;  // Transparent - shows Layer 2
            }
            *tmap++ = tile;
        }
    }
}

// Enable tilemap display
void tilemap_enable(void) {
    // Register 0x6B: Tilemap Control
    // Bit 7: Enable tilemap
    // Bit 6: 0 = 40x32, 1 = 80x32
    // Bit 5: 0 = 8x8 tiles, 1 = 16x16 tiles (in ULA mode)
    // Bit 4: 0 = no attributes, 1 = attributes in tilemap
    // Bit 1: 0 = palette 0, 1 = palette 1
    // Bit 0: 0 = 4-bit tiles, 1 = 8-bit tiles
    IO_NEXTREG_REG = REG_TILEMAP_CTRL;
    IO_NEXTREG_DAT = 0x80;  // Enable tilemap, 40x32, 4-bit tiles
}

// Disable tilemap display
void tilemap_disable(void) {
    IO_NEXTREG_REG = REG_TILEMAP_CTRL;
    IO_NEXTREG_DAT = 0x00;  // Disable tilemap
}

// Initialize tilemap
static void tilemap_init(void) {
    // Define tile patterns
    tilemap_define_tiles();

    // Set up palette
    tilemap_setup_palette();

    // Fill tilemap data
    tilemap_fill();

    // Set tilemap base address: 0x6000 >> 8 = 0x60
    IO_NEXTREG_REG = REG_TILEMAP_BASE;
    IO_NEXTREG_DAT = (TILEMAP_BASE_ADDR >> 8);

    // Set tile definitions address: 0x6C00 >> 8 = 0x6C
    IO_NEXTREG_REG = REG_TILEMAP_TILES;
    IO_NEXTREG_DAT = (TILES_BASE_ADDR >> 8);

    // Set transparency index to 0 (palette index 0 = transparent)
    IO_NEXTREG_REG = REG_TILEMAP_TRANS;
    IO_NEXTREG_DAT = 0x00;

    // Default attribute
    IO_NEXTREG_REG = REG_TILEMAP_ATTR;
    IO_NEXTREG_DAT = 0x00;

    // Tilemap starts disabled (enabled when gameplay starts)
}

// ============================================================
// PUBLIC API
// ============================================================

void tiles_init(void) {
    // Initialize Layer 2 (back - blue grid)
    layer2_init();

    // Initialize Tilemap (middle - highway)
    tilemap_init();

    // Start with everything disabled
    layer2_disable();

    // Black border
    z80_outp(0xFE, 0x00);
}

void layer2_enable(void) {
    // Enable Layer 2
    z80_outp(LAYER2_PORT, 0x02);

    // Enable tilemap (highway)
    tilemap_enable();

    // Register 0x15: Sprite and Layers System
    // Bits 4-2: Layer priority (SLU)
    //   000 = S L U (Sprites > Layer2 > ULA/Tilemap)
    //   010 = S U L (Sprites > ULA/Tilemap > Layer2) - what we want!
    // Bit 1: Sprites over border
    // Bit 0: Sprites visible
    IO_NEXTREG_REG = 0x15;
    IO_NEXTREG_DAT = 0x0B;  // S U L order, sprites visible and over border
}

void layer2_disable(void) {
    // Disable Layer 2
    z80_outp(LAYER2_PORT, 0x00);

    // Disable tilemap
    tilemap_disable();

    // Sprites only
    IO_NEXTREG_REG = 0x15;
    IO_NEXTREG_DAT = 0x01;
}

// Scroll Layer 2 (parallax background - half speed)
void layer2_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = 0x17;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

// Scroll Layer 2 horizontally
void layer2_scroll_x(int16_t offset_x) {
    IO_NEXTREG_REG = 0x16;
    IO_NEXTREG_DAT = (uint8_t)(offset_x & 0xFF);
}

// Scroll tilemap (highway - full speed)
void tilemap_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

void highway_scroll(int16_t offset_y) {
    tilemap_scroll(offset_y);
}

int16_t get_scroll_y(void) {
    return scroll_y;
}

// Stubs for compatibility
void tiles_upload_patterns(void) { }
void highway_init(void) { }
void level_generate_row(uint8_t row) { (void)row; }
void layer2_init_background(void) { }
void layer2_init_full_background(void) { }
