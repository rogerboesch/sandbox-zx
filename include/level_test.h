// ============================================================================
// TEST LEVEL - Simple short level for testing level completion
// ============================================================================

#ifndef LEVEL_TEST_H
#define LEVEL_TEST_H

#include "../src/level.h"

// ============================================================================
// SEGMENT DATA (ROM)
// ============================================================================

static const LevelSegment level_test_segments[] = {
    // Single lane, 4 wide, length 30
    { 30, SEGMENT_CONFIG(LANE_CENTER, 4), 0, 0 },

    // End marker
    { 0, 0, 0, 0 }
};

// ============================================================================
// OBJECT DATA (ROM)
// ============================================================================

static const LevelObject level_test_objects[] = {
    // No objects
    { 0, 0 }
};

// ============================================================================
// LEVEL DEFINITION
// ============================================================================

static const LevelDef level_test_def = {
    "TEST",
    1,                      // segment_count (excluding end marker)
    0,                      // object_count
    level_test_segments,
    level_test_objects
};

#endif // LEVEL_TEST_H
