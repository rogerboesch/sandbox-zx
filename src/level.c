#include <stdint.h>
#include <string.h>
#include "level.h"

// ============================================================================
// GLOBAL STATE
// ============================================================================

LevelState level_state;

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

// Recalculate lane boundaries based on current segment config
static void calculate_boundaries(void) {
    uint8_t width = level_state.current_width;
    int16_t width_px = width * BLOCK_SIZE_PX;
    int16_t half_width = width_px / 2;
    int16_t gap_px = GAP_BLOCKS * BLOCK_SIZE_PX / 2;  // Half gap on each side

    switch (level_state.current_lanes) {
        case LANE_CENTER:
            // Single centered lane
            level_state.left_lane_left = SCREEN_CENTER_PX - half_width;
            level_state.left_lane_right = SCREEN_CENTER_PX + half_width;
            level_state.right_lane_left = 0;
            level_state.right_lane_right = 0;
            break;

        case LANE_LEFT:
            // Single lane offset to left
            level_state.left_lane_right = SCREEN_CENTER_PX - gap_px;
            level_state.left_lane_left = level_state.left_lane_right - width_px;
            level_state.right_lane_left = 0;
            level_state.right_lane_right = 0;
            break;

        case LANE_RIGHT:
            // Single lane offset to right
            level_state.left_lane_left = SCREEN_CENTER_PX + gap_px;
            level_state.left_lane_right = level_state.left_lane_left + width_px;
            level_state.right_lane_left = 0;
            level_state.right_lane_right = 0;
            break;

        case LANE_BOTH:
            // Two lanes with gap in center
            // Left lane
            level_state.left_lane_right = SCREEN_CENTER_PX - gap_px;
            level_state.left_lane_left = level_state.left_lane_right - width_px;
            // Right lane
            level_state.right_lane_left = SCREEN_CENTER_PX + gap_px;
            level_state.right_lane_right = level_state.right_lane_left + width_px;
            break;
    }
}

// Load a segment by index
static void load_segment(uint8_t idx) {
    const LevelSegment* seg;

    if (idx >= level_state.def->segment_count) {
        // End of level - stay on last segment
        return;
    }

    seg = &level_state.def->segments[idx];

    // Check for end marker
    if (seg->length == 0) {
        // End of level
        return;
    }

    level_state.segment_idx = idx;
    level_state.block_counter = seg->length;
    level_state.blocks_scrolled = 0;
    level_state.current_lanes = SEGMENT_LANES(seg->config);
    level_state.current_width = SEGMENT_WIDTH(seg->config);

    // Set up object tracking for this segment
    level_state.obj_idx = seg->obj_offset;
    level_state.obj_segment_end = seg->obj_offset + seg->obj_count;

    // Recalculate boundaries
    calculate_boundaries();
}

// Check if next segment exists and has different lane config
static uint8_t check_transition_needed(void) {
    uint8_t next_idx = level_state.segment_idx + 1;
    const LevelSegment* next_seg;
    uint8_t next_lanes;

    if (next_idx >= level_state.def->segment_count) {
        return 0;
    }

    next_seg = &level_state.def->segments[next_idx];
    if (next_seg->length == 0) {
        return 0;
    }

    next_lanes = SEGMENT_LANES(next_seg->config);
    return (next_lanes != level_state.current_lanes) ? 1 : 0;
}

// ============================================================================
// PUBLIC API
// ============================================================================

// Initialize level system with a level definition
void level_init(const LevelDef* def) {
    // Clear state
    memset(&level_state, 0, sizeof(LevelState));

    // Store level reference
    level_state.def = def;

    // Load first segment
    load_segment(0);

    // Initialize scroll tracking
    level_state.last_scroll_y = 0;
}

// Called every frame - advances block counter, triggers segment changes
void level_update(int16_t scroll_y) {
    int16_t scroll_diff;
    int16_t blocks_to_advance;

    // Calculate how much we've scrolled since last update
    scroll_diff = level_state.last_scroll_y - scroll_y;  // Negative scroll = forward
    level_state.last_scroll_y = scroll_y;

    // Convert scroll to blocks (16px per block)
    // We track partial blocks with blocks_scrolled
    if (scroll_diff < 0) {
        scroll_diff = -scroll_diff;  // Make positive
    }

    // Accumulate scroll and check for block boundaries
    // This is simplified - in practice we track pixel-level scroll
    // For now, advance one block every 16 pixels scrolled
    blocks_to_advance = scroll_diff / BLOCK_SIZE_PX;

    while (blocks_to_advance > 0 && level_state.block_counter > 0) {
        level_state.block_counter--;
        level_state.blocks_scrolled++;
        blocks_to_advance--;

        // Check for segment transition
        if (level_state.block_counter == 0) {
            // Check if we need a transition zone
            if (check_transition_needed() && !level_state.in_transition) {
                level_state.in_transition = 1;
                level_state.transition_counter = TRANSITION_BLOCKS;
            }

            // Advance to next segment
            load_segment(level_state.segment_idx + 1);
        }
    }

    // Handle transition countdown
    if (level_state.in_transition && level_state.transition_counter > 0) {
        level_state.transition_counter--;
        if (level_state.transition_counter == 0) {
            level_state.in_transition = 0;
        }
    }
}

