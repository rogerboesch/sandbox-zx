#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>

// ============================================================================
// CONSTANTS
// ============================================================================

// Lane configurations (2 bits)
#define LANE_CENTER  0   // Single lane in center
#define LANE_LEFT    1   // Single lane on left
#define LANE_RIGHT   2   // Single lane on right
#define LANE_BOTH    3   // Two lanes with gap

// Object types (3 bits = 8 types max)
#define OBJ_NONE        0
#define OBJ_HOLE        1   // Pit in the road
#define OBJ_LASER       2   // Laser beam hazard
#define OBJ_POWERUP     3   // Collectible
#define OBJ_ENEMY_SPAWN 4   // Triggers enemy spawn
#define OBJ_SPEED_ZONE  5   // Speed modifier
// Reserved: 6, 7

// Object sizes/variants (3 bits = 8 variants)
#define SIZE_SMALL    0   // 1 block wide
#define SIZE_PARTIAL  1   // Half lane width
#define SIZE_FULL     2   // Full lane width (needs jump)
// Reserved: 3-7 for variants

// Object lane position (2 bits)
#define OBJ_LANE_LEFT   0
#define OBJ_LANE_RIGHT  1
#define OBJ_LANE_CENTER 2
// Reserved: 3

// Layout constants
#define BLOCK_SIZE_PX       16      // 1 block = 16 pixels = 2 tiles
#define GAP_BLOCKS          1       // Gap between lanes in LANE_BOTH
#define TRANSITION_BLOCKS   4       // Blocks where lanes connect for switching
#define SCREEN_CENTER_PX    128     // Center of 256px screen
#define TILEMAP_CENTER_TILE 20      // Center tile (with 4-tile offset)

// ============================================================================
// HELPER MACROS - Packing
// ============================================================================

// Pack segment config byte: LLWWWWWW
#define SEGMENT_CONFIG(lanes, width) \
    (((lanes) << 6) | ((width) & 0x3F))

// Pack object data byte: TTTLLSSS
#define OBJECT_DATA(type, lane, size) \
    (((type) << 5) | ((lane) << 3) | ((size) & 0x07))

// ============================================================================
// HELPER MACROS - Unpacking
// ============================================================================

// Unpack segment config
#define SEGMENT_LANES(config)  (((config) >> 6) & 0x03)
#define SEGMENT_WIDTH(config)  ((config) & 0x3F)

// Unpack object data
#define OBJECT_TYPE(data)  (((data) >> 5) & 0x07)
#define OBJECT_LANE(data)  (((data) >> 3) & 0x03)
#define OBJECT_SIZE(data)  ((data) & 0x07)

// ============================================================================
// DATA STRUCTURES - ROM (Read Only)
// ============================================================================

// Segment definition: 4 bytes each
// Stored in ROM (const), generated from YAML
typedef struct {
    uint8_t length;      // 1-255 blocks until next segment (0 = end of level)
    uint8_t config;      // Packed: LLWWWWWW
                         //   LL (bits 7-6): lane config (CENTER/LEFT/RIGHT/BOTH)
                         //   WWWWWW (bits 5-0): width per lane (1-63 blocks)
    uint8_t obj_count;   // Number of objects in this segment (0-255)
    uint8_t obj_offset;  // Starting index in objects array (0-255)
} LevelSegment;

// Object definition: 2 bytes each
// Stored in ROM (const), generated from YAML
typedef struct {
    uint8_t at;          // Block offset within segment (0-255)
    uint8_t data;        // Packed: TTTLLSSS
                         //   TTT (bits 7-5): object type
                         //   LL (bits 4-3): lane position
                         //   SSS (bits 2-0): size/variant
} LevelObject;

// Level header (stored in generated header file)
typedef struct {
    const char* name;              // Level name (for display)
    uint8_t segment_count;         // Total segments
    uint8_t object_count;          // Total objects
    const LevelSegment* segments;  // Pointer to segments array
    const LevelObject* objects;    // Pointer to objects array
} LevelDef;

// ============================================================================
// DATA STRUCTURES - RAM (Runtime State)
// ============================================================================

// Current level state - updated as player progresses
typedef struct {
    // Level reference
    const LevelDef* def;         // Pointer to current level definition

    // Segment tracking
    uint8_t segment_idx;         // Current segment index (0-255)
    uint8_t block_counter;       // Blocks remaining in current segment
    uint8_t blocks_scrolled;     // Blocks scrolled within current segment

    // Object tracking
    uint8_t obj_idx;             // Next object index to check
    uint8_t obj_segment_end;     // Last object index +1 for current segment

    // Cached boundaries (pixels, for collision detection)
    int16_t left_lane_left;      // Left edge of left/center lane
    int16_t left_lane_right;     // Right edge of left/center lane
    int16_t right_lane_left;     // Left edge of right lane (if BOTH)
    int16_t right_lane_right;    // Right edge of right lane (if BOTH)

    // Current config (cached for fast access)
    uint8_t current_lanes;       // LANE_CENTER/LEFT/RIGHT/BOTH
    uint8_t current_width;       // Width in blocks

    // Transition state
    uint8_t in_transition;       // 1 if lanes are connected (player can switch)
    uint8_t transition_counter;  // Blocks remaining in transition zone

    // Scroll tracking for row generation
    int16_t last_scroll_y;       // Last scroll position (for detecting new rows)

} LevelState;

// ============================================================================
// GLOBAL STATE
// ============================================================================

// Global level state instance
extern LevelState level_state;

// ============================================================================
// API FUNCTIONS
// ============================================================================

// Initialize level system with a level definition
void level_init(const LevelDef* def);

// Called every frame - advances block counter, triggers segment changes
// Pass current scroll_y value
void level_update(int16_t scroll_y);

// Get current lane boundaries for collision (single lane or center)
// For LANE_BOTH without transition, returns left lane bounds
void level_get_boundaries(int16_t* left, int16_t* right);

// For LANE_BOTH: get both lane boundaries
void level_get_both_boundaries(int16_t* l_left, int16_t* l_right,
                               int16_t* r_left, int16_t* r_right);

// Check if currently in LANE_BOTH configuration
uint8_t level_is_both_lanes(void);

// Check if player is in transition zone (can switch lanes)
uint8_t level_in_transition(void);

// Generate tilemap tiles for a specific row
// row: tilemap row (0-31) - used for writing to tilemap memory
// world_y: world position in pixels (0 = start of level)
// tiles: output array of 40 tile indices
void level_generate_row(uint8_t row, int16_t world_y, uint8_t* tiles);

// Get current segment info (for debugging/display)
uint8_t level_get_segment_index(void);
uint8_t level_get_blocks_remaining(void);

#endif // LEVEL_H
