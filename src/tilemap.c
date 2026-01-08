#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include <string.h>
#include "tilemap.h"
#include "tileset.h"
#include "level.h"

// Tile indices for tilemap
#define TILE_ROAD_LEFT    0x00  // left border (G6)
#define TILE_ROAD_MID_TL  0x01  // highway middle top-left (E0)
#define TILE_ROAD_MID_TR  0x02  // highway middle top-right (F0)
#define TILE_ROAD_MID_BL  0x03  // highway middle bottom-left (E1)
#define TILE_ROAD_MID_BR  0x04  // highway middle bottom-right (F1)
#define TILE_ROAD_RIGHT   0x05  // right border (H6)
#define TILE_TRANS        0x06  // transparent/empty
#define TILE_HOLE_TL      0x07  // hole top-left (K0)
#define TILE_HOLE_TR      0x08  // hole top-right (L0)
#define TILE_HOLE_BL      0x09  // hole bottom-left (K1)
#define TILE_HOLE_BR      0x0A  // hole bottom-right (L1)
#define TILE_LANE_MARK    0x0B  // lane marker (I4)
#define TILE_LANE_EDGE    0x0C  // lane start/end marker (J4)

// Transparent tile (bright magenta = palette index 11 = 0xBB per byte)
static const uint8_t tile_transparent[32] = {
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB,
    0xBB, 0xBB, 0xBB, 0xBB,  0xBB, 0xBB, 0xBB, 0xBB
};

// Tilemap tiles array (13 tiles for road + holes + lane markers)
static const uint8_t * const tilemap_tiles[13] = {
    tile_G6,           // 0: TILE_ROAD_LEFT
    tile_E0,           // 1: TILE_ROAD_MID_TL
    tile_F0,           // 2: TILE_ROAD_MID_TR
    tile_E1,           // 3: TILE_ROAD_MID_BL
    tile_F1,           // 4: TILE_ROAD_MID_BR
    tile_H6,           // 5: TILE_ROAD_RIGHT
    tile_transparent,  // 6: TILE_TRANS
    tile_K0,           // 7: TILE_HOLE_TL
    tile_L0,           // 8: TILE_HOLE_TR
    tile_K1,           // 9: TILE_HOLE_BL
    tile_L1,           // 10: TILE_HOLE_BR
    tile_I4,           // 11: TILE_LANE_MARK
    tile_J4            // 12: TILE_LANE_EDGE
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
#define MAX_TILES       13   // 13 tiles (13 * 32 = 416 bytes)
#define TILEMAP_WIDTH   40   // 40 tiles wide

// Scroll state
int16_t scroll_y = 0;

// Last generated row (to avoid regenerating same rows)
static int16_t last_generated_scroll = 0;

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

// Tilemap 40x32 is 320x256 pixels, but ULA screen is 256x192
// From testing: row 0 is just ABOVE the visible ULA area
// Row 1 is at the TOP of visible ULA screen when scroll_y = 0
//
// scroll_y is 0 or negative (0, -1, -2, ... -255, wraps)
// scroll_y-- makes content move DOWN on screen, new content appears at TOP
//
// Row at top of tilemap = ((256 + scroll_y) / 8) & 0x1F
// At scroll_y = 0: row 0 at tilemap top (row 1 at visible top)
// At scroll_y = -8: row 31 at tilemap top, row 0 moved down 8px

// Calculate world_y for a tilemap row given current scroll
// row: tilemap row index (0-31)
// returns: world position in pixels that this row should display
static int16_t calc_world_y_for_row(uint8_t row) {
    // Row at top of tilemap (may be in buffer above visible)
    uint8_t top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);

    // How many rows is this row below the top row (wrapping at 32)
    uint8_t rows_from_top = (row - top_row) & 0x1F;

    // world_y for top row = -scroll_y (scroll amount in pixels)
    // Each row below adds 8 pixels
    int16_t world_y = (-scroll_y) + (rows_from_top * 8);

    return world_y;
}

// Generate a single tilemap row using level data
static void tilemap_generate_row(uint8_t row) {
    uint8_t *tmap = (uint8_t *)(TILEMAP_ADDR + row * TILEMAP_WIDTH);
    uint8_t tiles[TILEMAP_WIDTH];
    uint8_t x;
    int16_t world_y;

    // Calculate world position for this row
    world_y = calc_world_y_for_row(row);

    // Get tiles from level system
    level_generate_row(row, world_y, tiles);

    // Copy to tilemap memory
    for (x = 0; x < TILEMAP_WIDTH; x++) {
        tmap[x] = tiles[x];
    }
}

// Fill entire tilemap using level data
static void tilemap_fill_from_level(void) {
    uint8_t y;

    for (y = 0; y < 32; y++) {
        tilemap_generate_row(y);
    }
}

// Public function to refresh tilemap (call after level_init)
void tilemap_refresh(void) {
    tilemap_fill_from_level();
    last_generated_scroll = scroll_y;
}

// Initialize tilemap hardware (call before level_init)
void tilemap_init(void) {
    // Define tile patterns at 0x6600
    tilemap_define_tiles();

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

    // Reset scroll tracking
    last_generated_scroll = 0;

    // NOTE: tilemap content is filled by tilemap_refresh() after level_init()
    // Tilemap starts disabled
}

// Enable tilemap display
void tilemap_enable(void) {
    // Set tilemap clip window to ULA visible area + half border
    // Register 0x1B: X1, X2, Y1, Y2 (written sequentially)
    // Vertical: skip 16px (2 tiles) at top and bottom to show half border
    ZXN_NEXTREG(0x1B, 0);     // X1 - start from left
    ZXN_NEXTREG(0x1B, 255);   // X2 - to right edge
    ZXN_NEXTREG(0x1B, 16);    // Y1 - skip top 2 rows (16 pixels)
    ZXN_NEXTREG(0x1B, 239);   // Y2 - show up to 2 rows in bottom border

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
// Also updates tilemap rows when new rows scroll into view
void tilemap_scroll(int16_t offset_y) {
    int16_t scroll_diff;
    uint8_t row_diff;
    uint8_t row;
    uint8_t old_top_row, new_top_row;

    // Calculate row at top BEFORE updating scroll
    old_top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);

    // Update hardware scroll register
    IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);

    // Store current scroll
    scroll_y = offset_y;

    // Calculate row at top AFTER updating scroll
    new_top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);

    // Check if we need to regenerate rows
    // scroll_y-- means content moves DOWN, new content appears at TOP
    // When offset_y decreases (goes more negative), new rows appear at top

    scroll_diff = last_generated_scroll - offset_y;

    // Only handle forward scrolling (scroll_diff > 0 means offset_y decreased)
    if (scroll_diff <= 0) {
        return;
    }

    row_diff = (uint8_t)(scroll_diff / 8);

    if (row_diff > 0) {
        if (row_diff >= 32) {
            // Major scroll - regenerate all
            tilemap_fill_from_level();
        }
        else {
            // Regenerate rows that are now at the top (scrolled into view)
            // new_top_row is the row now at the top of the tilemap
            // We need to regenerate row_diff rows starting from new_top_row
            for (row = 0; row < row_diff && row < 32; row++) {
                // new_top_row is first, then new_top_row+1, etc (going down)
                uint8_t gen_row = (new_top_row + row) & 0x1F;
                tilemap_generate_row(gen_row);
            }
        }

        last_generated_scroll = offset_y;
    }
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
