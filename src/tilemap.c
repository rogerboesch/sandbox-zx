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

// Use upper bank 5 area for tilemap (after ULA attributes at 0x5B00)
// Tilemap data at 0x6000 (40x32 = 1280 bytes with 8-bit entries)
// Tile definitions at 0x6600 (need 32 bytes per tile for 4-bit 8x8)
#define TILEMAP_ADDR    0x6000
#define TILES_ADDR      0x6600

// Tile indices
#define TILE_TRANS       0
#define TILE_ROAD        1   // Black road (solid)
#define TILE_DASH        2   // Road with white center dash
#define TILE_BORDER_L    3   // Left edge with cyan border
#define TILE_BORDER_R    4   // Right edge with cyan border
#define TILE_BORDER_L_D  5   // Left edge with dash
#define TILE_BORDER_R_D  6   // Right edge with dash

// Palette index for non-transparent black
#define PAL_BLACK  8

// Scroll state
int16_t scroll_y = 0;

// Define 8x8 tile patterns (4-bit = 32 bytes each)
// Each byte = 2 pixels, X-major order
// Palette: 0=transparent, 5=cyan, 7=white, 8=black (non-transparent)
static void tilemap_define_tiles(void) {
    uint8_t *tiles = (uint8_t *)TILES_ADDR;
    uint8_t i, row, col;

    // Tile 0: Transparent (palette index 0)
    for (i = 0; i < 32; i++) {
        tiles[i] = 0x00;
    }
    tiles += 32;

    // Tile 1: Solid black road
    for (i = 0; i < 32; i++) {
        tiles[i] = (PAL_BLACK << 4) | PAL_BLACK;
    }
    tiles += 32;

    // Tile 2: Black road with white center dash (rows 3-4)
    for (row = 0; row < 8; row++) {
        uint8_t color = (row >= 3 && row <= 4) ? 7 : PAL_BLACK;
        for (col = 0; col < 4; col++) {
            tiles[row * 4 + col] = (color << 4) | color;
        }
    }
    tiles += 32;

    // Tile 3: Left border (cyan on left edge, black rest)
    for (row = 0; row < 8; row++) {
        // First byte: cyan + black, rest: black + black
        tiles[row * 4 + 0] = (5 << 4) | PAL_BLACK;
        tiles[row * 4 + 1] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 2] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 3] = (PAL_BLACK << 4) | PAL_BLACK;
    }
    tiles += 32;

    // Tile 4: Right border (black, cyan on right edge)
    for (row = 0; row < 8; row++) {
        tiles[row * 4 + 0] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 1] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 2] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 3] = (PAL_BLACK << 4) | 5;
    }
    tiles += 32;

    // Tile 5: Left border (same as tile 3, no dash on edges)
    for (row = 0; row < 8; row++) {
        tiles[row * 4 + 0] = (5 << 4) | PAL_BLACK;
        tiles[row * 4 + 1] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 2] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 3] = (PAL_BLACK << 4) | PAL_BLACK;
    }
    tiles += 32;

    // Tile 6: Right border (same as tile 4, no dash on edges)
    for (row = 0; row < 8; row++) {
        tiles[row * 4 + 0] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 1] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 2] = (PAL_BLACK << 4) | PAL_BLACK;
        tiles[row * 4 + 3] = (PAL_BLACK << 4) | 5;
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
    ZXN_NEXTREG(0x43, 0x30);

    // Start at index 0
    ZXN_NEXTREG(0x40, 0);

    // Write ZX Spectrum colors to indices 0-15
    // Index 0 will be our transparent color (set to any value, transparency is via reg 0x4C)
    IO_NEXTREG_REG = 0x41;
    for (i = 0; i < 16; i++) {
        IO_NEXTREG_DAT = zx_colors[i];
    }

    // Reset palette control to ULA
    ZXN_NEXTREG(0x43, 0x00);
}

// Fill tilemap - highway in center, transparent elsewhere
static void tilemap_fill(void) {
    uint8_t *tmap = (uint8_t *)TILEMAP_ADDR;
    uint8_t x, y;
    uint8_t has_dash;

    // Highway spans tiles 16-23 (8 tiles wide, centered)
    // Left border at tile 16, right border at tile 23
    // Center dashes at tiles 19-20
    for (y = 0; y < 32; y++) {
        has_dash = (y % 4 == 0);  // Dash every 4 rows

        for (x = 0; x < 40; x++) {
            uint8_t tile;

            if (x == 16) {
                // Left edge of highway
                tile = has_dash ? TILE_BORDER_L_D : TILE_BORDER_L;
            } else if (x == 23) {
                // Right edge of highway
                tile = has_dash ? TILE_BORDER_R_D : TILE_BORDER_R;
            } else if (x > 16 && x < 23) {
                // Inside highway
                if ((x == 19 || x == 20) && has_dash) {
                    tile = TILE_DASH;
                } else {
                    tile = TILE_ROAD;
                }
            } else {
                tile = TILE_TRANS;
            }
            *tmap++ = tile;
        }
    }
}

