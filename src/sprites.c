#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include "game.h"
#include "spriteset.h"

// ZX Spectrum Next I/O ports
#define SPRITE_SLOT_PORT      0x303B
#define SPRITE_ATTR_PORT      0x57
#define SPRITE_PATTERN_PORT   0x5B

// Next register values
#define NEXTREG_SPRITE_SYSTEM  0x15

// 16x16 sprite patterns (256 bytes each, 8-bit per pixel)
// Palette indices 0-15 are ZX Spectrum colors, 0xE3 is transparent
#define C_TRANS  0xE3  // Transparent (magenta)

// ZX Spectrum palette indices (0-15)
#define C_BLACK   0   // Black
#define C_BLUE    1   // Blue
#define C_RED     2   // Red
#define C_MAGENTA 3   // Magenta
#define C_GREEN   4   // Green
#define C_CYAN    5   // Cyan
#define C_YELLOW  6   // Yellow
#define C_WHITE   7   // White
#define C_BRIGHT_BLACK   8   // Bright Black (same as black)
#define C_BRIGHT_BLUE    9   // Bright Blue
#define C_BRIGHT_RED     10  // Bright Red
#define C_BRIGHT_MAGENTA 11  // Bright Magenta
#define C_BRIGHT_GREEN   12  // Bright Green
#define C_BRIGHT_CYAN    13  // Bright Cyan
#define C_BRIGHT_YELLOW  14  // Bright Yellow
#define C_BRIGHT_WHITE   15  // Bright White

// Write to Next register
static void nextreg_write(uint8_t reg, uint8_t val) {
    IO_NEXTREG_REG = reg;
    IO_NEXTREG_DAT = val;
}

// ZX Spectrum palette colors in RGB332 format
static const uint8_t zx_palette[16] = {
    0x00,  // 0: Black
    0x02,  // 1: Blue
    0xC0,  // 2: Red
    0xC2,  // 3: Magenta
    0x18,  // 4: Green
    0x1A,  // 5: Cyan
    0xD8,  // 6: Yellow
    0xDA,  // 7: White
    0x49,  // 8: Dark Gray (RGB332: ~33% each channel)
    0x03,  // 9: Bright Blue
    0xE0,  // 10: Bright Red
    0xE3,  // 11: Bright Magenta (0xE3 = sprite transparent color)
    0x1C,  // 12: Bright Green
    0x1F,  // 13: Bright Cyan
    0xFC,  // 14: Bright Yellow
    0xFF   // 15: Bright White
};

// Set up sprite palette
// Register 0x43 palette control uses:
//   bits 6-4: palette for reading (SELECT)
//   bits 3-1: palette for writing (ENABLE adds to this)
//   bit 0: ULANext enable
// From z88dk: SELECT_SPRITES_PALETTE_0 = 0x20, ENABLE_SPRITES_PALETTE_0 = 0x00
// So to write to sprite palette 0: 0x20 | 0x00 = 0x20
static void sprites_setup_palette(void) {
    uint16_t i;
    uint8_t color;

    // Select sprite palette 0 for writing (0x20)
    IO_NEXTREG_REG = 0x43;
    IO_NEXTREG_DAT = 0x20;

    // Set starting palette index to 0
    IO_NEXTREG_REG = 0x40;
    IO_NEXTREG_DAT = 0x00;

    // Write palette entries using 8-bit format (register 0x41)
    // Auto-increment is enabled by default
    IO_NEXTREG_REG = 0x41;
    for (i = 0; i < 256; i++) {
        if (i < 16) {
            color = zx_palette[i];
        }
        else {
            color = (uint8_t)i;  // RGB332 identity
        }
        IO_NEXTREG_DAT = color;
    }

    // Reset to ULA palette (0x00)
    IO_NEXTREG_REG = 0x43;
    IO_NEXTREG_DAT = 0x00;
}

// Initialize sprite system
void sprites_init(void) {
    // Enable sprites, sprites visible, over border
    nextreg_write(NEXTREG_SPRITE_SYSTEM, 0x03);

    // Set up sprite palette
    sprites_setup_palette();

    // Upload sprite patterns
    sprites_upload_patterns();
}

// Upload sprite patterns to pattern memory
// 8-bit mode: 16x16 = 256 bytes per pattern
void sprites_upload_patterns(void) {
    uint16_t i;

    // Select sprite pattern slot 0
    z80_outp(SPRITE_SLOT_PORT, SPRITE_PLAYER);

    // Upload player sprite (H0 from spriteset)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_H0[i]);
    }

    // Upload bullet sprite (slot 1) - B1 from spriteset
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_B1[i]);
    }

    // Upload enemy animation frames A0-G0 (slots 2-8)
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_A0[i]);
    }
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_B0[i]);
    }
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_C0[i]);
    }
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_D0[i]);
    }
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_E0[i]);
    }
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_F0[i]);
    }
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_G0[i]);
    }

    // Upload player shadow sprite (slot 9) - A1 from spriteset
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_A1[i]);
    }

    // Upload enemy shadow sprite (slot 10) - C1 from spriteset
    for (i = 0; i < 256; i++) {
        z80_outp(SPRITE_PATTERN_PORT, sprite_C1[i]);
    }
}

// Set sprite attributes (5-byte mode for 8-bit sprites)
void sprite_set(uint8_t slot, int16_t x, int16_t y, uint8_t pattern) {
    // Adjust coordinates for sprite offset (sprites are positioned from 32,32)
    x += 32;
    y += 32;

    // Select sprite attribute slot
    z80_outp(SPRITE_SLOT_PORT, slot);

    // Write 5-byte sprite attributes (required for 8-bit sprites)
    // Byte 2: PPPP XM YM R X8
    //   Bits 7-4: Palette offset (0)
    //   Bit 3: X mirror
    //   Bit 2: Y mirror
    //   Bit 1: Rotate 90
    //   Bit 0: X coordinate bit 8
    // Byte 3: V E N5-N0
    //   Bit 7: Visible
    //   Bit 6: E=1 for 5-byte mode
    //   Bits 5-0: Pattern number
    // Byte 4: H N6 T 0 0 0 0 0
    //   Bit 7: H (pattern bit 7)
    //   Bit 6: N6 (pattern bit 6)
    //   Bit 5: T=0 for 8-bit sprites
    //   Bits 4-0: scaling=0 (1x), type=0 (anchor)
    z80_outp(SPRITE_ATTR_PORT, x & 0xFF);           // Byte 0: X low byte
    z80_outp(SPRITE_ATTR_PORT, y & 0xFF);           // Byte 1: Y low byte
    z80_outp(SPRITE_ATTR_PORT, (x >> 8) & 0x01);    // Byte 2: X MSB only
    z80_outp(SPRITE_ATTR_PORT, 0xC0 | (pattern & 0x3F));    // Byte 3: Visible, E=1, pattern[5:0]
    z80_outp(SPRITE_ATTR_PORT, (pattern & 0x40) << 1);      // Byte 4: N6->bit7, T=0, no scaling
}

// Hide a sprite (5-byte mode)
void sprite_hide(uint8_t slot) {
    z80_outp(SPRITE_SLOT_PORT, slot);
    z80_outp(SPRITE_ATTR_PORT, 0);
    z80_outp(SPRITE_ATTR_PORT, 0);
    z80_outp(SPRITE_ATTR_PORT, 0);
    z80_outp(SPRITE_ATTR_PORT, 0);  // Invisible (bit 7 = 0)
    z80_outp(SPRITE_ATTR_PORT, 0);  // Byte 4
}
