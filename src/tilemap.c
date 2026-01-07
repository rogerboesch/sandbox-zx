#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include <string.h>
#include "tilemap.h"
#include "tileset.h"

// Tile indices for tilemap
#define TILE_ROAD_LEFT    0x00  // left border (G6)
#define TILE_ROAD_MID     0x01  // middle (L6)
#define TILE_ROAD_RIGHT   0x02  // right border (H6)
#define TILE_TRANS        0x03  // transparent/empty
#define TILE_HOLE_TL      0x04  // hole top-left (K0)
#define TILE_HOLE_TR      0x05  // hole top-right (L0)
#define TILE_HOLE_BL      0x06  // hole bottom-left (K1)
#define TILE_HOLE_BR      0x07  // hole bottom-right (L1)

// Transparent tile (bright magenta = palette index 11 = 0xBB per byte)
static const uint8_t tile_transparent[32] = {
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB
};

// Tilemap tiles array (8 tiles for road + holes)
static const uint8_t * const tilemap_tiles[8] = {
    tile_G6,           // 0: TILE_ROAD_LEFT
    tile_L6,           // 1: TILE_ROAD_MID
    tile_H6,           // 2: TILE_ROAD_RIGHT
    tile_transparent,  // 3: TILE_TRANS
    tile_K0,           // 4: TILE_HOLE_TL
    tile_L0,           // 5: TILE_HOLE_TR
    tile_K1,           // 6: TILE_HOLE_BL
    tile_L1            // 7: TILE_HOLE_BR
};

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
// Max tiles: (0x7FFF - 0x6600) / 32 = 208 tiles
#define TILEMAP_ADDR    0x6000
#define TILES_ADDR      0x6600
#define MAX_TILES       8    // 8 tiles (8 * 32 = 256 bytes)

// Scroll state
int16_t scroll_y = 0;

// Copy tile definitions from ROM to tilemap memory
static void tilemap_define_tiles(void) {
    uint8_t *dest = (uint8_t *)TILES_ADDR;
    uint8_t i;

    for (i = 0; i < MAX_TILES; i++) {
        memcpy(dest, tilemap_tiles[i], TILE_SIZE);
        dest += TILE_SIZE;
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

// Fill tilemap in center with left/middle/right borders
// Left border: cyan line on left, Middle: solid black, Right border: cyan line on right
// Sporadic holes (16px wide) on left or right side that player must avoid
static void tilemap_fill(void) {
    uint8_t *tmap = (uint8_t *)TILEMAP_ADDR;
    uint8_t x, y;
    uint8_t hole_side;  // 0=no hole, 1=left hole, 2=right hole
    uint8_t hole_active;

    // Level spans tiles 16-23 (8 tiles wide)
    // x=16: left border, x=17-19: left half, x=20-22: right half, x=23: right border
    for (y = 0; y < 32; y++) {
        // Explicit hole positions to ensure safe start and both sides
        // Player starts at tilemap row ~25 (due to +32 sprite offset)
        // So avoid rows 20-31, place holes at rows 4-5 (right) and 12-13 (left)
        hole_active = 0;
        hole_side = 0;

        if (y == 4 || y == 5) {
            hole_active = 1;
            hole_side = 2;  // Right side
        }
        else if (y == 12 || y == 13) {
            hole_active = 1;
            hole_side = 1;  // Left side
        }

        for (x = 0; x < 40; x++) {
            uint8_t tile;
            uint8_t is_hole = 0;
            uint8_t hole_row_top = 0;  // 1 if top row of 2x2 hole block

            // Check if this tile should be a hole (half level width)
            if (hole_active) {
                if (hole_side == 1 && x >= 16 && x <= 19) {
                    is_hole = 1;  // Left half hole (32px)
                } else if (hole_side == 2 && x >= 20 && x <= 23) {
                    is_hole = 1;  // Right half hole (32px)
                }
                // Top row of hole block (y=4 or y=12)
                hole_row_top = (y == 4 || y == 12);
            }

            if (is_hole) {
                // 2x2 hole pattern: K0,L0 / K1,L1
                uint8_t is_left_of_pair = (x & 1) == 0;  // Even x = left tile
                if (hole_row_top) {
                    tile = is_left_of_pair ? TILE_HOLE_TL : TILE_HOLE_TR;
                }
                else {
                    tile = is_left_of_pair ? TILE_HOLE_BL : TILE_HOLE_BR;
                }
            } else if (x == 16) {
                // Left border
                tile = TILE_ROAD_LEFT;
            } else if (x >= 17 && x <= 22) {
                // Middle
                tile = TILE_ROAD_MID;
            } else if (x == 23) {
                // Right border
                tile = TILE_ROAD_RIGHT;
            }
            else {
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

    // Set tilemap transparency: palette index 11 (bright magenta) is transparent
    // Register 0x4C controls tilemap transparency
    // Bits 3:0 = transparent palette index
    // When a pixel has this palette index, it shows through to layer below
    ZXN_NEXTREG(REG_TILEMAP_TRANS, 0x0B);  // 11 = bright magenta

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

// Scroll tilemap vertically (level - full speed)
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
