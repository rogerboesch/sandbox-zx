/*
 * Dark Nebula - ZX Spectrum Next Prototype
 * sprites.c - Sprite handling and Layer 2 graphics
 */

#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include "game.h"

// ZX Spectrum Next I/O ports
#define LAYER2_ACCESS_PORT    0x123B
#define SPRITE_SLOT_PORT      0x303B
#define SPRITE_ATTR_PORT      0x57
#define SPRITE_PATTERN_PORT   0x5B

// Next register values (using literal values to avoid conflicts with z88dk)
#define NEXTREG_SPRITE_SYSTEM  0x15
#define NEXTREG_LAYER2_CTRL    0x70

// 16x16 sprite patterns (256 bytes each, 8-bit per pixel)
// Palette indices 0-15 are ZX Spectrum colors, 0xE3 is transparent
#define C_TRANS  0xE3  // Transparent (magenta)

// ZX Spectrum palette indices (0-15)
#define C_BLACK   0   // Black
#define C_BLUE    1   // Blue
#define C_RED     2   // Red
#define C_MAGENTA 3   // Magenta
#define C_GREEN   4   // Green
#define C_CYAN    5   // Cyan
#define C_YELLOW  6   // Yellow
#define C_WHITE   7   // White
#define C_BRIGHT_BLACK   8   // Bright Black (same as black)
#define C_BRIGHT_BLUE    9   // Bright Blue
#define C_BRIGHT_RED     10  // Bright Red
#define C_BRIGHT_MAGENTA 11  // Bright Magenta
#define C_BRIGHT_GREEN   12  // Bright Green
#define C_BRIGHT_CYAN    13  // Bright Cyan
#define C_BRIGHT_YELLOW  14  // Bright Yellow
#define C_BRIGHT_WHITE   15  // Bright White

// Player ship - facing upward (bright white with bright cyan cockpit)
// T=0xE3 (transparent), W=15 (bright white), C=13 (bright cyan)
#define T 0xE3
#define W 15
#define C 13
static const uint8_t sprite_player[256] = {
    T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,  // Row 0
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,  // Row 1
    T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,  // Row 2
    T,T,T,T,T,W,W,C,C,W,W,T,T,T,T,T,  // Row 3 - cyan cockpit
    T,T,T,T,W,W,W,C,C,W,W,W,T,T,T,T,  // Row 4
    T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,  // Row 5
    T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,  // Row 6
    T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,  // Row 7
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,  // Row 8
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,  // Row 9
    T,W,W,W,T,W,W,W,W,W,W,T,W,W,W,T,  // Row 10
    T,W,W,T,T,W,W,W,W,W,W,T,T,W,W,T,  // Row 11
    W,W,W,T,T,T,W,W,W,W,T,T,T,W,W,W,  // Row 12
    W,W,T,T,T,T,W,W,W,W,T,T,T,T,W,W,  // Row 13
    W,T,T,T,T,T,T,W,W,T,T,T,T,T,T,W,  // Row 14
    T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T   // Row 15
};
#undef T
#undef W
#undef C

// Bullet - vertical energy bolt (bright yellow/white)
// T=0xE3 (transparent), W=15 (bright white), Y=14 (bright yellow)
#define T 0xE3
#define W 15
#define Y 14
static const uint8_t sprite_bullet[256] = {
    T,T,T,T,T,T,T,Y,Y,T,T,T,T,T,T,T,  // Row 0
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,  // Row 1
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,  // Row 2
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,  // Row 3
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,  // Row 4
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,  // Row 5
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,  // Row 6
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,  // Row 7
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,  // Row 8
    T,T,T,T,T,T,T,Y,Y,T,T,T,T,T,T,T,  // Row 9
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 10
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 11
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 12
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 13
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 14
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T   // Row 15
};
#undef T
#undef W
#undef Y

