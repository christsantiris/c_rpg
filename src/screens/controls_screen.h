#ifndef CONTROLS_SCREEN_HEADER_H
#define CONTROLS_SCREEN_HEADER_H

#include "../game/controls.h"

// Row layout shared by the renderer and click handling.
#define CONTROLS_LIST_TOP 100
#define CONTROLS_ROW_H 26
#define CONTROLS_ROW_HALF_W 240

typedef enum {
    CONTROLS_NONE = 0,
    CONTROLS_CLOSED,
    CONTROLS_REBOUND,
    CONTROLS_KEY_RESERVED
} ControlsResult;

typedef struct {
    int selected;
    int waiting_for_key;
    // Command that took the rebound command's old key, or -1.
    int swapped_action;
    int key_refused;
} ControlsScreen;

void controls_screen_init(ControlsScreen *s);
ControlsResult controls_screen_handle_key(ControlsScreen *s, int scancode, int bindings[CONTROL_COUNT]);
void controls_screen_handle_click(ControlsScreen *s, int mouse_x, int mouse_y, int screen_w);

#endif
