#ifndef SPRITEDEF_H
#define SPRITEDEF_H

// Player ship - rotated 90° clockwise
// T=0xE3 (transparent), M=11 (bright magenta)
#define T 0xE3
#define M 11
static const uint8_t sprite_player[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,M,T,T,T,T,M,T,T,T,T,M,T,T,
    T,T,T,T,T,T,T,T,M,T,T,T,T,T,T,T,
    T,T,T,M,T,T,T,M,M,M,T,T,T,M,T,T,
    T,T,T,T,T,T,M,M,M,M,M,T,T,T,T,T,
    T,T,M,M,M,T,M,M,M,M,M,T,M,M,M,T,
    T,T,M,M,M,T,M,M,M,M,M,T,M,M,M,T,
    T,T,T,M,T,T,T,T,T,T,T,T,T,M,T,T,
    T,M,M,M,M,M,T,M,M,M,T,M,M,M,M,M,
    T,M,M,M,M,M,T,M,M,M,T,M,M,M,M,M,
    T,M,M,M,M,M,T,M,M,M,T,M,M,M,M,M,
    T,T,M,M,M,M,M,M,M,M,M,M,M,M,M,T,
    T,T,M,M,M,M,M,M,M,M,M,M,M,M,M,T,
    T,T,T,M,M,M,M,M,M,M,M,M,M,M,T,T,
    T,T,T,T,M,M,M,M,M,M,M,M,M,T,T,T
};
#undef T
#undef M

// Bullet - vertical energy bolt (bright yellow/white)
#define T 0xE3
#define W 8
#define Y 14
static const uint8_t sprite_bullet[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,W,W,T,T,T,W,W,T,T,T,W,W,T,
    T,T,T,W,W,T,T,T,W,W,T,T,T,W,W,T,
    T,T,T,W,W,T,T,T,W,W,T,T,T,W,W,T,
    T,T,T,Y,Y,T,T,T,Y,Y,T,T,T,Y,Y,T
};
#undef T
#undef W
#undef Y

// Enemy type 1 - from tilemap 2x2 block (7,7)-(8,8) in bright yellow
// B=0 (black background), Y=14 (bright yellow)
#define B 0
#define Y 14
#define T 0xE3
static const uint8_t sprite_enemy1[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,Y,Y,Y,Y,T,T,T,T,T,T,
    T,T,T,T,Y,Y,B,B,B,B,Y,Y,T,T,T,T,
    T,T,T,Y,B,B,B,B,Y,B,B,B,Y,T,T,T,
    T,T,Y,B,B,Y,Y,B,B,B,Y,B,B,Y,T,T,
    T,T,Y,B,Y,Y,Y,Y,B,B,Y,Y,B,Y,T,T,
    T,Y,B,B,B,B,Y,Y,B,Y,Y,Y,B,B,Y,T,
    T,Y,B,Y,B,B,B,Y,Y,Y,Y,B,B,B,Y,T,
    T,Y,B,B,B,Y,Y,Y,Y,B,B,B,Y,B,Y,T,
    T,Y,B,B,Y,Y,Y,B,Y,Y,B,B,B,B,Y,T,
    T,T,Y,B,Y,Y,B,B,Y,Y,Y,Y,B,Y,T,T,
    T,T,Y,B,B,Y,B,B,B,Y,Y,B,B,Y,T,T,
    T,T,T,Y,B,B,B,Y,B,B,B,B,Y,T,T,T,
    T,T,T,T,Y,Y,B,B,B,B,Y,Y,T,T,T,T,
    T,T,T,T,T,T,Y,Y,Y,Y,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
};
#undef B
#undef Y
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

// Player shadow - matches new player shape, dithered (checkerboard)
// T=0xE3 (transparent), S=8 (bright black / dark gray)
#define T 0xE3
#define S 8
static const uint8_t sprite_shadow[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,S,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,S,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,S,T,S,T,T,T,T,T,T,
    T,T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,
    T,T,T,S,T,T,T,S,T,S,T,T,T,S,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,S,
    T,T,S,T,S,T,T,T,S,T,T,T,S,T,S,T,
    T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,S,
    T,T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,
    T,T,T,S,T,S,T,S,T,S,T,S,T,S,T,T,
    T,T,T,T,S,T,S,T,S,T,S,T,S,T,T,T,
    T,T,T,T,T,S,T,S,T,S,T,S,T,T,T,T
};
#undef T
#undef S

// Enemy shadow - circular shape with dithered pattern (checkerboard)
// T=0xE3 (transparent), S=8 (bright black / dark gray)
#define T 0xE3
#define S 8
static const uint8_t sprite_enemy_shadow[256] = {
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
    T,T,T,T,T,T,S,T,S,T,T,T,T,T,T,T,
    T,T,T,T,S,T,S,T,S,T,S,T,T,T,T,T,
    T,T,T,S,T,S,T,S,T,S,T,S,T,T,T,T,
    T,T,S,T,S,T,S,T,S,T,S,T,S,T,T,T,
    T,T,T,S,T,S,T,S,T,S,T,S,T,S,T,T,
    T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,T,
    T,T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,
    T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,T,
    T,T,S,T,S,T,S,T,S,T,S,T,S,T,S,T,
    T,T,T,S,T,S,T,S,T,S,T,S,T,S,T,T,
    T,T,S,T,S,T,S,T,S,T,S,T,S,T,T,T,
    T,T,T,S,T,S,T,S,T,S,T,S,T,T,T,T,
    T,T,T,T,S,T,S,T,S,T,S,T,T,T,T,T,
    T,T,T,T,T,T,S,T,S,T,T,T,T,T,T,T,
    T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T
};
#undef T
#undef S

#endif
