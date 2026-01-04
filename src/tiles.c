/*
 * Dark Nebula - ZX Spectrum Next
 * tiles.c - Layer 2 moon background + Tilemap highway
 *
 * Layer 2: Moon surface with horizontal parallax
 * Tilemap: Highway (no horizontal parallax)
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

// Layer 2 port
#define LAYER2_PORT  0x123B

// Layer 2 colors (RGB332)
#define COL_BLACK       0x00
#define COL_MOON        0x49  // Dark grey
#define COL_MOON_LIGHT  0x6D  // Lighter grey
#define COL_CRATER      0x24  // Dark crater

// Tilemap registers
#define REG_TILEMAP_CTRL    0x6B
#define REG_TILEMAP_ATTR    0x6C
#define REG_TILEMAP_BASE    0x6E
#define REG_TILEMAP_TILES   0x6F
#define REG_TILEMAP_XSCROLL 0x2F
#define REG_TILEMAP_YSCROLL 0x31

// Tilemap memory: use 0x6000 to avoid ULA screen at 0x4000
// Tile definitions: use 0x6800
#define TILEMAP_BASE_ADDR   0x6000
#define TILES_BASE_ADDR     0x6800

// Tile indices
#define TILE_TRANS      0   // Transparent
#define TILE_HIGHWAY    1   // Grey highway
#define TILE_STRIPE     2   // Yellow stripe
#define TILE_DASH_ON    3   // White center dash
#define TILE_DASH_OFF   4   // Highway (no dash)

// Draw moon surface to one Layer 2 bank (NO highway - that's on tilemap)
static void layer2_draw_moon_bank(uint8_t bank) {
    uint8_t y;
    uint8_t *row;
    uint8_t world_y;

    // Enable Layer 2 write + visible, select bank
    z80_outp(LAYER2_PORT, 0x03 | (bank << 6));

    for (y = 0; y < 64; y++) {
        world_y = bank * 64 + y;
        row = (uint8_t *)(y * 256);

        // Moon left (0-95) with craters
        memset(row, COL_MOON, 96);
        if ((world_y % 32) < 8) {
            uint8_t crater_x = 20 + (world_y % 64);
            if (crater_x < 90) {
                row[crater_x] = COL_CRATER;
                row[crater_x + 1] = COL_CRATER;
            }
        }
        if ((world_y % 16) == 0) {
            row[50] = COL_MOON_LIGHT;
            row[70] = COL_MOON_LIGHT;
        }

        // Highway area (96-159) - BLACK (tilemap will overlay)
        memset(row + 96, COL_BLACK, 64);

        // Moon right (160-255) with craters
        memset(row + 160, COL_MOON, 96);
        if (((world_y + 16) % 32) < 8) {
            uint8_t crater_x = 180 + ((world_y * 3) % 50);
            if (crater_x < 250) {
                row[crater_x] = COL_CRATER;
                row[crater_x + 1] = COL_CRATER;
            }
        }
        if ((world_y % 24) == 0) {
            row[190] = COL_MOON_LIGHT;
            row[220] = COL_MOON_LIGHT;
        }
    }
}

// Initialize Layer 2 with moon surface only
void layer2_init_background(void) {
    layer2_draw_moon_bank(0);
    layer2_draw_moon_bank(1);
    layer2_draw_moon_bank(2);
    z80_outp(LAYER2_PORT, 0x02);  // Disable write, keep visible
}

// Draw full background to one Layer 2 bank
static void layer2_draw_full_bank(uint8_t bank) {
    uint8_t y;
    uint8_t *row;
    uint8_t world_y;

    z80_outp(LAYER2_PORT, 0x03 | (bank << 6));

    for (y = 0; y < 64; y++) {
        world_y = bank * 64 + y;
        row = (uint8_t *)(0x0000 + y * 256);

        // Moon left (0-95)
        memset(row, 0x49, 96);

        // Left yellow stripe
        memset(row + 96, 0xFC, 4);

        // Highway grey (100-155)
        memset(row + 100, 0xB6, 56);

        // Center dashed line
        if ((world_y % 8) < 4) {
            row[127] = 0xFF;
            row[128] = 0xFF;
        }

        // Right yellow stripe
        memset(row + 156, 0xFC, 4);

        // Moon right (160-255)
        memset(row + 160, 0x49, 96);
    }
}

// Initialize Layer 2 with full background (moon + highway)
void layer2_init_full_background(void) {
    layer2_draw_full_bank(0);
    layer2_draw_full_bank(1);
    layer2_draw_full_bank(2);
    z80_outp(LAYER2_PORT, 0x02);
}

// Define tile patterns (4-bit, 8x8 = 32 bytes each)
static void define_tiles(void) {
    uint8_t *tiles = (uint8_t *)TILES_BASE_ADDR;
    uint8_t row;

    // Tile 0: Transparent (all zeros = palette 0 = transparent)
    memset(tiles, 0x00, 32);
    tiles += 32;

    // Tile 1: Solid magenta (palette index 1)
    memset(tiles, 0x11, 32);
    tiles += 32;

    // Tile 2: Magenta with white top/bottom border
    for (row = 0; row < 8; row++) {
        if (row == 0 || row == 7) {
            // White border (palette index 2)
            memset(tiles, 0x22, 4);
        } else {
            // Magenta fill (palette index 1)
            memset(tiles, 0x11, 4);
        }
        tiles += 4;
    }

    // Tile 3: White center dash on magenta
    for (row = 0; row < 8; row++) {
        if (row >= 3 && row <= 4) {
            // White dash (palette index 2)
            memset(tiles, 0x22, 4);
        } else {
            // Magenta (palette index 1)
            memset(tiles, 0x11, 4);
        }
        tiles += 4;
    }
}

// Set up tilemap palette (first 16 colors of tilemap palette)
static void setup_tilemap_palette(void) {
    // Select tilemap palette (palette 1), auto-increment
    IO_NEXTREG_REG = 0x43;
    IO_NEXTREG_DAT = 0x30;  // Tilemap palette, auto-increment

    // Set palette index 0
    IO_NEXTREG_REG = 0x40;
    IO_NEXTREG_DAT = 0;

    // Write colors (RGB332 format via register 0x41)
    IO_NEXTREG_REG = 0x41;
    IO_NEXTREG_DAT = 0x00;  // 0: Transparent
    IO_NEXTREG_DAT = 0xE3;  // 1: Magenta
    IO_NEXTREG_DAT = 0xFF;  // 2: White
    IO_NEXTREG_DAT = 0xFC;  // 3: Yellow
}

// Tile indices
#define TILE_TRANS      0   // Transparent
#define TILE_MAGENTA    1   // Solid magenta
#define TILE_BORDER     2   // Magenta with border
#define TILE_DASH       3   // White dash on magenta

// Fill tilemap with highway pattern
static void fill_tilemap(void) {
    uint8_t *tmap = (uint8_t *)TILEMAP_BASE_ADDR;
    uint8_t x, y;

    // Tilemap is 40 columns x 32 rows (8x8 tiles)
    // Highway is at pixel 96-159 = tiles 12-19 (8 tiles wide)

    for (y = 0; y < 32; y++) {
        for (x = 0; x < 40; x++) {
            uint8_t tile;

            if (x < 12 || x >= 20) {
                // Outside highway - transparent
                tile = TILE_TRANS;
            } else if (x == 15 || x == 16) {
                // Center - dashed line pattern
                tile = ((y % 2) == 0) ? TILE_DASH : TILE_MAGENTA;
            } else {
                // Highway body - solid magenta
                tile = TILE_MAGENTA;
            }

            *tmap++ = tile;
        }
    }
}

// Initialize hardware tilemap for highway
void highway_init(void) {
    // Define tile patterns
    define_tiles();

    // Set up tilemap palette
    setup_tilemap_palette();

    // Fill tilemap data
    fill_tilemap();

    // Configure tilemap base address (bits 5:0 = A15:A10)
    // 0x6000 >> 10 = 0x18
    IO_NEXTREG_REG = REG_TILEMAP_BASE;
    IO_NEXTREG_DAT = 0x18;

    // Configure tile definitions address
    // 0x6800 >> 10 = 0x1A
    IO_NEXTREG_REG = REG_TILEMAP_TILES;
    IO_NEXTREG_DAT = 0x1A;

    // Set default attribute (no mirror/rotate, palette 0)
    IO_NEXTREG_REG = REG_TILEMAP_ATTR;
    IO_NEXTREG_DAT = 0x00;

    // Enable tilemap: 40x32, 4-bit tiles, UNDER ULA
    // Bit 7 = enable, Bit 0 = 4-bit mode
    // Bit 1 must be 0 (was forcing tilemap on top!)
    IO_NEXTREG_REG = REG_TILEMAP_CTRL;
    IO_NEXTREG_DAT = 0x81;  // Enable + 4-bit mode, tilemap under ULA

    // Set tilemap transparency index to 0 (palette index 0 = transparent)
    IO_NEXTREG_REG = 0x4C;
    IO_NEXTREG_DAT = 0x00;
}

// Scroll tilemap (highway) - vertical only, no horizontal parallax
void highway_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

// Scroll Layer 2 vertically
void layer2_scroll(int16_t offset_y) {
    IO_NEXTREG_REG = 0x17;
    IO_NEXTREG_DAT = (uint8_t)(offset_y & 0xFF);
}

// Scroll Layer 2 horizontally
void layer2_scroll_x(int16_t offset_x) {
    IO_NEXTREG_REG = 0x16;
    IO_NEXTREG_DAT = (uint8_t)(offset_x & 0xFF);
}

void tilemap_scroll(int16_t offset_y) {
    // This now scrolls the highway tilemap
    highway_scroll(offset_y);
}

void tiles_init(void) {
    // Initialize Layer 2 with background
    layer2_init_full_background();

    // Disable Layer 2 for title screen - ULA text will show
    layer2_disable();

    // Set border to black
    z80_outp(0xFE, 0x00);
}

// Enable Layer 2 and tilemap for gameplay
void layer2_enable(void) {
    // Enable Layer 2
    z80_outp(LAYER2_PORT, 0x02);

    // Sprites over Layer 2
    IO_NEXTREG_REG = 0x15;
    IO_NEXTREG_DAT = 0x03;
}

// Disable Layer 2 for menus
void layer2_disable(void) {
    z80_outp(LAYER2_PORT, 0x00);
    IO_NEXTREG_REG = 0x15;
    IO_NEXTREG_DAT = 0x01;
}

// Get current scroll Y for text position compensation
int16_t get_scroll_y(void) {
    return scroll_y;
}

void level_generate_row(uint8_t row) { (void)row; }
void tiles_upload_patterns(void) { }
void tilemap_init(void) { }

// Simple 4x6 font data for digits and letters (minimal set)
static const uint8_t font_data[40][6] = {
    // 0-9
    {0x6,0x9,0x9,0x9,0x6,0x0}, // 0
    {0x2,0x6,0x2,0x2,0x7,0x0}, // 1
    {0x6,0x9,0x2,0x4,0xF,0x0}, // 2
    {0xF,0x2,0x6,0x1,0xE,0x0}, // 3
    {0x9,0x9,0xF,0x1,0x1,0x0}, // 4
    {0xF,0x8,0xE,0x1,0xE,0x0}, // 5
    {0x6,0x8,0xE,0x9,0x6,0x0}, // 6
    {0xF,0x1,0x2,0x4,0x4,0x0}, // 7
    {0x6,0x9,0x6,0x9,0x6,0x0}, // 8
    {0x6,0x9,0x7,0x1,0x6,0x0}, // 9
    // A-Z (10-35)
    {0x6,0x9,0xF,0x9,0x9,0x0}, // A
    {0xE,0x9,0xE,0x9,0xE,0x0}, // B
    {0x7,0x8,0x8,0x8,0x7,0x0}, // C
    {0xE,0x9,0x9,0x9,0xE,0x0}, // D
    {0xF,0x8,0xE,0x8,0xF,0x0}, // E
    {0xF,0x8,0xE,0x8,0x8,0x0}, // F
    {0x7,0x8,0xB,0x9,0x6,0x0}, // G
    {0x9,0x9,0xF,0x9,0x9,0x0}, // H
    {0x7,0x2,0x2,0x2,0x7,0x0}, // I
    {0x1,0x1,0x1,0x9,0x6,0x0}, // J
    {0x9,0xA,0xC,0xA,0x9,0x0}, // K
    {0x8,0x8,0x8,0x8,0xF,0x0}, // L
    {0x9,0xF,0xF,0x9,0x9,0x0}, // M
    {0x9,0xD,0xB,0x9,0x9,0x0}, // N
    {0x6,0x9,0x9,0x9,0x6,0x0}, // O
    {0xE,0x9,0xE,0x8,0x8,0x0}, // P
    {0x6,0x9,0x9,0xA,0x5,0x0}, // Q
    {0xE,0x9,0xE,0xA,0x9,0x0}, // R
    {0x7,0x8,0x6,0x1,0xE,0x0}, // S
    {0x7,0x2,0x2,0x2,0x2,0x0}, // T
    {0x9,0x9,0x9,0x9,0x6,0x0}, // U
    {0x9,0x9,0x9,0x6,0x6,0x0}, // V
    {0x9,0x9,0xF,0xF,0x9,0x0}, // W
    {0x9,0x9,0x6,0x9,0x9,0x0}, // X
    {0x9,0x9,0x7,0x1,0x6,0x0}, // Y
    {0xF,0x1,0x2,0x4,0xF,0x0}, // Z
    // Special: 36=space, 37=colon, 38=!, 39=*
    {0x0,0x0,0x0,0x0,0x0,0x0}, // space
    {0x0,0x2,0x0,0x2,0x0,0x0}, // :
    {0x2,0x2,0x2,0x0,0x2,0x0}, // !
    {0xA,0x4,0xE,0x4,0xA,0x0}, // *
};

// Draw character to Layer 2 at pixel position
void layer2_draw_char(uint8_t px, uint8_t py, char c, uint8_t color) {
    uint8_t idx;
    uint8_t row, col;
    uint8_t bank;
    uint8_t *ptr;

    // Map character to font index
    if (c >= '0' && c <= '9') idx = c - '0';
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z') idx = c - 'a' + 10;
    else if (c == ' ') idx = 36;
    else if (c == ':') idx = 37;
    else if (c == '!') idx = 38;
    else if (c == '*') idx = 39;
    else return;

    // Draw 4x6 character
    for (row = 0; row < 6; row++) {
        uint8_t y = py + row;
        if (y >= 192) continue;

        bank = y / 64;
        z80_outp(LAYER2_PORT, 0x03 | (bank << 6));

        uint8_t local_y = y % 64;
        ptr = (uint8_t *)(local_y * 256 + px);

        uint8_t bits = font_data[idx][row];
        for (col = 0; col < 4; col++) {
            if (bits & (0x8 >> col)) {
                ptr[col] = color;
            }
        }
    }
    z80_outp(LAYER2_PORT, 0x02);  // Disable write
}

// Draw string to Layer 2
void layer2_draw_string(uint8_t px, uint8_t py, const char *str, uint8_t color) {
    while (*str) {
        layer2_draw_char(px, py, *str, color);
        px += 5;  // 4 pixels + 1 space
        str++;
    }
}

// Draw number to Layer 2
void layer2_draw_num(uint8_t px, uint8_t py, uint16_t num, uint8_t color) {
    char buf[6];
    uint8_t i = 0;

    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0 && i < 5) {
            buf[i++] = '0' + (num % 10);
            num /= 10;
        }
    }

    // Reverse and draw
    while (i > 0) {
        i--;
        layer2_draw_char(px, py, buf[i], color);
        px += 5;
    }
}
