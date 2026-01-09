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

    // Calculate how much we've scrolled since last update
    // scroll_y goes negative (0, -1, -2, ...), so diff = last - current = positive when scrolling
    scroll_diff = level_state.last_scroll_y - scroll_y;
    level_state.last_scroll_y = scroll_y;

    // Only count forward scrolling (positive diff)
    if (scroll_diff > 0) {
        // Accumulate scroll pixels
        level_state.scroll_accumulator += scroll_diff;

        // Check for block boundaries (16 pixels per block)
        while (level_state.scroll_accumulator >= BLOCK_SIZE_PX && level_state.block_counter > 0) {
            level_state.scroll_accumulator -= BLOCK_SIZE_PX;
            level_state.block_counter--;
            level_state.blocks_scrolled++;

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

// Check if level is complete (reached end of last segment)
uint8_t level_is_complete(void) {
    // On last segment and no blocks remaining
    return (level_state.segment_idx >= level_state.def->segment_count - 1 &&
            level_state.block_counter == 0) ? 1 : 0;
}

// Tile indices (must match tilemap.c definitions)
#define TILE_ROAD_LEFT    0x00  // left border
#define TILE_ROAD_MID_TL  0x01  // highway middle top-left
#define TILE_ROAD_MID_TR  0x02  // highway middle top-right
#define TILE_ROAD_MID_BL  0x03  // highway middle bottom-left
#define TILE_ROAD_MID_BR  0x04  // highway middle bottom-right
#define TILE_ROAD_RIGHT   0x05  // right border
#define TILE_TRANS        0x06  // transparent
#define TILE_LANE_MARK    0x0B  // lane marker (I4) - every 10 blocks
#define TILE_LANE_EDGE    0x0C  // lane start/end marker (J4)

// Calculate which segment a world-Y position belongs to
// Returns segment index, or 0xFF if beyond level end
// Also calculates lane boundaries for that segment
// out_seg_start_y: pixel position where this segment starts
// out_seg_end_y: pixel position where this segment ends
static uint8_t get_segment_at_world_y(int16_t world_y,
                                       uint8_t* out_lanes,
                                       int16_t* out_l_left, int16_t* out_l_right,
                                       int16_t* out_r_left, int16_t* out_r_right,
                                       int16_t* out_seg_start_y, int16_t* out_seg_end_y) {
    const LevelSegment* seg;
    int16_t accumulated_blocks = 0;
    uint8_t i;
    uint8_t lanes, width;
    int16_t width_px, half_width, gap_px;

    // world_y = distance in pixels from the start of the level
    // world_y = 0 at level start, increases as we progress
    // world_y = 500 means 500 pixels into the level

    int16_t distance_blocks;
    if (world_y < 0) {
        distance_blocks = 0;
    } else {
        distance_blocks = world_y / BLOCK_SIZE_PX;
    }

    // Walk through segments to find which one contains this position
    for (i = 0; i < level_state.def->segment_count; i++) {
        seg = &level_state.def->segments[i];
        if (seg->length == 0) break;  // End marker

        if (distance_blocks < accumulated_blocks + seg->length) {
            // Found the segment
            lanes = SEGMENT_LANES(seg->config);
            width = SEGMENT_WIDTH(seg->config);

            // Calculate boundaries for this segment
            width_px = width * BLOCK_SIZE_PX;
            half_width = width_px / 2;
            gap_px = GAP_BLOCKS * BLOCK_SIZE_PX / 2;

            *out_lanes = lanes;
            *out_seg_start_y = accumulated_blocks * BLOCK_SIZE_PX;
            *out_seg_end_y = (accumulated_blocks + seg->length) * BLOCK_SIZE_PX;

            switch (lanes) {
                case LANE_CENTER:
                    *out_l_left = SCREEN_CENTER_PX - half_width;
                    *out_l_right = SCREEN_CENTER_PX + half_width;
                    *out_r_left = 0;
                    *out_r_right = 0;
                    break;

                case LANE_LEFT:
                    *out_l_right = SCREEN_CENTER_PX - gap_px;
                    *out_l_left = *out_l_right - width_px;
                    *out_r_left = 0;
                    *out_r_right = 0;
                    break;

                case LANE_RIGHT:
                    *out_l_left = SCREEN_CENTER_PX + gap_px;
                    *out_l_right = *out_l_left + width_px;
                    *out_r_left = 0;
                    *out_r_right = 0;
                    break;

                case LANE_BOTH:
                    *out_l_right = SCREEN_CENTER_PX - gap_px;
                    *out_l_left = *out_l_right - width_px;
                    *out_r_left = SCREEN_CENTER_PX + gap_px;
                    *out_r_right = *out_r_left + width_px;
                    break;
            }

            return i;
        }
        accumulated_blocks += seg->length;
    }

    // Beyond end of level - use last segment config
    *out_lanes = level_state.current_lanes;
    *out_l_left = level_state.left_lane_left;
    *out_l_right = level_state.left_lane_right;
    *out_r_left = level_state.right_lane_left;
    *out_r_right = level_state.right_lane_right;
    *out_seg_start_y = 0;
    *out_seg_end_y = 0x7FFF;  // Very large
    return 0xFF;
}

// Get lane boundaries at a specific world Y position
// world_y: player's world position (scroll offset + screen Y)
// Returns lane config via out_lanes, returns segment index
uint8_t level_get_boundaries_at_y(int16_t world_y, uint8_t* out_lanes,
                               int16_t* l_left, int16_t* l_right,
                               int16_t* r_left, int16_t* r_right) {
    int16_t seg_start_y, seg_end_y;
    return get_segment_at_world_y(world_y, out_lanes, l_left, l_right, r_left, r_right,
                           &seg_start_y, &seg_end_y);
}

// Helper: get the correct middle tile for 2x2 highway pattern
// Every 10 rows (80 pixels), use lane marker tile
// At segment start/end, use lane edge tile
static uint8_t get_mid_tile(uint8_t col, int16_t world_y, int16_t seg_start_y, int16_t seg_end_y) {
    // Check for lane start/end (first or last row of segment)
    // Each tile row is 8 pixels
    int16_t row_in_seg = world_y - seg_start_y;
    int16_t rows_to_end = seg_end_y - world_y;

    // First row of segment (lane start) or last row (lane end)
    if (row_in_seg >= 0 && row_in_seg < 8) {
        return TILE_LANE_EDGE;  // Lane start
    }
    if (rows_to_end > 0 && rows_to_end <= 8) {
        return TILE_LANE_EDGE;  // Lane end
    }

    // Check for lane marker row: every 10 rows = 80 pixels
    int16_t block_row = world_y / 8;  // Which 8-pixel row
    if (world_y >= 0 && (block_row % 10) == 0) {
        return TILE_LANE_MARK;
    }

    // 2x2 pattern based on column and row (world_y / 8)
    uint8_t row_parity = (world_y / 8) & 1;
    uint8_t col_parity = col & 1;

    if (row_parity == 0) {
        return col_parity ? TILE_ROAD_MID_TR : TILE_ROAD_MID_TL;
    } else {
        return col_parity ? TILE_ROAD_MID_BR : TILE_ROAD_MID_BL;
    }
}

// Generate tilemap tiles for a specific row at a specific world position
void level_generate_row(uint8_t row, int16_t world_y, uint8_t* tiles) {
    uint8_t i;
    uint8_t left_start, left_end;
    uint8_t lanes;
    int16_t l_left, l_right, r_left, r_right;
    int16_t seg_start_y, seg_end_y;

    // Clear row to transparent
    for (i = 0; i < 40; i++) {
        tiles[i] = TILE_TRANS;
    }

    // Get segment configuration for this world position
    get_segment_at_world_y(world_y, &lanes, &l_left, &l_right, &r_left, &r_right,
                           &seg_start_y, &seg_end_y);

    (void)row;  // Row index not needed, world_y determines content

    // Calculate tile positions from pixel boundaries
    // Tilemap tiles are 8px each, boundaries are in pixels
    // Tilemap has 4-tile offset from screen, so tile 4 = screen pixel 0

    // Convert pixel boundaries to tile indices
    // Screen pixel X -> tilemap tile = (X / 8) + 4
    left_start = (uint8_t)((l_left / 8) + 4);
    left_end = (uint8_t)((l_right / 8) + 4);

    // Clamp to valid tile range
    if (left_start > 39) left_start = 39;
    if (left_end > 40) left_end = 40;
    if (left_end <= left_start) return;  // Invalid range

    // Fill lane tiles: left edge, middle tiles (2x2 pattern), right edge
    tiles[left_start] = TILE_ROAD_LEFT;
    for (i = left_start + 1; i < left_end - 1; i++) {
        tiles[i] = get_mid_tile(i, world_y, seg_start_y, seg_end_y);
    }
    if (left_end > left_start + 1) {
        tiles[left_end - 1] = TILE_ROAD_RIGHT;
    }

    // Handle LANE_BOTH - add right lane
    if (lanes == LANE_BOTH) {
        uint8_t right_start = (uint8_t)((r_left / 8) + 4);
        uint8_t right_end = (uint8_t)((r_right / 8) + 4);

        // Clamp to valid tile range
        if (right_start > 39) right_start = 39;
        if (right_end > 40) right_end = 40;

        if (right_end > right_start) {
            // Fill right lane tiles
            tiles[right_start] = TILE_ROAD_LEFT;
            for (i = right_start + 1; i < right_end - 1; i++) {
                tiles[i] = get_mid_tile(i, world_y, seg_start_y, seg_end_y);
            }
            if (right_end > right_start + 1) {
                tiles[right_end - 1] = TILE_ROAD_RIGHT;
            }
        }
    }
}

// Get current segment info (for debugging/display)
uint8_t level_get_segment_index(void) {
    return level_state.segment_idx;
}

uint8_t level_get_blocks_remaining(void) {
    return level_state.block_counter;
}
