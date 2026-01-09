# Dynamic Level System Specification

## Overview

This document describes the dynamic tilemap level system for the ZX Spectrum Next game. The system allows levels to be defined in human-readable YAML files, which are then converted to optimized C header files for runtime use.

Key features:
- Data-driven level design
- Minimal RAM usage (~20 bytes runtime state)
- Compact ROM storage (~400 bytes per level)
- Support for multiple lane configurations
- Dynamic obstacles and objects

---

## Lane Configurations

The level system supports four lane configurations:

```
LANE_CENTER (single lane in center):
    |        [========]        |

LANE_LEFT (single lane on left side):
    |    [========]            |

LANE_RIGHT (single lane on right side):
    |            [========]    |

LANE_BOTH (two lanes with 1-block gap):
    |    [======] [======]     |
              ^gap^
```

- All configurations are centered on screen
- Gap between lanes (when LANE_BOTH): 1 block (16px)
- Lane width is variable (1-63 blocks per lane)
- 1 block = 2 tiles = 16 pixels

---

## Transitions

When lane configuration changes between segments:

1. **Lane Split/Merge**: A 4-block transition zone where lanes are connected, allowing the player to switch between left and right lanes.

2. **Width Change**: Diagonal connectors show the lane narrowing or widening.

3. **Visual**: Transition sprites will be provided later; for now, draw connected lanes.

```
Example: CENTER -> BOTH transition

    [========]         <- center lane
    [========]
    [====╲ ╱====]      <- transition (connected)
    [====] [====]      <- both lanes
    [====] [====]
```

---

## Data Structures

### Constants & Enums

```c
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
// Reserved: 3-7 for variants (laser colors, powerup types, etc.)

// Object lane position (2 bits)
#define OBJ_LANE_LEFT   0
#define OBJ_LANE_RIGHT  1
#define OBJ_LANE_CENTER 2
// Reserved: 3
```

### Packed Data Structures (ROM)

#### LevelSegment (4 bytes)

Defines a section of the level with consistent lane configuration.

```c
typedef struct {
    uint8_t length;      // 1-255 blocks until next segment (0 = end of level)
    uint8_t config;      // Packed: LLWWWWWW
                         //   LL (bits 7-6): lane config (CENTER/LEFT/RIGHT/BOTH)
                         //   WWWWWW (bits 5-0): width per lane (1-63 blocks)
    uint8_t obj_count;   // Number of objects in this segment (0-255)
    uint8_t obj_offset;  // Starting index in objects array (0-255)
} LevelSegment;
```

| Byte | Bits | Field | Description |
|------|------|-------|-------------|
| 0 | 7-0 | length | Blocks in this segment (0 = end marker) |
| 1 | 7-6 | lanes | Lane configuration (0-3) |
| 1 | 5-0 | width | Width per lane in blocks (1-63) |
| 2 | 7-0 | obj_count | Number of objects in segment |
| 3 | 7-0 | obj_offset | Index into objects array |

#### LevelObject (2 bytes)

Defines an object (hazard, powerup, etc.) within a segment.

```c
typedef struct {
    uint8_t at;          // Block offset within segment (0-255)
    uint8_t data;        // Packed: TTTLLSSS
                         //   TTT (bits 7-5): object type
                         //   LL (bits 4-3): lane position
                         //   SSS (bits 2-0): size/variant
} LevelObject;
```

| Byte | Bits | Field | Description |
|------|------|-------|-------------|
| 0 | 7-0 | at | Block position within segment |
| 1 | 7-5 | type | Object type (0-7) |
| 1 | 4-3 | lane | Lane position (left/right/center) |
| 1 | 2-0 | size | Size or variant (0-7) |

#### LevelDef (8 bytes)

Level header containing metadata and pointers.

```c
typedef struct {
    const char* name;              // Level name (for display)
    uint8_t segment_count;         // Total segments
    uint8_t object_count;          // Total objects
    const LevelSegment* segments;  // Pointer to segments array
    const LevelObject* objects;    // Pointer to objects array
} LevelDef;
```

### Helper Macros

```c
// Pack segment config byte
#define SEGMENT_CONFIG(lanes, width) \
    (((lanes) << 6) | ((width) & 0x3F))

// Unpack segment config
#define SEGMENT_LANES(config)  (((config) >> 6) & 0x03)
#define SEGMENT_WIDTH(config)  ((config) & 0x3F)

// Pack object data byte
#define OBJECT_DATA(type, lane, size) \
    (((type) << 5) | ((lane) << 3) | ((size) & 0x07))

// Unpack object data
#define OBJECT_TYPE(data)  (((data) >> 5) & 0x07)
#define OBJECT_LANE(data)  (((data) >> 3) & 0x03)
#define OBJECT_SIZE(data)  ((data) & 0x07)
```

### Runtime State (RAM)

