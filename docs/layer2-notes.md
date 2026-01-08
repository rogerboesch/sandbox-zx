# Layer 2 Technical Notes

## Overview

Layer 2 on ZX Spectrum Next provides a 256-color bitmap layer that can be used for backgrounds with parallax scrolling.

## Modes

- **256x192** (default): 6 x 8K banks = 48K, register 0x69 = 0x80
- **256x256**: 8 x 8K banks = 64K, register 0x69 = 0xB0

## Memory Layout

- Layer 2 uses 8K banks 16-21 (256x192) or 16-23 (256x256)
- Each bank = 32 scanlines (256 bytes per line, 32 * 256 = 8192 bytes)
- Banks are mapped to MMU slot 2 (0x4000-0x5FFF) for writing
- Register 0x12 sets the starting 16K bank (value 8 = 8K banks 16-17)

## Scrolling

### The Jump Problem

In 256x192 mode, the scroll register (0x17) is 8-bit (0-255), but the visible area is only 192 lines. This causes a 64-pixel "jump" when the scroll value wraps from 191 to 192 (or 255 to 0).

**Solution**: Manually wrap the scroll value within the 0-191 range:

```c
void layer2_scroll(int16_t offset_y) {
    uint8_t scroll = (uint8_t)(offset_y % 192);
    IO_NEXTREG_REG = 0x17;
    IO_NEXTREG_DAT = scroll;
}
```

### 256x256 Mode Alternative

Using 256x256 mode with 8 banks allows the full 0-255 scroll range to wrap cleanly. However, this uses more memory (64K vs 48K) and the bottom 64 lines (192-255) aren't visible on the standard display.

## Interrupt Corruption Issue

### Problem

When writing to Layer 2 banks via MMU slot 2 (0x4000), the IM1 interrupt handler can fire and corrupt Layer 2 memory. This manifests as random colored dots appearing on the Layer 2 display.

### Cause

The z88dk default IM1 interrupt handler writes to memory in the 0x4000 region. When a Layer 2 bank is mapped to slot 2 during an interrupt, the handler writes to Layer 2 VRAM instead of its intended target.

### Solution

Disable interrupts during all Layer 2 bank manipulation:

```c
void layer2_init(void) {
    // Disable interrupts during bank manipulation
    intrinsic_di();

    // ... map banks and write to Layer 2 ...

    // Restore MMU slot 2 BEFORE enabling interrupts
    IO_NEXTREG_REG = 0x52;
    IO_NEXTREG_DAT = 10;  // Default bank

    // Now safe to enable interrupts
    intrinsic_ei();
}
```

**Important**: Always restore MMU slot 2 to its original bank before re-enabling interrupts.

## Registers Reference

| Register | Purpose |
|----------|---------|
| 0x12 | Layer 2 RAM bank (16K bank number) |
| 0x16 | Layer 2 X scroll offset |
| 0x17 | Layer 2 Y scroll offset |
| 0x52 | MMU slot 2 bank mapping |
| 0x69 | Layer 2 control (enable, mode, palette offset) |

## Test Program

See `test/test_layer2.c` for a minimal test program that demonstrates Layer 2 scrolling with the fixes applied.
