/*
 * Dark Nebula - ZX Spectrum Next Prototype
 * main.c - Entry point and main loop
 *
 * Controls:
 *   Q = Up, A = Down, O = Left, P = Right
 *   Space = Fire
 *   Kempston joystick supported
 */

#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <string.h>
#include "game.h"
#include "tiles.h"

// Screen addresses
#define SCREEN_ADDR  0x4000
#define ATTR_ADDR    0x5800

// Write to Next register
static void nextreg(uint8_t reg, uint8_t val) {
    IO_NEXTREG_REG = reg;
    IO_NEXTREG_DAT = val;
}

// Wait for vertical blank
static void wait_vblank(void) {
    intrinsic_halt();
}

// Print string at position
static void print_at(uint8_t x, uint8_t y, const char *str) {
    uint8_t *scr = (uint8_t *)(SCREEN_ADDR + ((y & 0x18) << 8) + ((y & 0x07) << 5) + x);
    uint8_t *attr = (uint8_t *)(ATTR_ADDR + (y << 5) + x);

    while (*str) {
        uint8_t *font = (uint8_t *)(0x3D00 + ((*str - 32) << 3));
        for (uint8_t i = 0; i < 8; i++) {
            *(scr + (i << 8)) = font[i];
        }
        *attr++ = 0x47;  // White on black
        scr++;
        str++;
    }
}

// Print number
static void print_num(uint8_t x, uint8_t y, uint16_t num) {
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

    print_at(x, y, buf);
}

// Clear screen
static void clear_screen(void) {
    memset((void *)SCREEN_ADDR, 0, 6144);
    memset((void *)ATTR_ADDR, 0x00, 768);
}

// Fill attributes with color
static void fill_attrs(uint8_t attr) {
    memset((void *)ATTR_ADDR, attr, 768);
}

// Draw title using ULA
static void draw_title(void) {
    clear_screen();
    fill_attrs(0x46);  // Yellow on black
    print_at(10, 5,  "DARK NEBULA");
    print_at(6, 8,  "ZX SPECTRUM NEXT");
    print_at(9, 12, "PRESS START");
}

// Draw HUD using ULA
static void draw_hud(void) {
    uint8_t *attr = (uint8_t *)ATTR_ADDR;
    uint8_t i;

    for (i = 0; i < 32; i++) attr[i] = 0x47;

    print_at(0, 0, "LIVES:");
    for (i = 0; i < player.lives && i < 3; i++) {
        print_at(7 + i, 0, "*");
    }
    print_at(20, 0, "SCORE:");
    print_num(27, 0, game.score);
}

// Draw CRASH text using ULA
static void draw_crash(void) {
    uint8_t *attr = (uint8_t *)(ATTR_ADDR + (11 * 32) + 12);
    uint8_t i;
    print_at(12, 11, "CRASH!");
    for (i = 0; i < 6; i++) attr[i] = 0x42;  // Red
}

// Apply screen shake
static void apply_shake(void) {
    int8_t offset = game_get_shake_offset();
    if (offset != 0) {
        layer2_scroll_x(offset);
    }
}

// Draw game over using ULA
static void draw_gameover(void) {
    uint8_t *attr = (uint8_t *)(ATTR_ADDR + (10 * 32) + 11);
    uint8_t i;
    print_at(11, 10, "GAME OVER");
    print_at(9, 12, "SCORE:");
    print_num(16, 12, game.score);
    print_at(8, 16, "PRESS START");
    for (i = 0; i < 9; i++) attr[i] = 0x42;  // Red
}

// Initialize hardware
static void init_next(void) {
    // Enable interrupts
    intrinsic_ei();

    // Set CPU speed (14MHz)
    nextreg(0x07, 0x02);

    // Set border black
    z80_outp(0xFE, 0x00);

    // Initialize Layer 2 scrolling background
    tiles_init();

    // Enable sprites over Layer 2
    nextreg(0x15, 0x03);

    // Upload sprite patterns
    sprites_upload_patterns();
}

// Main function
int main(void) {
    uint8_t input;
    uint8_t title_mode = 1;
    uint8_t debounce = 0;

    // Initialize
    init_next();

    // Show title
    draw_title();

    while (1) {
        wait_vblank();

        input = input_read();

        if (title_mode) {
            if ((input & INPUT_FIRE) && debounce == 0) {
                title_mode = 0;
                debounce = 10;
                clear_screen();
                fill_attrs(0x00);  // Black
                layer2_enable();   // Enable Layer 2 for gameplay
                game_init();
            }
        } else {
            if (game.state == STATE_PLAYING) {
                game_update();
                game_render();
                // HUD is now rendered via sprites in game_render()

                // Apply shake when hit
                if (game.crash_timer > 0) {
                    apply_shake();
                }
            } else if (game.state == STATE_GAMEOVER) {
                // Hide all sprites (including HUD)
                for (uint8_t s = 0; s < 32; s++) {
                    sprite_hide(s);
                }
                layer2_disable();  // Disable Layer 2 so text shows
                draw_gameover();

                if ((input & INPUT_FIRE) && debounce == 0) {
                    debounce = 10;
                    clear_screen();
                    fill_attrs(0x00);
                    layer2_enable();  // Re-enable for new game
                    game_init();
                }
            }
        }

        if (debounce > 0) debounce--;
    }

    return 0;
}
