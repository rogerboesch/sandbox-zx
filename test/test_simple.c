/*
 * Simple test program for ZX Spectrum Next
 * Just shows colored screen and cycles border
 */

#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <string.h>

#define SCREEN_ADDR  0x4000
#define ATTR_ADDR    0x5800

int main(void) {
    uint8_t color = 0;
    uint16_t i;

    // Enable interrupts
    intrinsic_ei();

    // Clear screen
    memset((void *)SCREEN_ADDR, 0, 6144);

    // Fill attributes with cyan
    memset((void *)ATTR_ADDR, 0x45, 768);

    // Write some text pattern to screen
    for (i = 0; i < 32; i++) {
        *((uint8_t *)(SCREEN_ADDR + i)) = 0xFF;
        *((uint8_t *)(SCREEN_ADDR + 256 + i)) = 0xAA;
        *((uint8_t *)(SCREEN_ADDR + 512 + i)) = 0x55;
    }

    // Main loop - cycle border color
    while (1) {
        intrinsic_halt();
        z80_outp(0xFE, color & 0x07);
        color++;
    }

    return 0;
}
