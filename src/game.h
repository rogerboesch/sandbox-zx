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

// Level boundaries (tiles 16-23 = pixels 128-191, adjusted for tilemap offset)
// Tilemap is offset by 4 tiles, so level at tiles 16-23 appears at pixels 96-159
#define LEVEL_LEFT      96
#define LEVEL_RIGHT     160

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

// Sprite pattern slots
#define SPRITE_PLAYER       0
#define SPRITE_BULLET       1
#define SPRITE_ENEMY_BASE   2   // Enemy frames A0-G0 at slots 2-8
#define ENEMY_ANIM_FRAMES   7   // 7 animation frames
#define SPRITE_SHADOW       9
#define SPRITE_ENEMY_SHADOW 10

// Shadow offset
#define SHADOW_OFFSET_X  3
#define SHADOW_OFFSET_Y  3

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
    STATE_DYING,      // Player dead, still on game screen, waiting for input
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

// Player structure
typedef struct {
    int16_t x;
    int16_t y;
    uint8_t lives;
    uint8_t shield;
    uint8_t fire_cooldown;
    uint8_t invincible;
} Player;

// Crash types for border flash colors
#define CRASH_NONE       0
#define CRASH_HOLE       1  // Blue flash
#define CRASH_ENEMY      2  // Yellow flash (normal enemy)
#define CRASH_ENEMY_FAST 3  // Red flash (fast enemy)
#define CRASH_LEVEL      4  // White flash (fell off level)

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
    uint8_t crash_type;       // Type of crash for border color
    uint8_t survival_timer;   // Counts frames for survival bonus
} GameData;

// Function prototypes - game.c
void game_init(void);
void game_update(void);
void game_update_dying(void);
void game_render(void);
void game_render_dying(void);
int8_t game_get_shake_offset(void);

// Input handling
uint8_t input_read(void);

// Input bit flags
#define INPUT_UP     0x01
#define INPUT_DOWN   0x02
#define INPUT_LEFT   0x04
#define INPUT_RIGHT  0x08
#define INPUT_FIRE   0x10
#define INPUT_PAUSE  0x20

// Global game data (extern declaration)
extern GameData game;

#endif // GAME_H