```c
typedef struct {
    // Level reference
    const LevelDef* def;         // Pointer to current level definition

    // Segment tracking
    uint8_t segment_idx;         // Current segment index (0-255)
    uint8_t block_counter;       // Blocks remaining in current segment

    // Object tracking
    uint8_t obj_idx;             // Next object index to check
    uint8_t obj_segment_end;     // Last object index for current segment

    // Cached boundaries (pixels, for collision detection)
    int16_t left_lane_left;      // Left edge of left/center lane
    int16_t left_lane_right;     // Right edge of left/center lane
    int16_t right_lane_left;     // Left edge of right lane (if BOTH)
    int16_t right_lane_right;    // Right edge of right lane (if BOTH)

    // Current config (cached for fast access)
    uint8_t current_lanes;       // LANE_CENTER/LEFT/RIGHT/BOTH
    uint8_t current_width;       // Width in blocks

    // Transition state
    uint8_t in_transition;       // 1 if lanes are connected
    uint8_t transition_counter;  // Blocks remaining in transition zone

} LevelState;
```

**RAM usage: ~20 bytes** (constant regardless of level size)

---

## Memory Budget

| Data | Size | Location |
|------|------|----------|
| LevelSegment | 4 bytes each | ROM |
| LevelObject | 2 bytes each | ROM |
| LevelDef | 8 bytes | ROM |
| LevelState | ~20 bytes | RAM |

**Typical level (50 segments, 100 objects):**
- ROM: 8 + (50 × 4) + (100 × 2) = **408 bytes**
- RAM: **20 bytes**

---

## YAML Level Format

Human-readable level definition format.

### Structure

```yaml
level:
  name: "Level Name"

segments:
  - length: <blocks>        # Duration in blocks (required)
    lanes: <config>         # center | left | right | both (optional, inherits)
    width: <blocks>         # Lane width in blocks (optional, inherits)
    connect: <bool>         # true if transition zone (optional)
    objects:                # List of objects (optional)
      - type: <type>        # hole | laser | powerup | enemy_spawn | speed_zone
        lane: <lane>        # left | right | center
        at: <block>         # Position within segment
        size: <size>        # small | partial | full (optional, default: small)
```

### Example: level1.yaml

```yaml
level:
  name: "Level 1 - First Run"

segments:
  # Starting section - single center lane
  - length: 30
    lanes: center
    width: 4

  # Transition to split lanes
  - length: 4
    lanes: both
    width: 4
    connect: true           # Player can switch lanes here

  # Split lanes with obstacles
  - length: 40
    lanes: both
    width: 3
    objects:
      - type: hole
        lane: left
        at: 10
        size: partial

      - type: hole
        lane: right
        at: 20
        size: partial

      - type: laser
        lane: left
        at: 35

  # Merge to right lane only
  - length: 4
    lanes: both
    width: 3
    connect: true

  - length: 20
    lanes: right
    width: 3
    objects:
      - type: powerup
        lane: right
        at: 10

  # Final section - back to center
  - length: 4
    lanes: both
    width: 4
    connect: true

  - length: 50
    lanes: center
    width: 4
    objects:
      - type: hole
        lane: center
        at: 15
        size: full

      - type: laser
        lane: center
        at: 30

      - type: enemy_spawn
        lane: center
        at: 40
```

### Inheritance Rules

- `lanes` and `width` inherit from previous segment if not specified
- First segment must specify both `lanes` and `width`
- `objects` does not inherit (empty if not specified)
- `connect` defaults to `false`

---

## Generated Header Format

The YAML file is converted to a C header by `tools/generate_level.py`.

### Example: level1.h

```c
#ifndef LEVEL1_H
#define LEVEL1_H

#include "level.h"

// Segment data (ROM)
static const LevelSegment level1_segments[] = {
    //  length  config                           obj_count  obj_offset
    {   30,     SEGMENT_CONFIG(LANE_CENTER, 4),  0,         0   },
    {   4,      SEGMENT_CONFIG(LANE_BOTH, 4),    0,         0   },
    {   40,     SEGMENT_CONFIG(LANE_BOTH, 3),    3,         0   },
    {   4,      SEGMENT_CONFIG(LANE_BOTH, 3),    0,         3   },
    {   20,     SEGMENT_CONFIG(LANE_RIGHT, 3),   1,         3   },
    {   4,      SEGMENT_CONFIG(LANE_BOTH, 4),    0,         4   },
    {   50,     SEGMENT_CONFIG(LANE_CENTER, 4),  3,         4   },
    {   0,      0,                               0,         0   },  // End marker
};

// Object data (ROM)
static const LevelObject level1_objects[] = {
    {   10,  OBJECT_DATA(OBJ_HOLE, OBJ_LANE_LEFT, SIZE_PARTIAL)   },
    {   20,  OBJECT_DATA(OBJ_HOLE, OBJ_LANE_RIGHT, SIZE_PARTIAL)  },
    {   35,  OBJECT_DATA(OBJ_LASER, OBJ_LANE_LEFT, 0)             },
    {   10,  OBJECT_DATA(OBJ_POWERUP, OBJ_LANE_RIGHT, 0)          },
    {   15,  OBJECT_DATA(OBJ_HOLE, OBJ_LANE_CENTER, SIZE_FULL)    },
    {   30,  OBJECT_DATA(OBJ_LASER, OBJ_LANE_CENTER, 0)           },
    {   40,  OBJECT_DATA(OBJ_ENEMY_SPAWN, OBJ_LANE_CENTER, 0)     },
};

// Level definition
static const LevelDef level1_def = {
    "LEVEL 1",
    7,                      // segment_count (excluding end marker)
    7,                      // object_count
    level1_segments,
    level1_objects
};

#endif // LEVEL1_H
```

