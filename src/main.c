#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include "game.h"
#include "sprites.h"
#include "layer2.h"
#include "tilemap.h"
#include "ula.h"
#include "sound.h"

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
    ula_print_at(6, 5,  "     NEBULA 8", ATTR_YELLOW_ON_BLACK);
    ula_print_at(6, 8,  " ZX SPECTRUM NEXT", ATTR_YELLOW_ON_BLACK);
    ula_print_at(6, 12, "PRESS FIRE TO START", ATTR_WHITE_ON_BLACK);
}

// Draw CRASH text
static void draw_crash(void) {
    ula_print_at(12, 11, "CRASH!", ATTR_RED_ON_BLACK);
}

// Draw game over screen
static void draw_gameover(void) {
    uint8_t score_len = 0;
    uint16_t s = game.score;
    uint8_t total_len, x;
    
    ula_clear();
    ula_print_at(6, 10, "     GAME OVER", ATTR_RED_ON_BLACK);
    ula_print_at(6, 16, "PRESS FIRE TO START", ATTR_WHITE_ON_BLACK);

    // Calculate score digit count
    if (s == 0) {
        score_len = 1;
    }
    else {
        while (s > 0) {
            score_len++;
            s /= 10;
        }
    }

    // "SCORE: " is 7 chars + score_len
    total_len = 7 + score_len;
    x = (32 - total_len) / 2;

    ula_print_at(x, 12, "SCORE: ", ATTR_WHITE_ON_BLACK);
    ula_print_num(x + 7, 12, game.score, ATTR_WHITE_ON_BLACK);
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

    // Initialize sprites (palette + patterns)
    sprites_init();

    // Initialize sound
    sound_init();
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
    uint8_t gameover_shown = 0;

    ula_clear();
    ula_print_at(8, 10, "INITIALISING...", ATTR_WHITE_ON_BLACK);

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
                    gameover_shown = 0;
                }
                break;

            case STATE_PLAYING:
                // Check for pause
                if ((input & INPUT_PAUSE) && debounce == 0) {
                    debounce = 15;
                    game.state = STATE_PAUSED;
                    ula_print_at(2, 10, " PAUSED ", ATTR_YELLOW_ON_BLACK);
                    ula_print_at(22, 10, " PAUSED ", ATTR_YELLOW_ON_BLACK);
                    break;
                }
                else {
                    ula_print_at(2, 10, "        ", ATTR_YELLOW_ON_BLACK);
                    ula_print_at(22, 10, "        ", ATTR_YELLOW_ON_BLACK);
                }

                // R key to restart game
                if ((input & INPUT_RESTART) && debounce == 0) {
                    debounce = 15;
                    game_init();
                    break;
                }

                game_update();
                game_render();
                sound_update();

                // Apply shake when shake_timer active (holes or crashes)
                if (game.shake_timer > 0) {
                    apply_shake();
                }

                // Border flash color based on crash type
                if (game.crash_timer > 0) {
                    uint8_t flash_color;
                    switch (game.crash_type) {
                        case CRASH_HOLE:       flash_color = 0x01; break;  // Blue
                        case CRASH_ENEMY:      flash_color = 0x02; break;  // Red
                        case CRASH_ENEMY_FAST: flash_color = 0x02; break;  // Red
                        default:               flash_color = 0x07; break;  // White (level)
                    }
                    z80_outp(0xFE, (game.crash_timer & 0x04) ? flash_color : 0x00);
                }
                else {
                    z80_outp(0xFE, 0x00);
                }
                break;

            case STATE_PAUSED:
                // Wait for unpause
                if ((input & INPUT_PAUSE) && debounce == 0) {
                    debounce = 15;
                    game.state = STATE_PLAYING;
                    // Clear the PAUSED text
                    ula_print_at(12, 11, "        ", ATTR_WHITE_ON_BLACK);
                }
                break;

            case STATE_DYING:
                // Player dead, still on game screen
                // Move enemies, no scrolling, hide player
                game_update_dying();
                game_render_dying();
                sound_update();

                // Show "You lost" on both sides of level
                ula_print_at(2, 10, "YOU LOST", ATTR_RED_ON_BLACK);
                ula_print_at(22, 10, "YOU LOST", ATTR_RED_ON_BLACK);

                // Apply shake if still active
                if (game.shake_timer > 0) {
                    apply_shake();
                }

                // Border flash if still active
                if (game.crash_timer > 0) {
                    z80_outp(0xFE, (game.crash_timer & 0x04) ? 0x02 : 0x00);  // Red flash
                }
                else {
                    z80_outp(0xFE, 0x00);
                }

                // Wait for fire to go to game over screen
                if ((input & INPUT_FIRE) && debounce == 0) {
                    debounce = 15;
                    // Clear text
                    ula_print_at(2, 10, "        ", ATTR_RED_ON_BLACK);
                    ula_print_at(22, 10, "        ", ATTR_RED_ON_BLACK);
                    sound_stop_all();
                    game.state = STATE_GAMEOVER;
                }
                break;

            case STATE_GAMEOVER:
                // Only run setup once when entering this state
                if (!gameover_shown) {
                    gameover_shown = 1;
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
                    gameover_shown = 0;
                }
                break;

            default:
                break;
        }

        if (debounce > 0) debounce--;
    }
}
