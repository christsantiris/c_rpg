#include "name_entry.h"
#include <string.h>
#include <SDL2/SDL.h>

void name_entry_init(NameEntry *n) {
    memset(n->name, 0, sizeof(n->name));
    n->length = 0;
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

    // Accept a-z, A-Z, space
    if (n->length < MAX_NAME_LEN && keyname && keyname[1] == '\0') {
        char c = keyname[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ' ') {
            n->name[n->length++] = c;
            n->name[n->length]   = '\0';
        }
    }

    return NAME_ENTRY_NONE;
}