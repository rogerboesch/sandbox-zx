/*
 * Dark Nebula - ZX Spectrum Next Prototype
 * game.h - Game constants, types, and function prototypes
 */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

// Screen dimensions (Layer 2 256x192)
#define SCREEN_WIDTH    256
#define SCREEN_HEIGHT   192

// Game area boundaries
#define GAME_TOP        8
#define GAME_BOTTOM     184
#define GAME_LEFT       0
#define GAME_RIGHT      240

// Player constants (vertical scroller - player at bottom)
#define PLAYER_WIDTH    16
#define PLAYER_HEIGHT   16
#define PLAYER_SPEED    3
#define PLAYER_START_X  120  // Center of screen
#define PLAYER_START_Y  160  // Near bottom
#define PLAYER_MAX_LIVES 3

// Bullet constants
#define MAX_BULLETS     8
#define BULLET_SPEED    4
#define BULLET_WIDTH    8
#define BULLET_HEIGHT   4

// Enemy constants
#define MAX_ENEMIES     8
#define ENEMY_WIDTH     16
#define ENEMY_HEIGHT    16
#define ENEMY_SPEED     1

// Scoring constants
#define SCORE_PER_SECOND    10   // Points for surviving each second
#define SCORE_ENEMY_NORMAL  100  // Points for normal enemy
#define SCORE_ENEMY_FAST    300  // Points for fast enemy

// Effects
#define SHAKE_DURATION      20   // Frames of screen shake
#define CRASH_TEXT_DURATION 60   // Frames to show "CRASH"

// Star field constants
#define MAX_STARS       40
#define STAR_LAYERS     3

// Sprite pattern slots
#define SPRITE_PLAYER   0
#define SPRITE_BULLET   1
#define SPRITE_ENEMY1   2
#define SPRITE_ENEMY2   3
#define SPRITE_EXPLOSION 4
#define SPRITE_LIFE     5
#define SPRITE_DIGIT_0  6   // Digits 0-9 are patterns 6-15
#define SPRITE_HIGHWAY  16  // Highway tile (cyan with white left/top border)

// HUD sprite slots (high slots to avoid game sprites)
#define HUD_SPRITE_LIFE_BASE  24  // 3 life sprites: 24, 25, 26
#define HUD_SPRITE_SCORE_BASE 27  // 5 score digits: 27-31

// Colors (9-bit RGB, using 8-bit approximation)
#define COLOR_BLACK     0x00
#define COLOR_WHITE     0xFF
#define COLOR_RED       0xE0
#define COLOR_GREEN     0x1C
#define COLOR_BLUE      0x03
#define COLOR_YELLOW    0xFC
#define COLOR_CYAN      0x1F
#define COLOR_MAGENTA   0xE3
#define COLOR_ORANGE    0xF4
#define COLOR_DARK_BLUE 0x01
#define COLOR_PURPLE    0x63

// Game states
typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER,
    STATE_LEVELCOMPLETE
} GameState;

// Entity structure for game objects
typedef struct {
    int16_t x;
    int16_t y;
    int8_t dx;
    int8_t dy;
    uint8_t active;
    uint8_t type;
    uint8_t frame;
    uint8_t health;
} Entity;

// Star structure for background
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t speed;
    uint8_t color;
} Star;

// Player structure
typedef struct {
    int16_t x;
    int16_t y;
    uint8_t lives;
    uint8_t shield;
    uint8_t fire_cooldown;
    uint8_t invincible;
} Player;

// Game data structure
typedef struct {
    GameState state;
    uint16_t score;
    uint16_t high_score;
    uint8_t level;
    uint8_t wave;
    uint8_t enemies_killed;
    uint8_t frame_count;
    uint8_t shake_timer;      // Screen shake countdown
    uint8_t crash_timer;      // "CRASH" text countdown
    uint8_t survival_timer;   // Counts frames for survival bonus
} GameData;

// Function prototypes - game.c
void game_init(void);
void game_update(void);
void game_render(void);
void game_spawn_enemy(void);
void game_check_collisions(void);
void game_update_enemies(void);
void game_update_bullets(void);
void game_fire_bullet(void);
int8_t game_get_shake_offset(void);

// Function prototypes - sprites.c
void sprites_init(void);
void sprites_upload_patterns(void);
void sprite_set(uint8_t slot, int16_t x, int16_t y, uint8_t pattern, uint8_t flags);
void sprite_hide(uint8_t slot);

// Layer 2 graphics functions
void layer2_clear(uint8_t color);
void layer2_plot(uint8_t x, uint8_t y, uint8_t color);
void layer2_hline(uint8_t x1, uint8_t x2, uint8_t y, uint8_t color);
void layer2_vline(uint8_t x, uint8_t y1, uint8_t y2, uint8_t color);
void layer2_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

// Star field functions
void stars_init(void);
void stars_update(void);
void stars_render(void);

// Input handling
uint8_t input_read(void);

// Input bit flags
#define INPUT_UP     0x01
#define INPUT_DOWN   0x02
#define INPUT_LEFT   0x04
#define INPUT_RIGHT  0x08
#define INPUT_FIRE   0x10
#define INPUT_PAUSE  0x20

// Global game data (extern declarations)
extern Player player;
extern Entity bullets[MAX_BULLETS];
extern Entity enemies[MAX_ENEMIES];
extern Star stars[MAX_STARS];
extern GameData game;

#endif // GAME_H
