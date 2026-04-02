#ifndef SPELL_HEADER_H
#define SPELL_HEADER_H

#define MAX_SPELLS 10

typedef enum {
    SPELL_MAGIC_ARROW = 0,
    SPELL_FIREBALL,
    SPELL_HEAL
} SpellID;

typedef enum {
    SPELL_TYPE_DAMAGE_RANGED,
    SPELL_TYPE_DAMAGE_AREA,
    SPELL_TYPE_HEAL
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

#endif