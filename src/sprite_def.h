#ifndef SPRITEDEF_H
#define SPRITEDEF_H

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

// Player shadow - dithered (checkerboard)
// T=0xE3 (transparent), S=8 (bright black / dark gray)
#define T 0xE3
#define S 8
static const uint8_t sprite_player_shadow[256] = {
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
