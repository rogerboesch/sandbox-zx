#ifndef SPRITEDEF_H
#define SPRITEDEF_H

// Player ship - facing upward (bright white with bright cyan cockpit)
// T=0xE3 (transparent), W=15 (bright white), C=13 (bright cyan)
#define T 0xE3
#define W 15
#define C 13
static const uint8_t sprite_player[256] = {
    T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,W,W,W,W,W,W,T,T,T,T,T,
    T,T,T,T,T,W,W,C,C,W,W,T,T,T,T,T,
    T,T,T,T,W,W,W,C,C,W,W,W,T,T,T,T,
    T,T,T,T,W,W,W,W,W,W,W,W,T,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,T,W,W,W,W,W,W,W,W,W,W,T,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,T,W,W,W,W,W,W,W,W,W,W,W,W,T,T,
    T,W,W,W,T,W,W,W,W,W,W,T,W,W,W,T,
    T,W,W,T,T,W,W,W,W,W,W,T,T,W,W,T,
    W,W,W,T,T,T,W,W,W,W,T,T,T,W,W,W,
    W,W,T,T,T,T,W,W,W,W,T,T,T,T,W,W,
    W,T,T,T,T,T,T,W,W,T,T,T,T,T,T,W,
    T,T,T,T,T,T,T,W,W,T,T,T,T,T,T,T
};
#undef T
#undef W
#undef C

// Bullet - vertical energy bolt (bright yellow/white)
#define T 0xE3
#define W 15
#define Y 14
static const uint8_t sprite_bullet[256] = {
    T,T,T,T,T,T,T,Y,Y,T,T,T,T,T,T,T,
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,W,W,W,W,T,T,T,T,T,T,
    T,T,T,T,T,T,Y,W,W,Y,T,T,T,T,T,T,
    T,T,T,T,T,T,T,Y,Y,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
};
#undef T
#undef W
#undef Y

// Enemy type 1 - from tilemap 2x2 block (7,7)-(8,8) in bright red
// B=0 (black background), R=10 (bright red)
#define B 0
#define R 10
#define T 0xE3
static const uint8_t sprite_enemy1[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,R,R,R,R,T,T,T,T,T,T,
    T,T,T,T,R,R,B,B,B,B,R,R,T,T,T,T,
    T,T,T,R,B,B,B,B,R,B,B,B,R,T,T,T,
    T,T,R,B,B,R,R,B,B,B,R,B,B,R,T,T,
    T,T,R,B,R,R,R,R,B,B,R,R,B,R,T,T,
    T,R,B,B,B,B,R,R,B,R,R,R,B,B,R,T,
    T,R,B,R,B,B,B,R,R,R,R,B,B,B,R,T,
    T,R,B,B,B,R,R,R,R,B,B,B,R,B,R,T,
    T,R,B,B,R,R,R,B,R,R,B,B,B,B,R,T,
    T,T,R,B,R,R,B,B,R,R,R,R,B,R,T,T,
    T,T,R,B,B,R,B,B,B,R,R,B,B,R,T,T,
    T,T,T,R,B,B,B,R,B,B,B,B,R,T,T,T,
    T,T,T,T,R,R,B,B,B,B,R,R,T,T,T,T,
    T,T,T,T,T,T,R,R,R,R,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
};
#undef B
#undef R
#undef T

// Enemy type 2 - speed enemy in bright magenta
// B=0 (black background), M=11 (bright magenta)
#define B 0
#define M 11
#define T 0xE3
static const uint8_t sprite_enemy2[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,M,M,M,M,T,T,T,T,T,T,
    T,T,T,T,M,M,B,B,B,B,M,M,T,T,T,T,
    T,T,T,M,B,B,B,B,M,B,B,B,M,T,T,T,
    T,T,M,B,B,M,M,B,B,B,M,B,B,M,T,T,
    T,T,M,B,M,M,M,M,B,B,M,M,B,M,T,T,
    T,M,B,B,B,B,M,M,B,M,M,M,B,B,M,T,
    T,M,B,M,B,B,B,M,M,M,M,B,B,B,M,T,
    T,M,B,B,B,M,M,M,M,B,B,B,M,B,M,T,
    T,M,B,B,M,M,M,B,M,M,B,B,B,B,M,T,
    T,T,M,B,M,M,B,B,M,M,M,M,B,M,T,T,
    T,T,M,B,B,M,B,B,B,M,M,B,B,M,T,T,
    T,T,T,M,B,B,B,M,B,B,B,B,M,T,T,T,
    T,T,T,T,M,M,B,B,B,B,M,M,T,T,T,T,
    T,T,T,T,T,T,M,M,M,M,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
};
#undef B
#undef M
#undef T

