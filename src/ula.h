#ifndef ULA_H
#define ULA_H

#include <stdint.h>

// ULA attribute colors
#define ULA_BLACK        0x00
#define ULA_BLUE         0x01
#define ULA_RED          0x02
#define ULA_MAGENTA      0x03
#define ULA_GREEN        0x04
#define ULA_CYAN         0x05
#define ULA_YELLOW       0x06
#define ULA_WHITE        0x07

// ULA attribute helpers (paper << 3 | ink)
#define ULA_ATTR(paper, ink)  (((paper) << 3) | (ink))
#define ULA_BRIGHT            0x40

// Common attributes
#define ATTR_WHITE_ON_BLACK   (ULA_ATTR(ULA_BLACK, ULA_WHITE) | ULA_BRIGHT)
#define ATTR_YELLOW_ON_BLACK  (ULA_ATTR(ULA_BLACK, ULA_YELLOW) | ULA_BRIGHT)
#define ATTR_RED_ON_BLACK     (ULA_ATTR(ULA_BLACK, ULA_RED) | ULA_BRIGHT)
#define ATTR_GREEN_ON_BLACK   (ULA_ATTR(ULA_BLACK, ULA_GREEN) | ULA_BRIGHT)
#define ATTR_YELLOW_ON_BLUE   (ULA_ATTR(ULA_BLUE, ULA_YELLOW) | ULA_BRIGHT)

// Clear ULA screen (pixels and attributes)
void ula_clear(void);

// Set all attributes to a color
void ula_set_attr(uint8_t attr);

// Print string at position with attribute
void ula_print_at(uint8_t x, uint8_t y, const char *str, uint8_t attr);

// Print number at position with attribute
void ula_print_num(uint8_t x, uint8_t y, uint16_t num, uint8_t attr);

#endif // ULA_H