---

## API Reference

### Initialization

```c
// Initialize level system with a level definition
void level_init(const LevelDef* def);
```

Resets all runtime state and loads the first segment.

### Update

```c
// Called every frame - advances block counter, triggers segment changes
void level_update(void);
```

Should be called once per frame. Handles:
- Advancing block counter based on scroll
- Transitioning to next segment when current ends
- Updating cached boundaries
- Triggering objects

### Boundary Queries

```c
// Get current lane boundaries for collision (single lane or center)
void level_get_boundaries(int16_t* left, int16_t* right);

// For LANE_BOTH: get both lane boundaries
void level_get_both_boundaries(int16_t* l_left, int16_t* l_right,
                               int16_t* r_left, int16_t* r_right);

// Check if player is in transition zone (can switch lanes)
uint8_t level_in_transition(void);
```

### Tilemap Generation

```c
// Generate tilemap row at given scroll position
void level_generate_row(uint8_t row, int16_t scroll_y);
```

Called by tilemap system to generate tiles for a specific row.

### Object Queries

```c
// Check for object at current position
// Returns object type or OBJ_NONE
uint8_t level_check_object(int16_t x, int16_t y);
```

---

## Tile Mapping

### Screen Layout

- Tilemap: 40 tiles wide (320px), 32 tiles visible (256px)
- Tilemap offset: 4 tiles (32px) from left edge
- Center of visible area: tile 20 (160px from tilemap origin)

### Position Calculation

For a centered layout:

```c
// Screen center in tilemap coordinates
#define TILEMAP_CENTER_TILE  20

// Calculate lane positions (in tiles)
// total_width = width * 2 (blocks to tiles) for single lane
// total_width = (width * 2) + (width * 2) + 2 (gap) for both lanes

// LANE_CENTER:
left_tile = TILEMAP_CENTER_TILE - width;
right_tile = TILEMAP_CENTER_TILE + width;

// LANE_BOTH:
// Total span = width + width + 1 (gap)
half_span = width + width + 1;  // in blocks
gap_center = TILEMAP_CENTER_TILE;
left_lane_left = gap_center - 1 - width * 2;
left_lane_right = gap_center - 1;
right_lane_left = gap_center + 1;
right_lane_right = gap_center + 1 + width * 2;

// LANE_LEFT:
// Offset left from center
right_tile = TILEMAP_CENTER_TILE;
left_tile = right_tile - width * 2;

// LANE_RIGHT:
// Offset right from center
left_tile = TILEMAP_CENTER_TILE;
right_tile = left_tile + width * 2;
```

### Tile Indices

Current tile definitions (tileset.h):

| Index | Tile | Description |
|-------|------|-------------|
| 0 | Transparent | Outside lane area |
| 1 | Road surface | Normal road |
| 2 | Road edge left | Left border |
| 3 | Road edge right | Right border |
| 4-7 | Hole tiles | TL, TR, BL, BR |

Additional tiles needed:

| Index | Tile | Description |
|-------|------|-------------|
| 8 | Laser horizontal | Laser beam |
| 9 | Laser glow | Laser source |
| 10-11 | Transition | Diagonal connectors |
| 12-15 | Reserved | Future use |

---

## File Structure

```
sandbox-zx/
├── src/
│   ├── level.h            # Data structures, constants, API declarations
│   ├── level.c            # Runtime level logic
│   ├── tilemap.c          # Modified to use level data
│   ├── player.c           # Dynamic boundary checks
│   └── collision.c        # Level-aware collision
│
├── include/
│   └── level1.h           # Generated from YAML
│
├── levels/
│   └── level1.yaml        # Human-editable level definition
│
├── tools/
│   └── generate_level.py  # YAML to C header converter
│
└── docs/
    └── level-system.md    # This specification
```

---

## Implementation Phases

### Phase 1: Data Structures
1. Create `src/level.h` with all structures and constants
2. Create `src/level.c` with basic init/update logic
3. Create `levels/level1.yaml` sample level

### Phase 2: Tilemap Integration
1. Modify `src/tilemap.c` to use level data
2. Implement `level_generate_row()`
3. Handle lane position calculations

### Phase 3: Collision Updates
1. Modify `src/player.c` for dynamic boundaries
2. Modify `src/collision.c` for level-aware checks
3. Implement transition zone logic

### Phase 4: Generator Tool
1. Create `tools/generate_level.py`
2. Parse YAML format
3. Generate optimized C headers

---

## Future Considerations

- **Multiple levels**: Load different LevelDef pointers
- **Level editor**: Visual tool outputs YAML
- **Checkpoints**: Save segment_idx for respawn
- **Branching paths**: Multiple segment sequences
- **Scrolling speed**: Per-segment speed modifiers
- **Background changes**: Per-segment Layer 2 patterns
