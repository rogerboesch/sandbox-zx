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
#define MY_REG_MMU6          0x56
#define MY_REG_MMU7          0x57

// Use bank 10 for tilemap data, mapped to 0xC000-0xDFFF via MMU slot 6
// Tilemap at offset 0 (0xC000 when mapped), tiles at 0x600 (0xC600 when mapped)
#define TILEMAP_BANK    10
#define TILEMAP_OFFSET  0xC000
#define TILES_OFFSET    0xC600

// Tile indices
#define TILE_TRANS    0
#define TILE_MAGENTA  1
#define TILE_DASH     2

// Scroll state
int16_t scroll_y = 0;

// Define 8x8 tile patterns (4-bit = 32 bytes each)
// Each byte = 2 pixels, X-major order
static void tilemap_define_tiles(uint8_t *tiles) {
    uint8_t i;

    // Tile 0: Transparent (palette index 0)
    for (i = 0; i < 32; i++) {
        tiles[i] = 0x00;  // Two transparent pixels
    }
    tiles += 32;

    // Tile 1: Solid color (palette index 3 = magenta)
    for (i = 0; i < 32; i++) {
        tiles[i] = 0x33;  // Two magenta pixels
    }
    tiles += 32;

    // Tile 2: White dash on magenta (for center line)
    for (i = 0; i < 32; i++) {
        // Rows 3-4 are white (palette 7), rest magenta (palette 3)
        uint8_t row = i / 4;  // 4 bytes per row (8 pixels / 2)
        if (row >= 3 && row <= 4) {
            tiles[i] = 0x77;  // Two white pixels
        } else {
            tiles[i] = 0x33;  // Two magenta pixels
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
static void tilemap_fill(uint8_t *tmap) {
    uint8_t x, y;

    // 40 columns x 32 rows (8x8 tiles = 320x256 pixels, but only 256x192 visible)
    // Highway at pixels 96-159 = tiles 12-19 (8x8 tiles, so 96/8=12, 159/8=19)
    for (y = 0; y < 32; y++) {
        for (x = 0; x < 40; x++) {
            uint8_t tile;

            if (x >= 12 && x <= 19) {
                // Highway area
                if ((x == 15 || x == 16) && (y % 4 == 0)) {
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
    uint8_t old_bank6;

    // Save old MMU6 bank
    IO_NEXTREG_REG = MY_REG_MMU6;
    old_bank6 = IO_NEXTREG_DAT;

    // Map bank 10 to slot 6 (0xC000-0xDFFF)
    IO_NEXTREG_REG = MY_REG_MMU6;
    IO_NEXTREG_DAT = TILEMAP_BANK;

    // Define tile patterns
    tilemap_define_tiles((uint8_t *)TILES_OFFSET);

    // Fill tilemap data
    tilemap_fill((uint8_t *)TILEMAP_OFFSET);

    // Restore old MMU6 bank
    IO_NEXTREG_REG = MY_REG_MMU6;
    IO_NEXTREG_DAT = old_bank6;

    // Set up palette
    tilemap_setup_palette();

    // Set tilemap base address
    // Bank 10 = 8K bank, physical address = 10 * 0x2000 = 0x14000
    // Register 0x6E: bits 5:0 = A13-A18 of tilemap address
    // 0x14000 = 0b0001_0100_0000_0000_0000
    // A13-A18 = bits 13-18 = 0b001010 = 10 (the bank number!)
    // But we also need to account for offset within bank
    // Tilemap is at start of bank, so just the bank * 2
    IO_NEXTREG_REG = REG_TILEMAP_BASE;
    IO_NEXTREG_DAT = TILEMAP_BANK * 2;  // 10 * 2 = 20 = 0x14

    // Set tile definitions address
    // Tiles at 0x14600 (bank 10 + 0x600 offset)
    // A13-A18 of 0x14600 = same calculation, 0x14600 >> 13 = 10, plus offset
    IO_NEXTREG_REG = REG_TILEMAP_TILES;
    IO_NEXTREG_DAT = (TILEMAP_BANK * 2) + (6 >> 5);  // ~0x14

    // Transparency color register
    IO_NEXTREG_REG = REG_TILEMAP_TRANS;
    IO_NEXTREG_DAT = 0x00;

    // Default attribute (palette offset 0, no mirror/rotate)
    IO_NEXTREG_REG = REG_TILEMAP_ATTR;
    IO_NEXTREG_DAT = 0x00;

    // Tilemap starts disabled
}

// Enable tilemap display
void tilemap_enable(void) {
    // DISABLED FOR DEBUGGING - tilemap not working correctly
    // IO_NEXTREG_REG = REG_TILEMAP_CTRL;
    // IO_NEXTREG_DAT = 0xA0;
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
