#include "name_entry.h"
#include <string.h>
#include <SDL2/SDL.h>

void name_entry_init(NameEntry *n) {
    memset(n->name, 0, sizeof(n->name));
    n->length             = 0;
    n->cursor_last_blink  = SDL_GetTicks();
    n->cursor_visible     = 1;
}

void name_entry_update(NameEntry *n) {
    Uint32 now = SDL_GetTicks();
    if (now - n->cursor_last_blink >= 500) {
        n->cursor_visible    = !n->cursor_visible;
        n->cursor_last_blink = now;
    }
}

NameEntryResult name_entry_handle_key(NameEntry *n, int scancode,
                                       const char *keyname) {
    if (scancode == SDL_SCANCODE_ESCAPE)  return NAME_ENTRY_CANCELLED;

    if (scancode == SDL_SCANCODE_RETURN) {
        if (n->length > 0) return NAME_ENTRY_CONFIRMED;
        return NAME_ENTRY_NONE;
    }

    if (scancode == SDL_SCANCODE_BACKSPACE) {
        if (n->length > 0)
            n->name[--n->length] = '\0';
        return NAME_ENTRY_NONE;
    }

    // Accept a-z, A-Z
    if (n->length < MAX_NAME_LEN && keyname && keyname[1] == '\0') {
        char c = keyname[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            n->name[n->length++] = c;
            n->name[n->length]   = '\0';
        }
    }

    // Accept spacebar explicitly
    if (scancode == SDL_SCANCODE_SPACE && n->length > 0 &&
        n->length < MAX_NAME_LEN) {
        n->name[n->length++] = ' ';
        n->name[n->length]   = '\0';
    }

    return NAME_ENTRY_NONE;
}