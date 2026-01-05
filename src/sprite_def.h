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

// Enemy type 1 - alien fighter (bright red with bright yellow eyes)
#define T 0xE3
#define R 10
#define Y 14
static const uint8_t sprite_enemy1[256] = {
    T,T,T,T,T,T,T,R,R,T,T,T,T,T,T,T,
    T,T,T,T,T,T,R,R,R,R,T,T,T,T,T,T,
    T,T,T,T,T,R,R,R,R,R,R,T,T,T,T,T,
    T,T,T,T,R,R,R,Y,Y,R,R,R,T,T,T,T,
    T,T,T,R,R,R,Y,Y,Y,Y,R,R,R,T,T,T,
    T,T,R,R,R,R,R,R,R,R,R,R,R,R,T,T,
    T,R,R,R,R,R,R,R,R,R,R,R,R,R,R,T,
    R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,
    T,R,R,R,R,R,R,R,R,R,R,R,R,R,R,T,
    T,T,R,R,R,R,R,R,R,R,R,R,R,R,T,T,
    T,T,T,R,R,R,Y,Y,Y,Y,R,R,R,T,T,T,
    T,T,T,T,R,R,R,Y,Y,R,R,R,T,T,T,T,
    T,T,T,T,T,R,R,R,R,R,R,T,T,T,T,T,
    T,T,T,T,T,T,R,R,R,R,T,T,T,T,T,T,
    T,T,T,T,T,T,T,R,R,T,T,T,T,T,T,T
};
#undef T
#undef R
#undef Y

// Enemy type 2 - larger cruiser (bright green with bright yellow accents)
#define T 0xE3
#define G 12
#define Y 14
static const uint8_t sprite_enemy2[256] = {
    T,T,T,T,G,G,G,G,G,G,G,G,T,T,T,T,
    T,T,T,G,G,G,G,G,G,G,G,G,G,T,T,T,
    T,T,G,G,G,G,G,G,G,G,G,G,G,G,T,T,
    T,G,G,G,G,G,G,Y,Y,G,G,G,G,G,G,T,
    G,G,G,G,G,G,Y,Y,Y,Y,G,G,G,G,G,G,
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
    G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
    G,G,G,G,G,G,Y,Y,Y,Y,G,G,G,G,G,G,
    T,G,G,G,G,G,G,Y,Y,G,G,G,G,G,G,T,
    T,T,G,G,G,G,G,G,G,G,G,G,G,G,T,T,
    T,T,T,G,G,G,G,G,G,G,G,G,G,T,T,T,
    T,T,T,T,G,G,G,G,G,G,G,G,T,T,T,T
};
#undef T
#undef G
#undef Y

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

#endif
