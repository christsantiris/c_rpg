#include "spellbook.h"
#include <SDL2/SDL.h>

void spellbook_init(SpellbookScreen *s) {
    s->selected = 0;
}

SpellbookResult spellbook_handle_key(SpellbookScreen *s, int scancode,
                                      int spell_count) {
    switch (scancode) {
        case SDL_SCANCODE_UP:
            s->selected--;
            if (s->selected < 0) s->selected = 0;
            break;
        case SDL_SCANCODE_DOWN:
            s->selected++;
            if (s->selected >= spell_count)
                s->selected = spell_count - 1;
            break;
        case SDL_SCANCODE_RETURN:
            if (spell_count > 0) return SPELLBOOK_EQUIP;
            break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_A:
            return SPELLBOOK_CLOSED;
        default:
            break;
    }
    return SPELLBOOK_NONE;
}