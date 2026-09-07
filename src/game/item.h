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

typedef enum {
    WEAPON_FAMILY_NONE = 0,
    WEAPON_FAMILY_SWORD,
    WEAPON_FAMILY_AXE,
    WEAPON_FAMILY_BOW,
    WEAPON_FAMILY_STAFF,
    WEAPON_FAMILY_DAGGER
} WeaponFamily;

typedef enum {
    WEAPON_HANDS_NONE = 0,
    WEAPON_HANDS_ONE,
    WEAPON_HANDS_TWO
} WeaponHands;

typedef enum {
    ITEM_RARITY_COMMON = 0,
    ITEM_RARITY_UNCOMMON,
    ITEM_RARITY_RARE
} ItemRarity;

typedef enum {
    ITEM_CLASS_WARRIOR = 1 << 0,
    ITEM_CLASS_MAGE = 1 << 1,
    ITEM_CLASS_ROGUE = 1 << 2,
    ITEM_CLASS_ALL = ITEM_CLASS_WARRIOR | ITEM_CLASS_MAGE | ITEM_CLASS_ROGUE
} ItemClassMask;

typedef enum {
    ITEM_VISUAL_NONE = 0,
    ITEM_VISUAL_WEAPON_GENERIC,
    ITEM_VISUAL_RUSTY_SWORD,
    ITEM_VISUAL_SHORT_SWORD,
    ITEM_VISUAL_LONG_SWORD,
    ITEM_VISUAL_BATTLE_AXE,
    ITEM_VISUAL_STAFF,
    ITEM_VISUAL_BOW
} ItemVisualId;

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
    WeaponFamily weapon_family;
    WeaponHands weapon_hands;
    ItemRarity rarity;
    int class_mask;
    ItemVisualId visual_id;
} Item;

typedef struct {
    int  active;
    int  x, y;
    int  underlying_tile;
    Item item;
} FloorItem;

Item item_make_health_potion(void);
Item item_make_mana_potion(void);
// Item item_make_weapon(const char *name, int attack_bonus, int value);
Item item_make_armor(const char *name, int defense_bonus, int value);

Item item_make_scroll_magic_arrow(void);
Item item_make_scroll_fireball(void);
Item item_make_scroll_heal(void);
Item item_make_scroll_return_to_town(void);

Item item_make_rusty_sword(void);
Item item_make_short_sword(void);
Item item_make_long_sword(void);
Item item_make_battle_axe(void);
Item item_make_staff(void);
Item item_make_bow(void);

Item item_make_leather_armor(void);
Item item_make_chain_mail(void);
void item_apply_legacy_metadata(Item *item);

#endif