// Enemy type 1 - alien fighter (bright red with bright yellow eyes)
// T=0xE3 (transparent), R=10 (bright red), Y=14 (bright yellow)
#define T 0xE3
#define R 10
#define Y 14
static const uint8_t sprite_enemy1[256] = {
    T,T,T,T,T,T,T,R,R,T,T,T,T,T,T,T,  // Row 0
    T,T,T,T,T,T,R,R,R,R,T,T,T,T,T,T,  // Row 1
    T,T,T,T,T,R,R,R,R,R,R,T,T,T,T,T,  // Row 2
    T,T,T,T,R,R,R,Y,Y,R,R,R,T,T,T,T,  // Row 3 - yellow eyes
    T,T,T,R,R,R,Y,Y,Y,Y,R,R,R,T,T,T,  // Row 4
    T,T,R,R,R,R,R,R,R,R,R,R,R,R,T,T,  // Row 5
    T,R,R,R,R,R,R,R,R,R,R,R,R,R,R,T,  // Row 6
    R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,  // Row 7
    R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,  // Row 8
    T,R,R,R,R,R,R,R,R,R,R,R,R,R,R,T,  // Row 9
    T,T,R,R,R,R,R,R,R,R,R,R,R,R,T,T,  // Row 10
    T,T,T,R,R,R,Y,Y,Y,Y,R,R,R,T,T,T,  // Row 11
    T,T,T,T,R,R,R,Y,Y,R,R,R,T,T,T,T,  // Row 12
    T,T,T,T,T,R,R,R,R,R,R,T,T,T,T,T,  // Row 13
    T,T,T,T,T,T,R,R,R,R,T,T,T,T,T,T,  // Row 14
    T,T,T,T,T,T,T,R,R,T,T,T,T,T,T,T   // Row 15
};
#undef T
#undef R
#undef Y

// Enemy type 2 - larger cruiser (bright green with bright yellow accents)
// T=0xE3 (transparent), G=12 (bright green), Y=14 (bright yellow)
#define T 0xE3
#define G 12
#define Y 14
static const uint8_t sprite_enemy2[256] = {
    T,T,T,T,G,G,G,G,G,G,G,G,T,T,T,T,  // Row 0
    T,T,T,G,G,G,G,G,G,G,G,G,G,T,T,T,  // Row 1
    T,T,G,G,G,G,G,G,G,G,G,G,G,G,T,T,  // Row 2
    T,G,G,G,G,G,G,Y,Y,G,G,G,G,G,G,T,  // Row 3 - yellow
    G,G,G,G,G,G,Y,Y,Y,Y,G,G,G,G,G,G,  // Row 4
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,  // Row 5
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,  // Row 6
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,  // Row 7
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,  // Row 8
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,  // Row 9
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,  // Row 10
    G,G,G,G,G,G,Y,Y,Y,Y,G,G,G,G,G,G,  // Row 11
    T,G,G,G,G,G,G,Y,Y,G,G,G,G,G,G,T,  // Row 12
    T,T,G,G,G,G,G,G,G,G,G,G,G,G,T,T,  // Row 13
    T,T,T,G,G,G,G,G,G,G,G,G,G,T,T,T,  // Row 14
    T,T,T,T,G,G,G,G,G,G,G,G,T,T,T,T   // Row 15
};
#undef T
#undef G
#undef Y

// Explosion animation frame (red/yellow/white) - ZX doesn't have orange
// T=0xE3 (transparent), W=15 (bright white), Y=14 (bright yellow), R=10 (bright red)
#define T 0xE3
#define W 15
#define Y 14
#define R 10
static const uint8_t sprite_explosion[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 0
    T,T,T,T,R,T,T,T,T,T,T,R,T,T,T,T,  // Row 1
    T,T,T,R,T,T,R,T,T,R,T,T,R,T,T,T,  // Row 2
    T,T,R,T,T,Y,Y,Y,Y,Y,Y,T,T,R,T,T,  // Row 3
    T,R,T,T,Y,W,W,W,W,W,W,Y,T,T,R,T,  // Row 4
    T,T,T,Y,W,W,W,W,W,W,W,W,Y,T,T,T,  // Row 5
    T,R,T,W,W,W,W,W,W,W,W,W,W,T,R,T,  // Row 6
    R,T,Y,W,W,W,W,W,W,W,W,W,W,Y,T,R,  // Row 7
    R,T,Y,W,W,W,W,W,W,W,W,W,W,Y,T,R,  // Row 8
    T,R,T,W,W,W,W,W,W,W,W,W,W,T,R,T,  // Row 9
    T,T,T,Y,W,W,W,W,W,W,W,W,Y,T,T,T,  // Row 10
    T,R,T,T,Y,W,W,W,W,W,W,Y,T,T,R,T,  // Row 11
    T,T,R,T,T,Y,Y,Y,Y,Y,Y,T,T,R,T,T,  // Row 12
    T,T,T,R,T,T,R,T,T,R,T,T,R,T,T,T,  // Row 13
    T,T,T,T,R,T,T,T,T,T,T,R,T,T,T,T,  // Row 14
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T   // Row 15
};
#undef T
#undef W
#undef Y
#undef R

