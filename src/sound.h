#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

// Initialize sound system
void sound_init(void);

// Sound effects
void sound_fire(void);       // Bullet fire
void sound_explosion(void);  // Enemy explosion
void sound_hole(void);       // Hit hole

// Update sound (call each frame to handle sound decay)
void sound_update(void);

// Stop all sounds
void sound_stop_all(void);

#endif
