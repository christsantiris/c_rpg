#ifndef SPELL_HEADER_H
#define SPELL_HEADER_H

#define MAX_SPELLS 10

typedef enum {
    SPELL_MAGIC_ARROW = 0,
    SPELL_FIREBALL,
    SPELL_HEAL,
    SPELL_RETURN_TO_TOWN
} SpellID;

typedef enum {
    SPELL_TYPE_DAMAGE_RANGED,
    SPELL_TYPE_DAMAGE_AREA,
    SPELL_TYPE_HEAL,
    SPELL_TYPE_UTILITY
} SpellType;

typedef struct {
    char      name[32];
    SpellID   id;
    SpellType type;
    int       mp_cost;
    int       damage;
    int       heal_hp;
    int       range;
    int       radius;
} Spell;

Spell spell_make_magic_arrow(void);
Spell spell_make_fireball(void);
Spell spell_make_heal(void);
Spell spell_make_return_to_town(void);

#endif
