#include <arch/zxn.h>
#include <z80.h>
#include <stdint.h>
#include "sound.h"

// AY-3-8912 ports
#define AY_REG_PORT   0xFFFD
#define AY_DATA_PORT  0xBFFD

// AY registers
#define AY_TONE_A_LO    0
#define AY_TONE_A_HI    1
#define AY_TONE_B_LO    2
#define AY_TONE_B_HI    3
#define AY_TONE_C_LO    4
#define AY_TONE_C_HI    5
#define AY_NOISE        6
#define AY_MIXER        7
#define AY_VOL_A        8
#define AY_VOL_B        9
#define AY_VOL_C        10
#define AY_ENV_LO       11
#define AY_ENV_HI       12
#define AY_ENV_SHAPE    13

// Sound effect durations (frames)
static uint8_t fire_timer = 0;
static uint8_t explosion_timer = 0;
static uint8_t hole_timer = 0;

// Write to AY register
static void ay_write(uint8_t reg, uint8_t val) {
    z80_outp(AY_REG_PORT, reg);
    z80_outp(AY_DATA_PORT, val);
}

// Initialize sound system
void sound_init(void) {
    // Silence all channels
    ay_write(AY_VOL_A, 0);
    ay_write(AY_VOL_B, 0);
    ay_write(AY_VOL_C, 0);

    // Mixer: all channels off (bits set = disabled)
    ay_write(AY_MIXER, 0x3F);
}

// Stop all sounds
void sound_stop_all(void) {
    fire_timer = 0;
    explosion_timer = 0;
    hole_timer = 0;

    ay_write(AY_VOL_A, 0);
    ay_write(AY_VOL_B, 0);
    ay_write(AY_VOL_C, 0);
    ay_write(AY_MIXER, 0x3F);
}

// Bullet fire - short high beep (channel A)
void sound_fire(void) {
    // High pitch tone (~2000 Hz)
    ay_write(AY_TONE_A_LO, 0x6E);
    ay_write(AY_TONE_A_HI, 0x00);

    // Enable tone A only
    ay_write(AY_MIXER, 0x3E);

    // Max volume
    ay_write(AY_VOL_A, 15);

    fire_timer = 6;
}

// Explosion - low tone (channel A, same as fire but lower pitch)
void sound_explosion(void) {
    // Cancel any fire sound (they share channel A)
    fire_timer = 0;

    // Low pitch tone
    ay_write(AY_TONE_A_LO, 0x00);
    ay_write(AY_TONE_A_HI, 0x03);

    // Enable tone A only (same as fire)
    ay_write(AY_MIXER, 0x3E);

    // Max volume
    ay_write(AY_VOL_A, 15);

    explosion_timer = 20;
}

// Hit hole - low descending tone (channel C)
void sound_hole(void) {
    // Low pitch tone
    ay_write(AY_TONE_C_LO, 0x00);
    ay_write(AY_TONE_C_HI, 0x02);

    // Enable tone C only
    ay_write(AY_MIXER, 0x3B);

    // Max volume
    ay_write(AY_VOL_C, 15);

    hole_timer = 12;
}

// Update sound - handle decay (call each frame)
void sound_update(void) {
    // Fire sound decay
    if (fire_timer > 0) {
        fire_timer--;
        if (fire_timer == 0) {
            ay_write(AY_VOL_A, 0);
        }
        else {
            // Stay loud, decay at end
            ay_write(AY_VOL_A, (fire_timer > 2) ? 15 : fire_timer * 5);
        }
    }

    // Explosion decay (uses channel A)
    if (explosion_timer > 0) {
        explosion_timer--;
        if (explosion_timer == 0) {
            ay_write(AY_VOL_A, 0);
        }
        else {
            // Simple decay
            uint8_t vol = (explosion_timer > 10) ? 15 : explosion_timer;
            ay_write(AY_VOL_A, vol);
        }
    }

    // Hole sound decay with pitch descent
    if (hole_timer > 0) {
        hole_timer--;
        if (hole_timer == 0) {
            ay_write(AY_VOL_C, 0);
        }
        else {
            // Lower pitch as it decays, stay loud
            ay_write(AY_TONE_C_HI, 0x02 + (12 - hole_timer) / 3);
            ay_write(AY_VOL_C, (hole_timer > 4) ? 15 : hole_timer * 3);
        }
    }
}
