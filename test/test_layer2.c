// Minimal Layer 2 test - just fill screen with color
#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include "ula.h"

#define L2_HEIGHT 192  // Visible height in 256x192 mode

int main(void) {
    uint8_t l2_bank;
    uint16_t i;
    uint8_t *ptr;
    uint8_t scroll_y = 0;

    // Disable interrupts during Layer 2 bank manipulation
    // to prevent IM1 handler from corrupting Layer 2 memory
    intrinsic_di();

    // 14MHz
    IO_NEXTREG_REG = 0x07;
    IO_NEXTREG_DAT = 0x02;

    // Blue border to confirm we're running
    z80_outp(0xFE, 0x01);

    // Clear ULA screen
    ula_clear();

    // Set Layer 2 RAM to 16K bank 8
    IO_NEXTREG_REG = 0x12;
    IO_NEXTREG_DAT = 8;

    // Fill Layer 2 banks - 6 banks for 256x192 mode
    // First bank red, rest transparent (0xE3)
    for (l2_bank = 0; l2_bank < 6; l2_bank++) {
        uint8_t fill_color = (l2_bank == 0) ? 0xE0 : 0xE3;

        // Map Layer 2 bank to slot 2 (0x4000)
        IO_NEXTREG_REG = 0x52;
        IO_NEXTREG_DAT = 16 + l2_bank;

        ptr = (uint8_t *)0x4000;
        for (i = 0; i < 8192; i++) {
            *ptr++ = fill_color;
        }
    }

    // Restore MMU slot 2 BEFORE enabling interrupts
    IO_NEXTREG_REG = 0x52;
    IO_NEXTREG_DAT = 10;

    // Now safe to enable interrupts
    intrinsic_ei();

    // Enable Layer 2 in 256x192 mode
    // Bit 7 = enable, bits 5-4 = 00 for 256x192
    IO_NEXTREG_REG = 0x69;
    IO_NEXTREG_DAT = 0x80;

    // Scroll = 0
    IO_NEXTREG_REG = 0x17;
    IO_NEXTREG_DAT = 0;

    // Main loop - hold SPACE to scroll continuously
    while (1) {
        uint8_t space_pressed;

        intrinsic_halt();

        space_pressed = (z80_inp(0x7FFE) & 0x01) == 0;

        if (space_pressed) {
            // Wrap scroll within 0-191 range
            if (scroll_y == 0) {
                scroll_y = L2_HEIGHT - 1;  // 191
            } else {
                scroll_y--;
            }
            IO_NEXTREG_REG = 0x17;
            IO_NEXTREG_DAT = scroll_y;
        }

        // Display scroll_y value (0-191)
        ula_print_at(0, 23, "SCROLL:                ", ATTR_WHITE_ON_BLACK);
        ula_print_num(8, 23, scroll_y, ATTR_YELLOW_ON_BLACK);
    }

    return 0;
}