// Get current lane boundaries for collision (single lane or center)
void level_get_boundaries(int16_t* left, int16_t* right) {
    // In transition, expand boundaries to cover both lanes
    if (level_state.in_transition && level_state.current_lanes == LANE_BOTH) {
        *left = level_state.left_lane_left;
        *right = level_state.right_lane_right;
    }
    else {
        *left = level_state.left_lane_left;
        *right = level_state.left_lane_right;
    }
}

// For LANE_BOTH: get both lane boundaries
void level_get_both_boundaries(int16_t* l_left, int16_t* l_right,
                               int16_t* r_left, int16_t* r_right) {
    *l_left = level_state.left_lane_left;
    *l_right = level_state.left_lane_right;
    *r_left = level_state.right_lane_left;
    *r_right = level_state.right_lane_right;
}

// Check if currently in LANE_BOTH configuration
uint8_t level_is_both_lanes(void) {
    return (level_state.current_lanes == LANE_BOTH) ? 1 : 0;
}

// Check if player is in transition zone (can switch lanes)
uint8_t level_in_transition(void) {
    return level_state.in_transition;
}

// Tile indices (must match tilemap.c definitions)
#define TILE_ROAD_LEFT    0x00  // left border
#define TILE_ROAD_MID     0x01  // middle
#define TILE_ROAD_RIGHT   0x02  // right border
#define TILE_TRANS        0x03  // transparent

// Generate tilemap tiles for a specific row
void level_generate_row(uint8_t row, int16_t scroll_y, uint8_t* tiles) {
    uint8_t i;
    uint8_t left_start, left_end;

    // Clear row to transparent (tile 3, not 0!)
    for (i = 0; i < 40; i++) {
        tiles[i] = TILE_TRANS;
    }

    // Calculate tile positions from pixel boundaries
    // Tilemap tiles are 8px each, boundaries are in pixels
    // Tilemap has 4-tile offset from screen, so tile 4 = screen pixel 0

    // Convert pixel boundaries to tile indices
    // Screen pixel X -> tilemap tile = (X / 8) + 4
    left_start = (uint8_t)((level_state.left_lane_left / 8) + 4);
    left_end = (uint8_t)((level_state.left_lane_right / 8) + 4);

    // Clamp to valid tile range
    if (left_start > 39) left_start = 39;
    if (left_end > 40) left_end = 40;
    if (left_end <= left_start) return;  // Invalid range

    // Fill lane tiles: left edge, middle tiles, right edge
    tiles[left_start] = TILE_ROAD_LEFT;
    for (i = left_start + 1; i < left_end - 1; i++) {
        tiles[i] = TILE_ROAD_MID;
    }
    if (left_end > left_start + 1) {
        tiles[left_end - 1] = TILE_ROAD_RIGHT;
    }

    // Handle LANE_BOTH - add right lane
    if (level_state.current_lanes == LANE_BOTH) {
        uint8_t right_start = (uint8_t)((level_state.right_lane_left / 8) + 4);
        uint8_t right_end = (uint8_t)((level_state.right_lane_right / 8) + 4);

        // Clamp to valid tile range
        if (right_start > 39) right_start = 39;
        if (right_end > 40) right_end = 40;

        if (right_end > right_start) {
            // Fill right lane tiles
            tiles[right_start] = TILE_ROAD_LEFT;
            for (i = right_start + 1; i < right_end - 1; i++) {
                tiles[i] = TILE_ROAD_MID;
            }
            if (right_end > right_start + 1) {
                tiles[right_end - 1] = TILE_ROAD_RIGHT;
            }

            // Handle transition - connect the lanes
            if (level_state.in_transition) {
                // Fill the gap between lanes with middle tiles
                for (i = left_end; i < right_start; i++) {
                    tiles[i] = TILE_ROAD_MID;
                }
                // Fix edges at connection points
                if (left_end > 0 && left_end <= 39) {
                    tiles[left_end - 1] = TILE_ROAD_MID;  // Was right edge, now middle
                }
                tiles[right_start] = TILE_ROAD_MID;  // Was left edge, now middle
            }
        }
    }

    // TODO: Check for objects at this row and modify tiles accordingly
    (void)scroll_y;
    (void)row;
}

// Get current segment info (for debugging/display)
uint8_t level_get_segment_index(void) {
    return level_state.segment_idx;
}

uint8_t level_get_blocks_remaining(void) {
    return level_state.block_counter;
}