// Explosion animation frame (red/yellow/white)
#define T 0xE3
#define W 15
#define Y 14
#define R 10
static const uint8_t sprite_explosion[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,R,T,T,T,T,T,T,R,T,T,T,T,
    T,T,T,R,T,T,R,T,T,R,T,T,R,T,T,T,
    T,T,R,T,T,Y,Y,Y,Y,Y,Y,T,T,R,T,T,
    T,R,T,T,Y,W,W,W,W,W,W,Y,T,T,R,T,
    T,T,T,Y,W,W,W,W,W,W,W,W,Y,T,T,T,
    T,R,T,W,W,W,W,W,W,W,W,W,W,T,R,T,
    R,T,Y,W,W,W,W,W,W,W,W,W,W,Y,T,R,
    R,T,Y,W,W,W,W,W,W,W,W,W,W,Y,T,R,
    T,R,T,W,W,W,W,W,W,W,W,W,W,T,R,T,
    T,T,T,Y,W,W,W,W,W,W,W,W,Y,T,T,T,
    T,R,T,T,Y,W,W,W,W,W,W,Y,T,T,R,T,
    T,T,R,T,T,Y,Y,Y,Y,Y,Y,T,T,R,T,T,
    T,T,T,R,T,T,R,T,T,R,T,T,R,T,T,T,
    T,T,T,T,R,T,T,T,T,T,T,R,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
};
#undef T
#undef W
#undef Y
#undef R

// Highway tile - bright cyan fill with bright white border on left and top
#define W 15
#define C 13
static const uint8_t sprite_highway[256] = {
    W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,W,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C,
    W,C,C,C,C,C,C,C,C,C,C,C,C,C,C,C
};
#undef W
#undef C

// Player shadow - same shape as player but dark gray
// T=0xE3 (transparent), S=8 (bright black / dark gray)
#define T 0xE3
#define S 8
static const uint8_t sprite_shadow[256] = {
    T,T,T,T,T,T,T,S,S,T,T,T,T,T,T,T,
    T,T,T,T,T,T,S,S,S,S,T,T,T,T,T,T,
    T,T,T,T,T,S,S,S,S,S,S,T,T,T,T,T,
    T,T,T,T,T,S,S,S,S,S,S,T,T,T,T,T,
    T,T,T,T,S,S,S,S,S,S,S,S,T,T,T,T,
    T,T,T,T,S,S,S,S,S,S,S,S,T,T,T,T,
    T,T,T,S,S,S,S,S,S,S,S,S,S,T,T,T,
    T,T,T,S,S,S,S,S,S,S,S,S,S,T,T,T,
    T,T,S,S,S,S,S,S,S,S,S,S,S,S,T,T,
    T,T,S,S,S,S,S,S,S,S,S,S,S,S,T,T,
    T,S,S,S,T,S,S,S,S,S,S,T,S,S,S,T,
    T,S,S,T,T,S,S,S,S,S,S,T,T,S,S,T,
    S,S,S,T,T,T,S,S,S,S,T,T,T,S,S,S,
    S,S,T,T,T,T,S,S,S,S,T,T,T,T,S,S,
    S,T,T,T,T,T,T,S,S,T,T,T,T,T,T,S,
    T,T,T,T,T,T,T,S,S,T,T,T,T,T,T,T
};
#undef T
#undef S

#endif
