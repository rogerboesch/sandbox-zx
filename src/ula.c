/*
 * Dark Nebula - ZX Spectrum Next
 * ula.c - ULA Screen Text Functions
 *
 * ULA is used for title screen and game over text
 */

#include <stdint.h>
#include <string.h>
#include "ula.h"

// Screen addresses
#define SCREEN_ADDR  0x4000
#define ATTR_ADDR    0x5800

// Clear ULA screen
void ula_clear(void) {
    memset((void *)SCREEN_ADDR, 0, 6144);
    memset((void *)ATTR_ADDR, 0x00, 768);
}

// Fill all attributes with a color
void ula_set_attr(uint8_t attr) {
    memset((void *)ATTR_ADDR, attr, 768);
}

// Print string at position with attribute
void ula_print_at(uint8_t x, uint8_t y, const char *str, uint8_t attr) {
    uint8_t *scr = (uint8_t *)(SCREEN_ADDR + ((y & 0x18) << 8) + ((y & 0x07) << 5) + x);
    uint8_t *atr = (uint8_t *)(ATTR_ADDR + (y << 5) + x);

    while (*str) {
        uint8_t *font = (uint8_t *)(0x3D00 + ((*str - 32) << 3));
        for (uint8_t i = 0; i < 8; i++) {
            *(scr + (i << 8)) = font[i];
        }
        *atr++ = attr;
        scr++;
        str++;
    }
}

// Print number at position with attribute
void ula_print_num(uint8_t x, uint8_t y, uint16_t num, uint8_t attr) {
    char buf[6];
    uint8_t i = 0;

    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0 && i < 5) {
            buf[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    buf[i] = '\0';

    // Reverse
    for (uint8_t j = 0; j < i / 2; j++) {
        char t = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = t;
    }

    ula_print_at(x, y, buf, attr);
}
