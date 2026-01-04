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
#include "game.h"
#include "layer2.h"
#include "tilemap.h"
#include "ula.h"

// Write to Next register
static void nextreg(uint8_t reg, uint8_t val) {
    IO_NEXTREG_REG = reg;
    IO_NEXTREG_DAT = val;
}

// Wait for vertical blank
static void wait_vblank(void) {
    intrinsic_halt();
}

// Draw title screen
static void draw_title(void) {
    ula_clear();
    ula_print_at(10, 5,  "DARK NEBULA", ATTR_YELLOW_ON_BLACK);
    ula_print_at(6, 8,  "ZX SPECTRUM NEXT", ATTR_YELLOW_ON_BLACK);
    ula_print_at(9, 12, "PRESS FIRE", ATTR_WHITE_ON_BLACK);
}

// Draw CRASH text
static void draw_crash(void) {
    ula_print_at(12, 11, "CRASH!", ATTR_RED_ON_BLACK);
}

// Draw game over screen
static void draw_gameover(void) {
    ula_clear();
    ula_print_at(11, 10, "GAME OVER", ATTR_RED_ON_BLACK);
    ula_print_at(9, 12, "SCORE:", ATTR_WHITE_ON_BLACK);
    ula_print_num(16, 12, game.score, ATTR_WHITE_ON_BLACK);
    ula_print_at(8, 16, "PRESS FIRE", ATTR_WHITE_ON_BLACK);
}

// Apply screen shake
static void apply_shake(void) {
    int8_t offset = game_get_shake_offset();
    if (offset != 0) {
        layer2_scroll_x(offset);
    }
}

// Initialize hardware
static void init_next(void) {
    // Enable interrupts
    intrinsic_ei();

    // Set CPU speed (14MHz)
    nextreg(0x07, 0x02);

    // Set border black
    z80_outp(0xFE, 0x00);

    // Initialize graphics layers
    layer2_init();
    tilemap_init();

    // Set layer priority for menu (sprites only)
    set_layers_menu();

    // Upload sprite patterns
    sprites_upload_patterns();
}

// Enable gameplay graphics
static void enable_gameplay(void) {
    layer2_enable();
    tilemap_enable();  
    set_layers_gameplay();  
}

// Disable gameplay graphics (for menus)
static void disable_gameplay(void) {
    layer2_disable();
    tilemap_disable();
    set_layers_menu();
}

// Main function
int main(void) {
    uint8_t input;
    uint8_t debounce = 0;
    uint8_t prev_state = STATE_TITLE;

    // Initialize
    init_next();

    // Start at title screen
    game.state = STATE_TITLE;
    draw_title();

    while (1) {
        wait_vblank();

        input = input_read();

        switch (game.state) {
            case STATE_TITLE:
                if ((input & INPUT_FIRE) && debounce == 0) {
                    debounce = 10;
                    ula_clear();
                    enable_gameplay();
                    game_init();
                }
                break;

            case STATE_PLAYING:
                game_update();
                game_render();

                // Apply shake when hit
                if (game.crash_timer > 0) {
                    apply_shake();
                }
                break;

            case STATE_GAMEOVER:
                // Only run setup once when entering this state
                if (prev_state != STATE_GAMEOVER) {
                    // Hide all sprites
                    for (uint8_t s = 0; s < 32; s++) {
                        sprite_hide(s);
                    }
                    disable_gameplay();
                    draw_gameover();
                }

                if ((input & INPUT_FIRE) && debounce == 0) {
                    debounce = 10;
                    ula_clear();
                    enable_gameplay();
                    game_init();
                }
                break;

            default:
                break;
        }

        prev_state = game.state;
        if (debounce > 0) debounce--;
    }

    return 0;
}
