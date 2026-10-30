#include "controls.h"
#include <SDL2/SDL.h>

static const int default_bindings[CONTROL_COUNT] = {
    SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D,
    SDL_SCANCODE_PERIOD, SDL_SCANCODE_COMMA, SDL_SCANCODE_P, SDL_SCANCODE_T,
    SDL_SCANCODE_I, SDL_SCANCODE_B, SDL_SCANCODE_Q, SDL_SCANCODE_C,
    SDL_SCANCODE_F, SDL_SCANCODE_H, SDL_SCANCODE_V
};

static const char *labels[CONTROL_COUNT] = {
    "Move up", "Move down", "Move left / interact", "Move right",
    "Descend stairs / exit", "Ascend stairs", "Pick up", "Talk", "Inventory",
    "Spellbook", "Quest journal", "Cast spell", "Fire ranged", "Help", "Drink potion"
};

void controls_reset(int bindings[CONTROL_COUNT]) {
    for (int i = 0; i < CONTROL_COUNT; i++) {
        bindings[i] = default_bindings[i];
    }
}

int controls_key_reserved(int scancode) {
    return scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_NUM_SCANCODES ||
        scancode == SDL_SCANCODE_ESCAPE || scancode == SDL_SCANCODE_UP ||
        scancode == SDL_SCANCODE_DOWN || scancode == SDL_SCANCODE_LEFT ||
        scancode == SDL_SCANCODE_RIGHT;
}

// Returns the command bound to scancode, or -1 when the key is unbound.
int controls_action_for_key(const int bindings[CONTROL_COUNT], int scancode) {
    for (int i = 0; i < CONTROL_COUNT; i++) {
        if (bindings[i] == scancode) {
            return i;
        }
    }
    return -1;
}

// Binds scancode to action. A command already using that key takes over the
// action's previous key, so every command keeps exactly one key.
int controls_assign(int bindings[CONTROL_COUNT], ControlAction action, int scancode) {
    if (action < 0 || action >= CONTROL_COUNT || controls_key_reserved(scancode)) {
        return 0;
    }
    int other = controls_action_for_key(bindings, scancode);
    if (other >= 0) {
        bindings[other] = bindings[action];
    }
    bindings[action] = scancode;
    return 1;
}

int controls_valid(const int bindings[CONTROL_COUNT]) {
    for (int i = 0; i < CONTROL_COUNT; i++) {
        if (controls_key_reserved(bindings[i])) {
            return 0;
        }
        for (int j = 0; j < i; j++) {
            if (bindings[i] == bindings[j]) {
                return 0;
            }
        }
    }
    return 1;
}

const char *controls_label(ControlAction action) {
    if (action < 0 || action >= CONTROL_COUNT) {
        return "";
    }
    return labels[action];
}

int controls_migrate(int bindings[CONTROL_COUNT], const int *saved, int count) {
    if (count != CONTROL_COUNT && count != CONTROL_DRINK_POTION) {
        return 0;
    }
    int candidate[CONTROL_COUNT];
    controls_reset(candidate);
    for (int i = 0; i < count; i++) {
        candidate[i] = saved[i];
    }
    if (count == CONTROL_DRINK_POTION) {
        // Preserve every existing assignment, even when V was already remapped.
        for (int key = SDL_SCANCODE_A; key <= SDL_SCANCODE_Z; key++) {
            if (controls_valid(candidate)) {
                break;
            }
            candidate[CONTROL_DRINK_POTION] = key;
        }
    }
    if (!controls_valid(candidate)) {
        return 0;
    }
    for (int i = 0; i < CONTROL_COUNT; i++) {
        bindings[i] = candidate[i];
    }
    return 1;
}
