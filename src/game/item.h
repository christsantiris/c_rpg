#ifndef ITEM_HEADER_H
#define ITEM_HEADER_H

#define MAX_INVENTORY   10
#define MAX_FLOOR_ITEMS 20

typedef enum {
    ITEM_NONE = 0,
    ITEM_POTION_HEALTH,
    ITEM_POTION_MANA,
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_SCROLL
} ItemType;

typedef struct {
    int      active;
    ItemType type;
    char     name[32];
    int      heal_hp;
    int      heal_mp;
    int      attack_bonus;
    int      defense_bonus;
    int      value;
    int      spell_id;
    int is_ranged;
    int range;
    int is_two_handed;
} Item;

typedef struct {
    int  active;
    int  x, y;
    Item item;
} FloorItem;

Item item_make_health_potion(void);
Item item_make_mana_potion(void);
Item item_make_weapon(const char *name, int attack_bonus, int value);
Item item_make_armor(const char *name, int defense_bonus, int value);

Item item_make_scroll_magic_arrow(void);
Item item_make_scroll_fireball(void);
Item item_make_scroll_heal(void);

Item item_make_rusty_sword(void);
Item item_make_short_sword(void);
Item item_make_long_sword(void);
Item item_make_battle_axe(void);
Item item_make_staff(void);
Item item_make_bow(void);

#endif