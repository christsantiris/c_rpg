#include "spell.h"
#include <string.h>

Spell spell_make_magic_arrow(void) {
    Spell s = {0};
    strncpy(s.name, "Magic Arrow", sizeof(s.name) - 1);
    s.id      = SPELL_MAGIC_ARROW;
    s.type    = SPELL_TYPE_DAMAGE_RANGED;
    s.mp_cost = 10;
    s.damage  = 15;
    s.range   = 6;
    s.radius  = 0;
    s.heal_hp = 0;
    return s;
}

Spell spell_make_fireball(void) {
    Spell s = {0};
    strncpy(s.name, "Fireball", sizeof(s.name) - 1);
    s.id      = SPELL_FIREBALL;
    s.type    = SPELL_TYPE_DAMAGE_AREA;
    s.mp_cost = 20;
    s.damage  = 25;
    s.range   = 4;
    s.radius  = 2;
    s.heal_hp = 0;
    return s;
}

Spell spell_make_heal(void) {
    Spell s = {0};
    strncpy(s.name, "Heal", sizeof(s.name) - 1);
    s.id      = SPELL_HEAL;
    s.type    = SPELL_TYPE_HEAL;
    s.mp_cost = 15;
    s.damage  = 0;
    s.range   = 0;
    s.radius  = 0;
    s.heal_hp = 40;
    return s;
}