// Initialize tilemap
void tilemap_init(void) {
    // Define tile patterns at 0x6600
    tilemap_define_tiles();

    // Fill tilemap data at 0x6000
    tilemap_fill();

    // Set up palette
    tilemap_setup_palette();

    // Set tilemap base address
    // Register expects MSB of offset from 0x4000 (bank 5 start)
    // For address 0x6000: offset = 0x6000 - 0x4000 = 0x2000, MSB = 0x20
    ZXN_NEXTREG(REG_TILEMAP_BASE, 0x20);

    // Set tile definitions address
    // For address 0x6600: offset = 0x6600 - 0x4000 = 0x2600, MSB = 0x26
    ZXN_NEXTREG(REG_TILEMAP_TILES, 0x26);

    // Default attribute (palette offset 0, no mirror/rotate)
    ZXN_NEXTREG(REG_TILEMAP_ATTR, 0x00);

    // Set tilemap transparency: palette index 0 is transparent
    // Register 0x4C controls tilemap transparency
    // Bits 3:0 = transparent palette index (0 = index 0 is transparent)
    // When a pixel has this palette index, it shows through to layer below
    ZXN_NEXTREG(REG_TILEMAP_TRANS, 0x00);

    // Tilemap starts disabled
}

// Enable tilemap display
void tilemap_enable(void) {
    // Reg 0x6B Tilemap Control:
    // Bit 7: Enable tilemap (1)
    // Bit 6: 0=40x32, 1=80x32 (0)
    // Bit 5: 1=8-bit tilemap entries (1)
    // Bit 4: Palette select (0 = first palette)
    // Bit 3: Text mode (0 = tile mode)
    // Bit 2: Reserved (0)
    // Bit 1: 512 tile mode (0)
    // Bit 0: Tilemap over ULA (0 = tilemap under ULA)
    ZXN_NEXTREG(REG_TILEMAP_CTRL, 0xA0);  // Enable, 40x32, 8-bit entries

    // Reg 0x6C Default Tilemap Attribute:
    // For 8-bit entries, this provides palette offset and flags
    // Bit 4: X mirror, Bit 3: Y mirror, Bit 2: Rotate, Bits 7-5,1-0: palette offset
    // We want no transformations and palette offset 0
    ZXN_NEXTREG(REG_TILEMAP_ATTR, 0x00);
}

// Disable tilemap display
void tilemap_disable(void) {
    ZXN_NEXTREG(REG_TILEMAP_CTRL, 0x00);
}

// Scroll tilemap vertically (highway - full speed)
void tilemap_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

// Set layer priority for gameplay
// From top to bottom: ULA > Sprites > Tilemap > Layer2
void set_layers_gameplay(void) {
    // Register 0x15: Sprite and Layers System
    // Bits 4-2: Layer priority (S=Sprites, L=Layer2, U=ULA+Tilemap)
    //   000 = S L U, 001 = L S U, 010 = S U L, 011 = L U S
    //   100 = U S L, 101 = U L S
    // Bit 7: Enable lo-res mode (0)
    // Bit 6: Sprite priority (0 = sprite 127 on top)
    // Bit 5: Enable sprite clipping in over-border mode (0)
    // Bit 1: Sprites over border
    // Bit 0: Sprites visible
    //
    // We want: Sprites above Tilemap, Layer 2 at bottom
    // Use S U L = 010 which gives: Sprites on top, then ULA/Tilemap, then Layer2
    // 010 in bits 4:2 = 0b00001000 = 0x08
    // Add sprites visible (0x01) and over border (0x02) = 0x0B
    ZXN_NEXTREG(0x15, 0x0B);  // 0b00001011 = S U L order, sprites visible and over border

    // Register 0x14: Global Transparency Color
    // Set ULA paper black (0x00) as transparent color
    // This makes ULA black areas show-through to layers below
    ZXN_NEXTREG(0x14, 0x00);  // Black is transparent

    // Tilemap is under ULA (reg 0x6B bit 0 = 0)
    // So final order: Sprites > ULA > Tilemap > Layer2
}

// Set layer priority for menus (ULA only, no sprites)
void set_layers_menu(void) {
    // U L S order (101) - ULA on top
    // Bit 0 = 0 (sprites disabled)
    ZXN_NEXTREG(0x15, 0x14);  // 0b00010100 = U L S, no sprites
}
