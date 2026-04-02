#ifndef SPELLBOOK_HEADER_H
#define SPELLBOOK_HEADER_H

typedef enum {
    SPELLBOOK_NONE = 0,
    SPELLBOOK_CLOSED,
    SPELLBOOK_EQUIP
} SpellbookResult;

typedef struct {
    int selected;
} SpellbookScreen;

void            spellbook_init(SpellbookScreen *s);
SpellbookResult spellbook_handle_key(SpellbookScreen *s, int scancode,
                                     int spell_count);

#endif