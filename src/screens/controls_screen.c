#include "controls_screen.h"
#include <SDL2/SDL.h>

void controls_screen_init(ControlsScreen *s) {
    s->selected = 0;
    s->waiting_for_key = 0;
    s->swapped_action = -1;
    s->key_refused = 0;
}

static void begin_rebind(ControlsScreen *s) {
    s->waiting_for_key = 1;
    s->swapped_action = -1;
    s->key_refused = 0;
}

// While waiting, the next key rebinds the selected command; Escape cancels
// and a fixed key is refused so the player can try another.
static ControlsResult handle_rebind_key(ControlsScreen *s, int scancode, int bindings[CONTROL_COUNT]) {
    if (scancode == SDL_SCANCODE_ESCAPE) {
        s->waiting_for_key = 0;
        s->key_refused = 0;
        return CONTROLS_NONE;
    }
    if (controls_key_reserved(scancode)) {
        s->key_refused = 1;
        return CONTROLS_KEY_RESERVED;
    }
    int other = controls_action_for_key(bindings, scancode);
    controls_assign(bindings, (ControlAction)s->selected, scancode);
    s->swapped_action = other == s->selected ? -1 : other;
    s->waiting_for_key = 0;
    s->key_refused = 0;
    return CONTROLS_REBOUND;
}

ControlsResult controls_screen_handle_key(ControlsScreen *s, int scancode, int bindings[CONTROL_COUNT]) {
    if (s->waiting_for_key) {
        return handle_rebind_key(s, scancode, bindings);
    }
    switch (scancode) {
        case SDL_SCANCODE_UP:
            if (s->selected > 0) {
                s->selected--;
            }
            break;
        case SDL_SCANCODE_DOWN:
            if (s->selected < CONTROL_COUNT - 1) {
                s->selected++;
            }
            break;
        case SDL_SCANCODE_RETURN:
            begin_rebind(s);
            break;
        case SDL_SCANCODE_ESCAPE:
            return CONTROLS_CLOSED;
        default:
            break;
    }
    return CONTROLS_NONE;
}

// Clicking a row starts rebinding it; clicking during a rebind cancels it.
void controls_screen_handle_click(ControlsScreen *s, int mouse_x, int mouse_y, int screen_w) {
    if (s->waiting_for_key) {
        s->waiting_for_key = 0;
        s->key_refused = 0;
        return;
    }
    int center_x = screen_w / 2;
    if (mouse_x < center_x - CONTROLS_ROW_HALF_W || mouse_x > center_x + CONTROLS_ROW_HALF_W ||
        mouse_y < CONTROLS_LIST_TOP) {
        return;
    }
    int row = (mouse_y - CONTROLS_LIST_TOP) / CONTROLS_ROW_H;
    if (row < CONTROL_COUNT) {
        s->selected = row;
        begin_rebind(s);
    }
}
