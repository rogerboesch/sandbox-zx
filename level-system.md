# ZX Spectrum Next Tilemap Scrolling System

## Hardware Overview

- Tilemap: 40x32 tiles (320x256 pixels)
- ULA visible area: 256x192 pixels
- Tile size: 8x8 pixels (4-bit, 32 bytes per tile)
- 4 tile rows (32px) above ULA visible area
- 4 tile rows (32px) below ULA visible area

## Scroll Mechanics

### Scroll Direction

- `scroll_y--` makes content move **DOWN** on screen
- New content appears at the **TOP**
- `scroll_y` is 0 or negative (0, -1, -2, ... -255, wraps via & 0xFF)

### Row at Top Calculation

```c
uint8_t top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);
```

Examples:
- `scroll_y = 0`: row 0 at tilemap top (row 1 at visible ULA top)
- `scroll_y = -8`: row 31 at tilemap top, row 0 moved down 8px
- `scroll_y = -16`: row 30 at tilemap top

### World Position Calculation

```c
static int16_t calc_world_y_for_row(uint8_t row) {
    uint8_t top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);
    uint8_t rows_from_top = (row - top_row) & 0x1F;
    int16_t world_y = (-scroll_y) + (rows_from_top * 8);
    return world_y;
}
```

### Row Regeneration

When scrolling, regenerate rows that appear at the top:

```c
uint8_t old_top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);
// ... update scroll_y ...
uint8_t new_top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);

if (new_top_row != old_top_row) {
    // Regenerate from new_top_row going down
    for (row = 0; row < row_diff; row++) {
        uint8_t gen_row = (new_top_row + row) & 0x1F;
        tilemap_generate_row(gen_row);
    }
}
```

## Tilemap Clip Window

Register **0x1B** controls the tilemap clip window. Values are written sequentially:
1. X1 (left)
2. X2 (right)
3. Y1 (top)
4. Y2 (bottom)

### Clip to ULA area only:
```c
ZXN_NEXTREG(0x1B, 0);     // X1
ZXN_NEXTREG(0x1B, 255);   // X2
ZXN_NEXTREG(0x1B, 32);    // Y1 - skip 4 rows (32px)
ZXN_NEXTREG(0x1B, 223);   // Y2 - stop before bottom 4 rows
```

### Clip to ULA + half border:
```c
ZXN_NEXTREG(0x1B, 0);     // X1
ZXN_NEXTREG(0x1B, 255);   // X2
ZXN_NEXTREG(0x1B, 16);    // Y1 - skip 2 rows (16px)
ZXN_NEXTREG(0x1B, 239);   // Y2 - show 2 rows in bottom border
```

## Important Registers

| Register | Name | Purpose |
|----------|------|---------|
| 0x1B | Clip Window Tilemap | X1, X2, Y1, Y2 (sequential writes) |
| 0x31 | Tilemap Y Scroll | Vertical scroll offset (0-255) |
| 0x6B | Tilemap Control | Enable, mode, 8-bit entries |
| 0x6C | Tilemap Attribute | Default palette/flags for 8-bit mode |
| 0x6E | Tilemap Base | Address MSB for tilemap data |
| 0x6F | Tilemap Tiles | Address MSB for tile definitions |
| 0x4C | Tilemap Transparency | Transparent palette index |

## Memory Layout

- Tilemap data: 0x6000 (40x32 = 1280 bytes for 8-bit entries)
- Tile definitions: 0x6600 (32 bytes per tile)

## Key Findings from Testing

1. Row 0 is just **above** the visible ULA area when scroll_y = 0
2. Row 1 is at the **top** of the visible ULA screen when scroll_y = 0
3. Hardware scroll register takes value `scroll_y & 0xFF`
4. Negative scroll values wrap: -8 becomes 248 (0xF8)
5. The tilemap wraps vertically - row 31 is followed by row 0
