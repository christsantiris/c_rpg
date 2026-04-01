#ifndef ITEM_HEADER_H
#define ITEM_HEADER_H

#define MAX_INVENTORY   10
#define MAX_FLOOR_ITEMS 20

typedef enum {
    ITEM_NONE = 0,
    ITEM_POTION_HEALTH,
    ITEM_POTION_MANA,
    ITEM_WEAPON,
    ITEM_ARMOR
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

#endif