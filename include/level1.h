// ============================================================================
// GENERATED FILE - DO NOT EDIT
// Generated from: levels/level1.yaml
// ============================================================================

#ifndef LEVEL1_H
#define LEVEL1_H

#include "../src/level.h"

// ============================================================================
// SEGMENT DATA (ROM)
// ============================================================================

static const LevelSegment level1_segments[] = {
    // Section 1: Introduction
    { 30, SEGMENT_CONFIG(LANE_CENTER, 4), 0, 0 },

    // Section 2: First split
    { 4,  SEGMENT_CONFIG(LANE_BOTH, 4),   0, 0 },   // transition
    { 40, SEGMENT_CONFIG(LANE_BOTH, 3),   2, 0 },   // 2 objects at idx 0

    // Section 3: Right lane only
    { 4,  SEGMENT_CONFIG(LANE_BOTH, 3),   0, 2 },   // transition
    { 30, SEGMENT_CONFIG(LANE_RIGHT, 3),  2, 2 },   // 2 objects at idx 2

    // Section 4: Left lane only
    { 4,  SEGMENT_CONFIG(LANE_BOTH, 3),   0, 4 },   // transition
    { 35, SEGMENT_CONFIG(LANE_LEFT, 3),   2, 4 },   // 2 objects at idx 4

    // Section 5: Both lanes with more obstacles
    { 4,  SEGMENT_CONFIG(LANE_BOTH, 3),   0, 6 },   // transition
    { 50, SEGMENT_CONFIG(LANE_BOTH, 3),   5, 6 },   // 5 objects at idx 6

    // Section 6: Narrow finale
    { 4,  SEGMENT_CONFIG(LANE_BOTH, 2),   0, 11 },  // transition
    { 40, SEGMENT_CONFIG(LANE_CENTER, 2), 3, 11 },  // 3 objects at idx 11

    // Section 7: Victory stretch
    { 4,  SEGMENT_CONFIG(LANE_CENTER, 4), 0, 14 },
    { 30, SEGMENT_CONFIG(LANE_CENTER, 4), 1, 14 },  // 1 object at idx 14

    // End marker
    { 0, 0, 0, 0 }
};

// ============================================================================
// OBJECT DATA (ROM)
// ============================================================================

static const LevelObject level1_objects[] = {
    // Section 2: First split (idx 0-1)
    { 15, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_LEFT, SIZE_PARTIAL) },
    { 25, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_RIGHT, SIZE_PARTIAL) },

    // Section 3: Right lane (idx 2-3)
    { 10, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_RIGHT, SIZE_SMALL) },
    { 20, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_RIGHT, SIZE_PARTIAL) },

    // Section 4: Left lane (idx 4-5)
    { 12, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_LEFT, SIZE_PARTIAL) },
    { 25, OBJECT_DATA(OBJ_LASER, OBJ_LANE_LEFT, 0) },

    // Section 5: Both lanes (idx 6-10)
    { 8,  OBJECT_DATA(OBJ_HOLE, OBJ_LANE_LEFT, SIZE_PARTIAL) },
    { 16, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_RIGHT, SIZE_PARTIAL) },
    { 24, OBJECT_DATA(OBJ_LASER, OBJ_LANE_LEFT, 0) },
    { 32, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_RIGHT, SIZE_PARTIAL) },
    { 40, OBJECT_DATA(OBJ_LASER, OBJ_LANE_RIGHT, 0) },

    // Section 6: Narrow finale (idx 11-13)
    { 10, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_CENTER, SIZE_SMALL) },
    { 20, OBJECT_DATA(OBJ_HOLE, OBJ_LANE_CENTER, SIZE_SMALL) },
    { 30, OBJECT_DATA(OBJ_LASER, OBJ_LANE_CENTER, 0) },

    // Section 7: Victory stretch (idx 14)
    { 15, OBJECT_DATA(OBJ_POWERUP, OBJ_LANE_CENTER, 0) },
};

// ============================================================================
// LEVEL DEFINITION
// ============================================================================

static const LevelDef level1_def = {
    "LEVEL 1",
    14,                     // segment_count (excluding end marker)
    15,                     // object_count
    level1_segments,
    level1_objects
};

#endif // LEVEL1_H
