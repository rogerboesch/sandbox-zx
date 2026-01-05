#ifndef SPRITEDEF_H
#define SPRITEDEF_H


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

#endif