// Highway tile - bright cyan fill with bright white border on left and top
// W=15 (bright white), C=13 (bright cyan)
#define W 15
#define C 13
static const uint8_t sprite_highway[256] = {
    W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,  // Row 0 - all white
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 1 - cyan
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 2
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 3
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 4
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 5
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 6
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 7
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 8
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 9
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 10
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 11
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 12
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 13
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,  // Row 14
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C   // Row 15
};
#undef W
#undef C

// Life indicator - small ship icon (bright white, 8x8 centered in 16x16)
// T=0xE3 (transparent), W=15 (bright white)
#define T 0xE3
#define W 15
static const uint8_t sprite_life[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 0
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 1
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 2
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 3
    T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,  // Row 4
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,  // Row 5
    T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,  // Row 6
    T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,  // Row 7
    T,T,T,W,T,W,W,W,W,W,W,T,W,T,T,T,  // Row 8
    T,T,W,T,T,T,W,W,W,W,T,T,T,W,T,T,  // Row 9
    T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,  // Row 10
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 11
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 12
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 13
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,  // Row 14
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T   // Row 15
};
#undef T
#undef W

// Digit sprites for score display (12x12 centered in 16x16, digits 0-9)
// T=0xE3 (transparent), W=15 (bright white)
#define T 0xE3
#define W 15
static const uint8_t sprite_digits[10][256] = {
    // Digit 0
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 1
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,W,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 2
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,W,T,T,T,T,T,
        T,T,T,T,T,T,T,W,W,W,T,T,T,T,T,T,
        T,T,T,T,T,T,W,W,W,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 3
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 4
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 5
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 6
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 7
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,W,T,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 8
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    },
    // Digit 9
    {
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,W,W,T,T,T,
        T,T,T,W,W,T,T,T,T,T,W,W,W,T,T,T,
        T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
        T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
    }
};
#undef T
#undef W

// Write to Next register
static void nextreg_write(uint8_t reg, uint8_t val) {
    IO_NEXTREG_REG = reg;
    IO_NEXTREG_DAT = val;
}

// Read from Next register
static uint8_t nextreg_read(uint8_t reg) {
    IO_NEXTREG_REG = reg;
    return IO_NEXTREG_DAT;
}

// ZX Spectrum palette colors in RGB332 format
static const uint8_t zx_palette[16] = {
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

// Set up sprite palette (ZX colors at 0-15, RGB332 identity for rest)
static void sprites_setup_palette(void) {
    uint16_t i;

    // Transparency index is default 0xE3 (magenta)

    // Select sprite palette for writing
    // Nextreg 0x43: bit 7=0 (auto-inc), bits 6-4=read palette, bits 3-1=write palette, bit 0=ULANext
    // Palette select: 000=ULA, 010=Layer2, 100=Sprites, 110=Tilemap
    // For sprites first palette: bits 3-1 = 100 -> 0b00001000 = 0x08
    nextreg_write(0x43, 0x08);

    // Start at palette index 0
    nextreg_write(0x40, 0);

    // Write ZX Spectrum colors to indices 0-15
    for (i = 0; i < 16; i++) {
        nextreg_write(0x41, zx_palette[i]);
    }

    // Write RGB332 identity for indices 16-255
    for (i = 16; i < 256; i++) {
        nextreg_write(0x41, (uint8_t)i);
    }

    // Reset palette control to ULA, ULANext disabled
    nextreg_write(0x43, 0x00);
}

// Initialize sprite system
void sprites_init(void) {
    // Enable sprites and set sprite priority over Layer 2
    nextreg_write(NEXTREG_SPRITE_SYSTEM, 0x03);  // Enable sprites, sprites over ULA

    // Set up sprite palette
    sprites_setup_palette();

    // Upload sprite patterns
    sprites_upload_patterns();
}

// Upload sprite patterns to pattern memory
// 8-bit mode: 16x16 = 256 bytes per pattern
void sprites_upload_patterns(void) {
    uint16_t i;
    uint8_t d;

    // Select sprite pattern slot 0
    z80_outp(SPRITE_SLOT_PORT, SPRITE_PLAYER);

    // Upload player sprite
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_player[i]);
    }

    // Upload bullet sprite (slot 1)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_bullet[i]);
    }

    // Upload enemy1 sprite (slot 2)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_enemy1[i]);
    }

    // Upload enemy2 sprite (slot 3)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_enemy2[i]);
    }

    // Upload explosion sprite (slot 4)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_explosion[i]);
    }

    // Upload life sprite (slot 5)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_life[i]);
    }

    // Upload digit sprites (slots 6-15 for digits 0-9)
    for (d = 0; d < 10; d++) {
        for (i = 0; i < 256; i++) {
            z80_outp(SPRITE_PATTERN_PORT, sprite_digits[d][i]);
        }
    }

    // Upload highway sprite (slot 16)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_highway[i]);
    }
}

