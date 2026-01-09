#ifndef DEBUG_HUD_H
#define DEBUG_HUD_H

#include <stdint.h>

void debug_hud_init(void);
void debug_hud_toggle(void);
uint8_t debug_hud_is_enabled(void);
void debug_hud_clear(void);
void debug_hud_render(void);

#endif
