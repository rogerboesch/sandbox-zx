#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include "tilemap.h"

// Tilemap registers
#define REG_TILEMAP_CTRL     0x6B
#define REG_TILEMAP_ATTR     0x6C
#define REG_TILEMAP_BASE     0x6E
#define REG_TILEMAP_TILES    0x6F
#define REG_TILEMAP_TRANS    0x4C
#define REG_TILEMAP_XSCROLL  0x2F
#define REG_TILEMAP_YSCROLL  0x31

// Tilemap: 40x32 tiles, each tile = 1 byte at 0x6000
#define TILEMAP_BASE_ADDR    0x6000

// Tile patterns: 16x16 4-bit = 128 bytes each at 0x6C00
#define TILES_BASE_ADDR      0x6C00

// Tile indices
#define TILE_TRANS    0
#define TILE_MAGENTA  1
#define TILE_DASH     2

// Scroll state
int16_t scroll_y = 0;

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

// ZX Spectrum classic colors in RGB332 format
static const uint8_t zx_colors[16] = {
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
    0xE7,  // 11: Bright Magenta (avoid 0xE3 transparency)
    0x1C,  // 12: Bright Green
    0x1F,  // 13: Bright Cyan
    0xFC,  // 14: Bright Yellow
    0xFF   // 15: Bright White
};

// Set up tilemap palette
static void tilemap_setup_palette(void) {
    uint8_t i;

    // Select tilemap palette 0 for writing
    // From z88dk pattern: ULA=0x00, Layer2=0x10, Sprites=0x20, Tilemap=0x30
    IO_NEXTREG_REG = 0x43;
    IO_NEXTREG_DAT = 0x30;

    // Start at index 0
    IO_NEXTREG_REG = 0x40;
    IO_NEXTREG_DAT = 0;

    // Write ZX Spectrum colors to indices 0-15
    IO_NEXTREG_REG = 0x41;
    for (i = 0; i < 16; i++) {
        IO_NEXTREG_DAT = zx_colors[i];
    }

    // Reset palette control to ULA
    IO_NEXTREG_REG = 0x43;
    IO_NEXTREG_DAT = 0x00;
}

// Fill tilemap - highway in center, transparent elsewhere
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

// Initialize tilemap
void tilemap_init(void) {
    // Define tile patterns
    tilemap_define_tiles();

    // Set up palette
    tilemap_setup_palette();

    // Fill tilemap data
    tilemap_fill();

    // Set tilemap base address
    IO_NEXTREG_REG = REG_TILEMAP_BASE;
    IO_NEXTREG_DAT = (TILEMAP_BASE_ADDR >> 8);

    // Set tile definitions address
    IO_NEXTREG_REG = REG_TILEMAP_TILES;
    IO_NEXTREG_DAT = (TILES_BASE_ADDR >> 8);

    // Set transparency index to 0
    IO_NEXTREG_REG = REG_TILEMAP_TRANS;
    IO_NEXTREG_DAT = 0x00;

    // Default attribute
    IO_NEXTREG_REG = REG_TILEMAP_ATTR;
    IO_NEXTREG_DAT = 0x00;

    // Tilemap starts disabled
}

// Enable tilemap display
void tilemap_enable(void) {
    IO_NEXTREG_REG = REG_TILEMAP_CTRL;
    // Bit 7: Enable tilemap
    // Bit 6: Tilemap over Layer 2 (1 = tilemap on top of Layer 2)
    // Bit 5: 0 = 40x32, 1 = 80x32
    // Bit 4: 0 = 8-bit tiles, 1 = 4-bit tiles (we use 4-bit)
    // Bit 0: Enable palette index 0 as transparent
    IO_NEXTREG_DAT = 0xD1;  // Enable, over Layer2, 40x32, 4-bit, transparent index 0
}

// Disable tilemap display
void tilemap_disable(void) {
    IO_NEXTREG_REG = REG_TILEMAP_CTRL;
    IO_NEXTREG_DAT = 0x00;
}

// Scroll tilemap vertically (highway - full speed)
void tilemap_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

// Set layer priority for gameplay
// ULA on top (for HUD text), then Sprites, Layer2, Tilemap at back
void set_layers_gameplay(void) {
    // Register 0x15: Sprite and Layers System
    // Bits 4-2: Layer priority
    //   000 = S L U, 001 = L S U, 010 = S U L, 011 = L U S
    //   100 = U S L, 101 = U L S
    // We want ULA on top: U S L = 100
    // Bit 1: Sprites over border
    // Bit 0: Sprites visible
    IO_NEXTREG_REG = 0x15;
    IO_NEXTREG_DAT = 0x13;  // 0b00010011 = U S L order, sprites visible and over border

    // Register 0x14: Global Transparency Color
    // Set ULA paper black (0x00) as transparent color
    // This makes ULA black areas show-through to layers below
    IO_NEXTREG_REG = 0x14;
    IO_NEXTREG_DAT = 0x00;  // Black is transparent
}

// Set layer priority for menus (ULA only, no sprites)
void set_layers_menu(void) {
    // U L S order (101) - ULA on top
    // Bit 0 = 0 (sprites disabled)
    IO_NEXTREG_REG = 0x15;
    IO_NEXTREG_DAT = 0x14;  // 0b00010100 = U L S, no sprites
}