// Set sprite attributes (8-bit mode)
void sprite_set(uint8_t slot, int16_t x, int16_t y, uint8_t pattern, uint8_t flags) {
    // Adjust coordinates for sprite offset (sprites are positioned from 32,32)
    x += 32;
    y += 32;

    // Select sprite attribute slot
    z80_outp(SPRITE_SLOT_PORT, slot);

    // Write attribute bytes (5 bytes for 8-bit sprites)
    // Byte 4: H=N6 for pattern, bits 6,5=00 for 8-bit mode
    z80_outp(SPRITE_ATTR_PORT, x & 0xFF);           // Byte 0: X low byte
    z80_outp(SPRITE_ATTR_PORT, y & 0xFF);           // Byte 1: Y low byte
    z80_outp(SPRITE_ATTR_PORT, (x >> 8) | flags);   // Byte 2: X MSB, palette offset, mirror, rotate
    z80_outp(SPRITE_ATTR_PORT, 0xC0 | (pattern & 0x3F));  // Byte 3: Visible + enable byte4 + pattern[5:0]
    z80_outp(SPRITE_ATTR_PORT, (pattern & 0x40) << 1);    // Byte 4: 8-bit mode (N6=0,T=0) + H from pattern bit 6
}

// Hide a sprite
void sprite_hide(uint8_t slot) {
    z80_outp(SPRITE_SLOT_PORT, slot);
    z80_outp(SPRITE_ATTR_PORT, 0);
    z80_outp(SPRITE_ATTR_PORT, 0);
    z80_outp(SPRITE_ATTR_PORT, 0);
    z80_outp(SPRITE_ATTR_PORT, 0);  // Invisible (bit 7 = 0)
    z80_outp(SPRITE_ATTR_PORT, 0);  // Byte 4 for 8-bit mode
}

// Layer 2 functions
// Enable Layer 2 and map it for writing
static void layer2_enable(void) {
    nextreg_write(NEXTREG_LAYER2_CTRL, 0x00);
    z80_outp(LAYER2_ACCESS_PORT, 0x03);  // Enable Layer 2 + write enable
}

// Clear Layer 2 with a color
void layer2_clear(uint8_t color) {
    uint8_t bank;
    uint16_t i;
    uint8_t *ptr;

    // Layer 2 is in banks 8, 9, 10 (256x192 @ 8bpp = 48KB)
    for (bank = 0; bank < 3; bank++) {
        // Map Layer 2 bank to slot 2 (0x4000-0x7FFF)
        z80_outp(LAYER2_ACCESS_PORT, 0x03 | (bank << 6));

        ptr = (uint8_t *)0x4000;
        for (i = 0; i < 16384; i++) {
            *ptr++ = color;
        }
    }

    // Reset to normal operation
    z80_outp(LAYER2_ACCESS_PORT, 0x02);
}

// Plot a pixel on Layer 2
void layer2_plot(uint8_t x, uint8_t y, uint8_t color) {
    uint8_t bank;
    uint8_t *ptr;

    if (y >= 192) return;

    // Determine which bank (each bank = 64 lines)
    bank = y / 64;

    // Map the appropriate Layer 2 bank
    z80_outp(LAYER2_ACCESS_PORT, 0x03 | (bank << 6));

    // Calculate address within the bank
    ptr = (uint8_t *)0x4000 + ((y % 64) * 256) + x;
    *ptr = color;
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
