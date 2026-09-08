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
    ARMOR_FAMILY_NONE = 0,
    ARMOR_FAMILY_HEAVY,
    ARMOR_FAMILY_LIGHT,
    ARMOR_FAMILY_ROBE
} ArmorFamily;

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
    ITEM_VISUAL_BOW,
    ITEM_VISUAL_DAGGER,
    ITEM_VISUAL_GREATSWORD,
    ITEM_VISUAL_MAGIC_GREATSWORD,
    ITEM_VISUAL_LONGBOW,
    ITEM_VISUAL_MAGIC_LONGBOW,
    ITEM_VISUAL_MAGIC_STAFF,
    ITEM_VISUAL_MAGIC_LONG_SWORD,
    ITEM_VISUAL_MAGIC_DAGGER,
    ITEM_VISUAL_MAGIC_BATTLE_AXE,
    ITEM_VISUAL_RUNED_STAFF,
    ITEM_VISUAL_CHAIN_MAIL,
    ITEM_VISUAL_SCALE_MAIL,
    ITEM_VISUAL_PLATE_ARMOR,
    ITEM_VISUAL_MAGIC_PLATE,
    ITEM_VISUAL_LEATHER_ARMOR,
    ITEM_VISUAL_STUDDED_LEATHER,
    ITEM_VISUAL_RANGER_CLOAK,
    ITEM_VISUAL_SHADOW_ARMOR,
    ITEM_VISUAL_APPRENTICE_ROBES,
    ITEM_VISUAL_RUNED_ROBES,
    ITEM_VISUAL_ENCHANTER_ROBES,
    ITEM_VISUAL_ARCHMAGE_ROBES
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
    int critical_chance_bonus;
    int cleave_percent;
    int pierces_targets;
    int spell_power_bonus;
    int armor_penetration_percent;
    ArmorFamily armor_family;
    int max_hp_bonus;
    int max_mp_bonus;
    int evasion_chance;
    int spell_cost_reduction_percent;
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
Item item_make_magic_long_sword(void);
Item item_make_battle_axe(void);
Item item_make_magic_battle_axe(void);
Item item_make_staff(void);
Item item_make_runed_staff(void);
Item item_make_magic_staff(void);
Item item_make_bow(void);
Item item_make_longbow(void);
Item item_make_magic_longbow(void);
Item item_make_dagger(void);
Item item_make_magic_dagger(void);
Item item_make_greatsword(void);
Item item_make_magic_greatsword(void);

Item item_make_leather_armor(void);
Item item_make_chain_mail(void);
Item item_make_scale_mail(void);
Item item_make_plate_armor(void);
Item item_make_magic_plate(void);
Item item_make_studded_leather(void);
Item item_make_ranger_cloak(void);
Item item_make_shadow_armor(void);
Item item_make_apprentice_robes(void);
Item item_make_runed_robes(void);
Item item_make_enchanter_robes(void);
Item item_make_archmage_robes(void);
void item_apply_legacy_metadata(Item *item);
int item_class_allowed(const Item *item, int player_class);
const char *item_class_label(const Item *item);
const char *item_hands_label(const Item *item);
const char *item_rarity_label(const Item *item);

#endif
