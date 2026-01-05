#ifndef TILES_H
#define TILES_H

#include <stdint.h>

// Tile definitions for 4-bit 8x8 tiles (32 bytes each)
// Each byte = 2 pixels (4 bits per pixel)
// Palette: 0=transparent, 5=cyan, 7=white, 8=black (non-transparent)

// Tile indices
#define TILE_TRANS       0
#define TILE_ROAD        1   // Black road (solid)
#define TILE_DASH        2   // Road with white center dash
#define TILE_BORDER_L    3   // Left edge with cyan border
#define TILE_BORDER_R    4   // Right edge with cyan border
#define TILE_BORDER_L_D  5   // Left edge with dash
#define TILE_BORDER_R_D  6   // Right edge with dash

#define NUM_TILES        7

// Palette index for non-transparent black
#define PAL_BLACK  8u

// Shorthand macros for readability
#define __ 0x00  // Transparent pair
#define BB 0x88  // Black pair (8<<4 | 8)
#define WW 0x77  // White pair (7<<4 | 7)
#define CB 0x58  // Cyan + Black (5<<4 | 8)
#define BC 0x85  // Black + Cyan (8<<4 | 5)

// Tile 0: Fully transparent
static const uint8_t tile_trans[32] = {
    __,__,__,__,
    __,__,__,__,
    __,__,__,__,
    __,__,__,__,
    __,__,__,__,
    __,__,__,__,
    __,__,__,__,
    __,__,__,__
};

// Tile 1: Solid black road
static const uint8_t tile_road[32] = {
    BB,BB,BB,BB,
    BB,BB,BB,BB,
    BB,BB,BB,BB,
    BB,BB,BB,BB,
    BB,BB,BB,BB,
    BB,BB,BB,BB,
    BB,BB,BB,BB,
    BB,BB,BB,BB
};

// Tile 2: Black road with white center dash (rows 3-4)
static const uint8_t tile_dash[32] = {
    BB,BB,BB,BB,  // row 0
    BB,BB,BB,BB,  // row 1
    BB,BB,BB,BB,  // row 2
    WW,WW,WW,WW,  // row 3 - white dash
    WW,WW,WW,WW,  // row 4 - white dash
    BB,BB,BB,BB,  // row 5
    BB,BB,BB,BB,  // row 6
    BB,BB,BB,BB   // row 7
};

// Tile 3: Left border (cyan on left edge, black rest)
static const uint8_t tile_border_l[32] = {
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB
};

// Tile 4: Right border (black, cyan on right edge)
static const uint8_t tile_border_r[32] = {
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC
};

// Tile 5: Left border with dash (same as tile 3 - no dash on edges)
static const uint8_t tile_border_l_d[32] = {
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB,
    CB,BB,BB,BB
};

// Tile 6: Right border with dash (same as tile 4 - no dash on edges)
static const uint8_t tile_border_r_d[32] = {
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC,
    BB,BB,BB,BC
};

#undef __
#undef BB
#undef WW
#undef CB
#undef BC

// Array of pointers to all tiles for easy iteration
static const uint8_t * const tile_defs[NUM_TILES] = {
    tile_trans,
    tile_road,
    tile_dash,
    tile_border_l,
    tile_border_r,
    tile_border_l_d,
    tile_border_r_d
};

#endif
