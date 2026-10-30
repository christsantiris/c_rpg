#include <stdio.h>
#include "item.h"
#include <string.h>
#include "spell.h"

static void set_weapon_metadata(Item *it, WeaponFamily family, WeaponHands hands, ItemRarity rarity, int class_mask, ItemVisualId visual_id) {
    it->weapon_family = family;
    it->weapon_hands = hands;
    it->rarity = rarity;
    it->class_mask = class_mask;
    it->visual_id = visual_id;
}

static void set_armor_metadata(Item *it, ArmorFamily family, ItemRarity rarity, int class_mask, ItemVisualId visual_id) {
    it->armor_family = family;
    it->rarity = rarity;
    it->class_mask = class_mask;
    it->visual_id = visual_id;
}

static void set_shield_metadata(Item *it, ItemRarity rarity, int class_mask, ItemVisualId visual_id) {
    it->rarity = rarity;
    it->class_mask = class_mask;
    it->visual_id = visual_id;
    it->block_reduction_percent = 50;
}

static Item make_accessory(const char *name, AccessoryFamily family, int value) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_ACCESSORY;
    snprintf(item.name, sizeof(item.name), "%s", name);
    item.accessory_family = family;
    item.class_mask = ITEM_CLASS_ALL;
    item.value = value;
    return item;
}

Item item_make_leather_cap(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_HELMET;
    snprintf(item.name, sizeof(item.name), "Leather Cap");
    item.class_mask = ITEM_CLASS_ALL;
    item.defense_bonus = 1;
    item.value = 40;
    return item;
}

typedef struct {
    const char *name;
    int value;
    int attack;
    int defense;
    int hp;
    int mp;
} AccessorySpec;

static const AccessorySpec accessory_catalog[6][3] = {
    {{"Leather Gauntlets", 60, 1, 0, 0, 0}, {"Iron Gauntlets", 160, 2, 0, 0, 0}, {"Runed Gauntlets", 350, 3, 0, 0, 5}},
    {{"Leather Bracers", 40, 0, 1, 0, 0}, {"Iron Bracers", 120, 0, 2, 0, 0}, {"Runed Bracers", 300, 0, 3, 5, 0}},
    {{"Ring of Vigor", 80, 0, 0, 10, 0}, {"Ring of Fortitude", 200, 0, 0, 20, 0}, {"Ring of Vitality", 360, 0, 0, 30, 0}},
    {{"Copper Amulet", 80, 0, 0, 0, 10}, {"Silver Amulet", 200, 0, 0, 0, 20}, {"Moonstone Amulet", 360, 0, 0, 0, 30}},
    {{"Leather Belt", 50, 0, 0, 5, 0}, {"Studded Belt", 150, 0, 0, 15, 0}, {"Guardian Belt", 300, 0, 1, 25, 0}},
    {{"Leather Boots", 50, 0, 1, 0, 0}, {"Reinforced Boots", 140, 0, 2, 0, 0}, {"Warden Boots", 300, 0, 3, 5, 0}}
};

Item item_make_accessory(AccessoryFamily family, int tier) {
    if (family < ACCESSORY_FAMILY_GAUNTLETS || family > ACCESSORY_FAMILY_BOOTS || tier < 1 || tier > 3) {
        return (Item){0};
    }
    const AccessorySpec *spec = &accessory_catalog[family - ACCESSORY_FAMILY_GAUNTLETS][tier - 1];
    Item item = make_accessory(spec->name, family, spec->value);
    item.attack_bonus = spec->attack;
    item.defense_bonus = spec->defense;
    item.max_hp_bonus = spec->hp;
    item.max_mp_bonus = spec->mp;
    item.rarity = tier - 1;
    return item;
}

Item item_make_leather_gauntlets(void) {
    return item_make_accessory(ACCESSORY_FAMILY_GAUNTLETS, 1);
}

Item item_make_leather_bracers(void) {
    return item_make_accessory(ACCESSORY_FAMILY_BRACERS, 1);
}

Item item_make_ring_of_vigor(void) {
    return item_make_accessory(ACCESSORY_FAMILY_RING, 1);
}

Item item_make_copper_amulet(void) {
    return item_make_accessory(ACCESSORY_FAMILY_AMULET, 1);
}

int item_accessory_limit(const Item *item) {
    if (item->type != ITEM_ACCESSORY) {
        return 0;
    }
    switch (item->accessory_family) {
        case ACCESSORY_FAMILY_RING:
            return 2;
        case ACCESSORY_FAMILY_GAUNTLETS:
        case ACCESSORY_FAMILY_BRACERS:
        case ACCESSORY_FAMILY_AMULET:
        case ACCESSORY_FAMILY_BELT:
        case ACCESSORY_FAMILY_BOOTS:
            return 1;
        default:
            return 0;
    }
}

int item_is_jewelry(const Item *item) {
    return item->type == ITEM_ACCESSORY && (item->accessory_family == ACCESSORY_FAMILY_RING || item->accessory_family == ACCESSORY_FAMILY_AMULET);
}

Item item_make_health_potion(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_POTION_HEALTH;
    strncpy(it.name, "Health Potion", sizeof(it.name) - 1);
    it.value = 10;
    return it;
}

Item item_make_mana_potion(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_POTION_MANA;
    strncpy(it.name, "Mana Potion", sizeof(it.name) - 1);
    it.value = 10;
    return it;
}

Item item_make_arrows(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARROWS;
    strncpy(it.name, "20 Arrows", sizeof(it.name) - 1);
    it.value = 5;
    return it;
}

Item item_make_strength_potion(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_POTION_STRENGTH;
    strncpy(it.name, "Potion of Strength", sizeof(it.name) - 1);
    it.value = 600;
    it.class_mask = ITEM_CLASS_ALL;
    return it;
}

Item item_make_intelligence_potion(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_POTION_INTELLIGENCE;
    strncpy(it.name, "Potion of Intelligence", sizeof(it.name) - 1);
    it.value = 600;
    it.class_mask = ITEM_CLASS_ALL;
    return it;
}

// Item item_make_weapon(const char *name, int attack_bonus, int value) {
//     Item it = {0};
//     it.active       = 1;
//     it.type         = ITEM_WEAPON;
//     strncpy(it.name, name, sizeof(it.name) - 1);
//     it.attack_bonus = attack_bonus;
//     it.value        = value;
//     return it;
// }

int item_can_sharpen(const Item *item) {
    return item->type == ITEM_WEAPON && !item->sharpened &&
        (item->weapon_family == WEAPON_FAMILY_SWORD || item->weapon_family == WEAPON_FAMILY_AXE ||
        item->weapon_family == WEAPON_FAMILY_DAGGER);
}

Item item_make_rusty_sword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Rusty Sword", sizeof(it.name) - 1);
    it.attack_bonus = 1;
    it.value = 10;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_COMMON, ITEM_CLASS_WARRIOR, ITEM_VISUAL_RUSTY_SWORD);
    return it;
}

Item item_make_short_sword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Short Sword", sizeof(it.name) - 1);
    it.attack_bonus = 3;
    it.value = 40;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_COMMON, ITEM_CLASS_WARRIOR | ITEM_CLASS_ROGUE,
        ITEM_VISUAL_SHORT_SWORD);
    return it;
}

Item item_make_long_sword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Long Sword", sizeof(it.name) - 1);
    it.attack_bonus = 6;
    it.value = 180;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_LONG_SWORD);
    return it;
}

Item item_make_demonic_sword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Demonic Sword", sizeof(it.name) - 1);
    it.attack_bonus = 6;
    it.value = 180;
    it.is_ranged = 1;
    it.range = 2;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_RARE, ITEM_CLASS_ALL, ITEM_VISUAL_DEMONIC_SWORD);
    return it;
}

Item item_make_poison_sword(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_WEAPON;
    strncpy(item.name, "Poison Sword", sizeof(item.name) - 1);
    item.attack_bonus = 16;
    item.value = 1000;
    set_weapon_metadata(&item, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR | ITEM_CLASS_ROGUE, ITEM_VISUAL_POISON_SWORD);
    return item;
}

Item item_make_magic_long_sword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Long Sword", sizeof(it.name) - 1);
    it.attack_bonus = 10;
    it.value = 550;
    it.critical_chance_bonus = 20;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_MAGIC_LONG_SWORD);
    return it;
}

Item item_make_battle_axe(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Battle Axe", sizeof(it.name) - 1);
    it.attack_bonus = 10;
    it.value = 275;
    it.armor_penetration_percent = 25;
    set_weapon_metadata(&it, WEAPON_FAMILY_AXE, WEAPON_HANDS_TWO,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_BATTLE_AXE);
    return it;
}

Item item_make_magic_battle_axe(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Battle Axe", sizeof(it.name) - 1);
    it.attack_bonus = 13;
    it.value = 700;
    it.armor_penetration_percent = 50;
    set_weapon_metadata(&it, WEAPON_FAMILY_AXE, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_MAGIC_BATTLE_AXE);
    return it;
}

Item item_make_embercleaver(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_WEAPON;
    strncpy(item.name, "Embercleaver", sizeof(item.name) - 1);
    item.attack_bonus = 11;
    item.armor_penetration_percent = 25;
    item.cleave_percent = 25;
    item.value = 650;
    set_weapon_metadata(&item, WEAPON_FAMILY_AXE, WEAPON_HANDS_TWO, ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR, ITEM_VISUAL_EMBERCLEAVER);
    return item;
}

Item item_make_worn_staff(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_WEAPON;
    strncpy(item.name, "Worn Staff", sizeof(item.name) - 1);
    item.attack_bonus = 1;
    item.value = 10;
    set_weapon_metadata(&item, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO, ITEM_RARITY_COMMON, ITEM_CLASS_MAGE, ITEM_VISUAL_STAFF);
    return item;
}

Item item_make_staff(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Staff", sizeof(it.name) - 1);
    it.attack_bonus = 4;
    it.value = 60;
    it.spell_power_bonus = 2;
    it.max_mp_bonus = 10;
    set_weapon_metadata(&it, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO,
        ITEM_RARITY_COMMON, ITEM_CLASS_MAGE, ITEM_VISUAL_STAFF);
    return it;
}

Item item_make_runed_staff(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Runed Staff", sizeof(it.name) - 1);
    it.attack_bonus = 6;
    it.value = 300;
    it.spell_power_bonus = 4;
    it.max_mp_bonus = 20;
    it.spell_cost_reduction_percent = 5;
    set_weapon_metadata(&it, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_MAGE, ITEM_VISUAL_RUNED_STAFF);
    return it;
}

Item item_make_magic_staff(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Staff", sizeof(it.name) - 1);
    it.attack_bonus = 9;
    it.value = 1000;
    it.spell_power_bonus = 8;
    it.max_mp_bonus = 35;
    it.spell_cost_reduction_percent = 10;
    set_weapon_metadata(&it, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_MAGE, ITEM_VISUAL_MAGIC_STAFF);
    return it;
}

Item item_make_sandstorm_staff(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Sandstorm Staff", sizeof(it.name) - 1);
    it.attack_bonus = 7;
    it.value = 700;
    it.spell_power_bonus = 6;
    it.max_mp_bonus = 25;
    it.spell_cost_reduction_percent = 7;
    set_weapon_metadata(&it, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_MAGE, ITEM_VISUAL_SANDSTORM_STAFF);
    return it;
}

Item item_make_bow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Hunting Bow", sizeof(it.name) - 1);
    it.attack_bonus = 3;
    it.is_ranged = 1;
    it.range = 3;
    it.value = 60;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_COMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_BOW);
    return it;
}

Item item_make_shortbow(void) {
    Item it = item_make_bow();
    strncpy(it.name, "Shortbow", sizeof(it.name) - 1);
    it.attack_bonus = 5;
    it.range = 5;
    it.value = 140;
    it.visual_id = ITEM_VISUAL_SHORTBOW;
    return it;
}

Item item_make_longbow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Longbow", sizeof(it.name) - 1);
    it.attack_bonus = 7;
    it.is_ranged = 1;
    it.range = MAX_BOW_RANGE;
    it.value = 250;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_LONGBOW);
    return it;
}

Item item_make_crossbow(void) {
    Item it = item_make_longbow();
    strncpy(it.name, "Crossbow", sizeof(it.name) - 1);
    it.attack_bonus = 9;
    it.range = 5;
    it.value = 450;
    it.visual_id = ITEM_VISUAL_CROSSBOW;
    return it;
}

// The Frost Kraken's reward sits between the Crossbow and Magic Longbow.
Item item_make_krakenbone_bow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Krakenbone Bow", sizeof(it.name) - 1);
    it.attack_bonus = 10;
    it.is_ranged = 1;
    it.range = MAX_BOW_RANGE;
    it.value = 700;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_ROGUE, ITEM_VISUAL_LONGBOW);
    return it;
}

Item item_make_magic_longbow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Longbow", sizeof(it.name) - 1);
    it.attack_bonus = 13;
    it.is_ranged = 1;
    it.range = MAX_BOW_RANGE;
    it.value = 1200;
    it.pierces_targets = 1;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_ROGUE, ITEM_VISUAL_MAGIC_LONGBOW);
    return it;
}

Item item_make_rusty_dagger(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Rusty Dagger", sizeof(it.name) - 1);
    it.attack_bonus = 1;
    it.value = 10;
    it.critical_chance_bonus = 10;
    it.gold_drop_bonus_percent = 10;
    set_weapon_metadata(&it, WEAPON_FAMILY_DAGGER, WEAPON_HANDS_ONE,
        ITEM_RARITY_COMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_RUSTY_DAGGER);
    return it;
}

Item item_make_dagger(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Dagger", sizeof(it.name) - 1);
    it.attack_bonus = 3;
    it.value = 90;
    it.critical_chance_bonus = 25;
    it.gold_drop_bonus_percent = 25;
    set_weapon_metadata(&it, WEAPON_FAMILY_DAGGER, WEAPON_HANDS_ONE,
        ITEM_RARITY_COMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_DAGGER);
    return it;
}

Item item_make_smuggled_dagger(void) {
    // A blade small enough to hide in a cell, so Tomas can arm any class.
    Item it = item_make_dagger();
    strncpy(it.name, "Smuggled Dagger", sizeof(it.name) - 1);
    it.class_mask = ITEM_CLASS_ALL;
    return it;
}

Item item_make_magic_dagger(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Dagger", sizeof(it.name) - 1);
    it.attack_bonus = 7;
    it.value = 500;
    it.critical_chance_bonus = 40;
    it.gold_drop_bonus_percent = 50;
    set_weapon_metadata(&it, WEAPON_FAMILY_DAGGER, WEAPON_HANDS_ONE,
        ITEM_RARITY_RARE, ITEM_CLASS_ROGUE, ITEM_VISUAL_MAGIC_DAGGER);
    return it;
}

Item item_make_greatsword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Greatsword", sizeof(it.name) - 1);
    it.attack_bonus = 12;
    it.value = 400;
    it.cleave_percent = 50;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_TWO,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_GREATSWORD);
    return it;
}

Item item_make_magic_greatsword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Greatsword", sizeof(it.name) - 1);
    it.attack_bonus = 18;
    it.value = 1100;
    it.cleave_percent = 75;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_MAGIC_GREATSWORD);
    return it;
}

Item item_make_cryptblade(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Cryptblade", sizeof(it.name) - 1);
    it.attack_bonus = 2;
    it.value = 350;
    it.critical_chance_bonus = 10;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_MAGIC_LONG_SWORD);
    return it;
}

void item_apply_legacy_metadata(Item *item) {
    Item definition = {0};
    item->critical_chance_bonus = 0;
    item->gold_drop_bonus_percent = 0;
    item->cleave_percent = 0;
    item->pierces_targets = 0;
    item->spell_power_bonus = 0;
    item->armor_penetration_percent = 0;
    if (strcmp(item->name, "Rusty Sword") == 0) {
        definition = item_make_rusty_sword();
    } else if (strcmp(item->name, "Short Sword") == 0) {
        definition = item_make_short_sword();
    } else if (strcmp(item->name, "Long Sword") == 0) {
        definition = item_make_long_sword();
    } else if (strcmp(item->name, "Demonic Sword") == 0) {
        definition = item_make_demonic_sword();
    } else if (strcmp(item->name, "Poison Sword") == 0) {
        definition = item_make_poison_sword();
    } else if (strcmp(item->name, "Magic Long Sword") == 0) {
        definition = item_make_magic_long_sword();
    } else if (strcmp(item->name, "Battle Axe") == 0) {
        definition = item_make_battle_axe();
    } else if (strcmp(item->name, "Embercleaver") == 0) {
        definition = item_make_embercleaver();
    } else if (strcmp(item->name, "Magic Battle Axe") == 0) {
        definition = item_make_magic_battle_axe();
    } else if (strcmp(item->name, "Worn Staff") == 0) {
        definition = item_make_worn_staff();
    } else if (strcmp(item->name, "Staff") == 0) {
        definition = item_make_staff();
    } else if (strcmp(item->name, "Runed Staff") == 0) {
        definition = item_make_runed_staff();
    } else if (strcmp(item->name, "Magic Staff") == 0) {
        definition = item_make_magic_staff();
    } else if (strcmp(item->name, "Bow") == 0 || strcmp(item->name, "Hunting Bow") == 0) {
        definition = item_make_bow();
    } else if (strcmp(item->name, "Shortbow") == 0) {
        definition = item_make_shortbow();
    } else if (strcmp(item->name, "Longbow") == 0) {
        definition = item_make_longbow();
    } else if (strcmp(item->name, "Crossbow") == 0) {
        definition = item_make_crossbow();
    } else if (strcmp(item->name, "Krakenbone Bow") == 0) {
        definition = item_make_krakenbone_bow();
    } else if (strcmp(item->name, "Magic Longbow") == 0) {
        definition = item_make_magic_longbow();
    } else if (strcmp(item->name, "Rusty Dagger") == 0) {
        definition = item_make_rusty_dagger();
    } else if (strcmp(item->name, "Dagger") == 0) {
        definition = item_make_dagger();
    } else if (strcmp(item->name, "Magic Dagger") == 0) {
        definition = item_make_magic_dagger();
    } else if (strcmp(item->name, "Greatsword") == 0) {
        definition = item_make_greatsword();
    } else if (strcmp(item->name, "Magic Greatsword") == 0) {
        definition = item_make_magic_greatsword();
    } else if (strcmp(item->name, "Magic Stone Warhammer") == 0) {
        definition = item_make_stone_warhammer();
    } else if (strcmp(item->name, "Cryptblade") == 0) {
        definition = item_make_cryptblade();
    } else {
        set_weapon_metadata(item, WEAPON_FAMILY_NONE, WEAPON_HANDS_ONE,
            ITEM_RARITY_COMMON, ITEM_CLASS_ALL, ITEM_VISUAL_WEAPON_GENERIC);
        return;
    }
    item->weapon_family = definition.weapon_family;
    item->attack_bonus = definition.attack_bonus;
    item->value = definition.value;
    item->is_ranged = definition.is_ranged;
    item->range = definition.range;
    item->weapon_hands = definition.weapon_hands;
    item->rarity = definition.rarity;
    item->class_mask = definition.class_mask;
    item->visual_id = definition.visual_id;
    item->critical_chance_bonus = definition.critical_chance_bonus;
    item->gold_drop_bonus_percent = definition.gold_drop_bonus_percent;
    item->cleave_percent = definition.cleave_percent;
    item->pierces_targets = definition.pierces_targets;
    item->spell_power_bonus = definition.spell_power_bonus;
    item->armor_penetration_percent =
        definition.armor_penetration_percent;
}

void item_apply_legacy_armor_metadata(Item *item) {
    Item definition = {0};
    if (strcmp(item->name, "Leather Armor") == 0) {
        definition = item_make_leather_armor();
    } else if (strcmp(item->name, "Chain Mail") == 0) {
        definition = item_make_chain_mail();
    } else if (strcmp(item->name, "Scale Mail") == 0) {
        definition = item_make_scale_mail();
    } else if (strcmp(item->name, "Plate Armor") == 0) {
        definition = item_make_plate_armor();
    } else if (strcmp(item->name, "Magic Plate") == 0) {
        definition = item_make_magic_plate();
    } else if (strcmp(item->name, "Studded Leather") == 0) {
        definition = item_make_studded_leather();
    } else if (strcmp(item->name, "Ranger Cloak") == 0) {
        definition = item_make_ranger_cloak();
    } else if (strcmp(item->name, "Briarweave Cloak") == 0) {
        definition = item_make_briarweave_cloak();
    } else if (strcmp(item->name, "Shadow Armor") == 0) {
        definition = item_make_shadow_armor();
    } else if (strcmp(item->name, "Apprentice Robes") == 0) {
        definition = item_make_apprentice_robes();
    } else if (strcmp(item->name, "Runed Robes") == 0) {
        definition = item_make_runed_robes();
    } else if (strcmp(item->name, "Enchanter Robes") == 0) {
        definition = item_make_enchanter_robes();
    } else if (strcmp(item->name, "Prismweave Robes") == 0) {
        definition = item_make_prismweave_robes();
    } else if (strcmp(item->name, "Crown Chapel Vestments") == 0) {
        definition = item_make_crown_chapel_vestments();
    } else if (strcmp(item->name, "Sunwarden Mantle") == 0) {
        definition = item_make_sunwarden_mantle();
    } else if (strcmp(item->name, "Archmage Robes") == 0) {
        definition = item_make_archmage_robes();
    } else if (strcmp(item->name, "Necromancer's Cloak") == 0) {
        definition = item_make_necromancer_cloak();
    } else if (strcmp(item->name, "Tidecaller Robes") == 0) {
        definition = item_make_tidecaller_robes();
    } else {
        return;
    }
    *item = definition;
}

int item_class_allowed(const Item *item, int player_class) {
    if (item->class_mask == 0 || item->class_mask == ITEM_CLASS_ALL) {
        return 1;
    }
    if (player_class < 0 || player_class > 2) {
        return 0;
    }
    return (item->class_mask & (1 << player_class)) != 0;
}

const char *item_class_label(const Item *item) {
    switch (item->class_mask) {
        case ITEM_CLASS_WARRIOR:
            return "WARRIOR";
        case ITEM_CLASS_MAGE:
            return "MAGE";
        case ITEM_CLASS_ROGUE:
            return "ROGUE";
        case ITEM_CLASS_WARRIOR | ITEM_CLASS_MAGE:
            return "WARRIOR/MAGE";
        case ITEM_CLASS_WARRIOR | ITEM_CLASS_ROGUE:
            return "WARRIOR/ROGUE";
        case ITEM_CLASS_MAGE | ITEM_CLASS_ROGUE:
            return "MAGE/ROGUE";
        default:
            return "ALL CLASSES";
    }
}

const char *item_hands_label(const Item *item) {
    if (item->weapon_hands == WEAPON_HANDS_TWO) {
        return "TWO-HANDED";
    }
    if (item->weapon_hands == WEAPON_HANDS_ONE) {
        return "ONE-HANDED";
    }
    return "";
}

const char *item_rarity_label(const Item *item) {
    if (item->rarity == ITEM_RARITY_RARE) {
        return "RARE";
    }
    if (item->rarity == ITEM_RARITY_UNCOMMON) {
        return "UNCOMMON";
    }
    return "COMMON";
}

// Item item_make_armor(const char *name, int defense_bonus, int value) {
//     Item it = {0};
//     it.active        = 1;
//     it.type          = ITEM_ARMOR;
//     strncpy(it.name, name, sizeof(it.name) - 1);
//     it.defense_bonus = defense_bonus;
//     it.value         = value;
//     return it;
// }

Item item_make_leather_armor(void) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_ARMOR;
    strncpy(it.name, "Leather Armor", sizeof(it.name) - 1);
    it.defense_bonus = 2;
    it.value         = 60;
    set_armor_metadata(&it, ARMOR_FAMILY_LIGHT, ITEM_RARITY_COMMON,
        ITEM_CLASS_ALL, ITEM_VISUAL_LEATHER_ARMOR);
    return it;
}

Item item_make_chain_mail(void) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_ARMOR;
    strncpy(it.name, "Chain Mail", sizeof(it.name) - 1);
    it.defense_bonus = 3;
    it.value         = 80;
    set_armor_metadata(&it, ARMOR_FAMILY_HEAVY, ITEM_RARITY_COMMON,
        ITEM_CLASS_WARRIOR, ITEM_VISUAL_CHAIN_MAIL);
    return it;
}

Item item_make_scale_mail(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Scale Mail", sizeof(it.name) - 1);
    it.defense_bonus = 5;
    it.max_hp_bonus = 10;
    it.value = 275;
    set_armor_metadata(&it, ARMOR_FAMILY_HEAVY, ITEM_RARITY_UNCOMMON,
        ITEM_CLASS_WARRIOR, ITEM_VISUAL_SCALE_MAIL);
    return it;
}

Item item_make_plate_armor(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Plate Armor", sizeof(it.name) - 1);
    it.defense_bonus = 8;
    it.max_hp_bonus = 25;
    it.value = 650;
    set_armor_metadata(&it, ARMOR_FAMILY_HEAVY, ITEM_RARITY_RARE,
        ITEM_CLASS_WARRIOR, ITEM_VISUAL_PLATE_ARMOR);
    return it;
}

Item item_make_magic_plate(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Magic Plate", sizeof(it.name) - 1);
    it.defense_bonus = 11;
    it.max_hp_bonus = 50;
    it.value = 1200;
    set_armor_metadata(&it, ARMOR_FAMILY_HEAVY, ITEM_RARITY_RARE,
        ITEM_CLASS_WARRIOR, ITEM_VISUAL_MAGIC_PLATE);
    return it;
}

Item item_make_studded_leather(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Studded Leather", sizeof(it.name) - 1);
    it.defense_bonus = 4;
    it.evasion_chance = 5;
    it.value = 250;
    set_armor_metadata(&it, ARMOR_FAMILY_LIGHT, ITEM_RARITY_UNCOMMON,
        ITEM_CLASS_ROGUE, ITEM_VISUAL_STUDDED_LEATHER);
    return it;
}

Item item_make_ranger_cloak(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Ranger Cloak", sizeof(it.name) - 1);
    it.defense_bonus = 5;
    it.evasion_chance = 10;
    it.value = 600;
    set_armor_metadata(&it, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE,
        ITEM_CLASS_ROGUE, ITEM_VISUAL_RANGER_CLOAK);
    return it;
}

Item item_make_shadow_armor(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Shadow Armor", sizeof(it.name) - 1);
    it.defense_bonus = 7;
    it.evasion_chance = 15;
    it.value = 1100;
    set_armor_metadata(&it, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE,
        ITEM_CLASS_ROGUE, ITEM_VISUAL_SHADOW_ARMOR);
    return it;
}

Item item_make_apprentice_robes(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Apprentice Robes", sizeof(it.name) - 1);
    it.defense_bonus = 1;
    it.max_mp_bonus = 10;
    it.value = 60;
    set_armor_metadata(&it, ARMOR_FAMILY_ROBE, ITEM_RARITY_COMMON,
        ITEM_CLASS_MAGE, ITEM_VISUAL_APPRENTICE_ROBES);
    return it;
}

Item item_make_runed_robes(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Runed Robes", sizeof(it.name) - 1);
    it.defense_bonus = 2;
    it.max_mp_bonus = 25;
    it.value = 275;
    set_armor_metadata(&it, ARMOR_FAMILY_ROBE, ITEM_RARITY_UNCOMMON,
        ITEM_CLASS_MAGE, ITEM_VISUAL_RUNED_ROBES);
    return it;
}

Item item_make_enchanter_robes(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Enchanter Robes", sizeof(it.name) - 1);
    it.defense_bonus = 3;
    it.max_mp_bonus = 40;
    it.spell_cost_reduction_percent = 10;
    it.value = 675;
    set_armor_metadata(&it, ARMOR_FAMILY_ROBE, ITEM_RARITY_RARE,
        ITEM_CLASS_MAGE, ITEM_VISUAL_ENCHANTER_ROBES);
    return it;
}

Item item_make_stone_warhammer(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_WEAPON;
    snprintf(item.name, sizeof(item.name), "Magic Stone Warhammer");
    item.attack_bonus = 8;
    item.max_hp_bonus = 30;
    item.value = 1000;
    set_weapon_metadata(&item, WEAPON_FAMILY_HAMMER, WEAPON_HANDS_TWO, ITEM_RARITY_RARE, ITEM_CLASS_ALL, ITEM_VISUAL_STONE_WARHAMMER);
    return item;
}

Item item_make_prismweave_robes(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_ARMOR;
    strncpy(item.name, "Prismweave Robes", sizeof(item.name) - 1);
    item.defense_bonus = 3;
    item.max_mp_bonus = 35;
    item.spell_cost_reduction_percent = 12;
    item.value = 700;
    set_armor_metadata(&item, ARMOR_FAMILY_ROBE, ITEM_RARITY_RARE, ITEM_CLASS_MAGE, ITEM_VISUAL_PRISMWEAVE_ROBES);
    return item;
}

Item item_make_sunwarden_mantle(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_ARMOR;
    strncpy(item.name, "Sunwarden Mantle", sizeof(item.name) - 1);
    item.defense_bonus = 5;
    item.max_hp_bonus = 8;
    item.max_mp_bonus = 7;
    item.value = 950;
    set_armor_metadata(&item, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE, ITEM_CLASS_ALL, ITEM_VISUAL_SUNWARDEN_MANTLE);
    return item;
}

Item item_make_archmage_robes(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Archmage Robes", sizeof(it.name) - 1);
    it.defense_bonus = 4;
    it.max_mp_bonus = 60;
    it.spell_cost_reduction_percent = 20;
    it.value = 1250;
    set_armor_metadata(&it, ARMOR_FAMILY_ROBE, ITEM_RARITY_RARE,
        ITEM_CLASS_MAGE, ITEM_VISUAL_ARCHMAGE_ROBES);
    return it;
}

Item item_make_crown_chapel_vestments(void) {
    Item item = item_make_archmage_robes();
    strncpy(item.name, "Crown Chapel Vestments", sizeof(item.name) - 1);
    item.max_mp_bonus = 45;
    item.spell_cost_reduction_percent = 15;
    item.value = 1000;
    item.visual_id = ITEM_VISUAL_CROWN_CHAPEL_VESTMENTS;
    return item;
}

Item item_make_gravekeeper_mantle(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_ARMOR;
    strncpy(item.name, "Gravekeeper's Mantle", sizeof(item.name) - 1);
    item.defense_bonus = 6;
    item.value = 600;
    set_armor_metadata(&item, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE, ITEM_CLASS_ALL, ITEM_VISUAL_RANGER_CLOAK);
    item.max_hp_bonus = 20;
    return item;
}

Item item_make_briarweave_cloak(void) {
    Item item = {0};
    item.active = 1;
    item.type = ITEM_ARMOR;
    strncpy(item.name, "Briarweave Cloak", sizeof(item.name) - 1);
    item.defense_bonus = 4;
    item.evasion_chance = 10;
    item.value = 550;
    set_armor_metadata(&item, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE, ITEM_CLASS_ALL, ITEM_VISUAL_BRIARWEAVE_CLOAK);
    return item;
}

Item item_make_necromancer_cloak(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Necromancer's Cloak", sizeof(it.name) - 1);
    it.defense_bonus = 5;
    it.evasion_chance = 8;
    it.value = 450;
    set_armor_metadata(&it, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE,
        ITEM_CLASS_ROGUE, ITEM_VISUAL_RANGER_CLOAK);
    return it;
}

Item item_make_dragon_scale_mantle(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Dragon Scale Mantle", sizeof(it.name) - 1);
    it.defense_bonus = 4;
    it.max_hp_bonus = 20;
    it.value = 1400;
    set_armor_metadata(&it, ARMOR_FAMILY_LIGHT, ITEM_RARITY_RARE,
        ITEM_CLASS_ALL, ITEM_VISUAL_SCALE_MAIL);
    return it;
}

Item item_make_tidecaller_robes(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_ARMOR;
    strncpy(it.name, "Tidecaller Robes", sizeof(it.name) - 1);
    it.defense_bonus = 3;
    it.max_mp_bonus = 30;
    it.spell_cost_reduction_percent = 8;
    it.value = 500;
    set_armor_metadata(&it, ARMOR_FAMILY_ROBE, ITEM_RARITY_RARE,
        ITEM_CLASS_MAGE, ITEM_VISUAL_ENCHANTER_ROBES);
    return it;
}

Item item_make_buckler(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SHIELD;
    strncpy(it.name, "Buckler", sizeof(it.name) - 1);
    it.defense_bonus = 2;
    it.block_chance = 10;
    it.value = 75;
    set_shield_metadata(&it, ITEM_RARITY_COMMON,
        ITEM_CLASS_WARRIOR | ITEM_CLASS_ROGUE, ITEM_VISUAL_BUCKLER);
    return it;
}

Item item_make_kite_shield(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SHIELD;
    strncpy(it.name, "Kite Shield", sizeof(it.name) - 1);
    it.defense_bonus = 4;
    it.block_chance = 15;
    it.value = 300;
    set_shield_metadata(&it, ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_KITE_SHIELD);
    return it;
}

Item item_make_tower_shield(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SHIELD;
    strncpy(it.name, "Tower Shield", sizeof(it.name) - 1);
    it.defense_bonus = 7;
    it.block_chance = 20;
    it.value = 700;
    set_shield_metadata(&it, ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_TOWER_SHIELD);
    return it;
}

Item item_make_magic_shield(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SHIELD;
    strncpy(it.name, "Magic Shield", sizeof(it.name) - 1);
    it.defense_bonus = 9;
    it.block_chance = 25;
    it.value = 1250;
    set_shield_metadata(&it, ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_MAGIC_SHIELD);
    return it;
}

Item item_make_labyrinth_aegis(void) {
    Item it = item_make_magic_shield();
    strncpy(it.name, "Labyrinth Aegis", sizeof(it.name) - 1);
    it.visual_id = ITEM_VISUAL_LABYRINTH_AEGIS;
    return it;
}

Item item_make_castellan_bulwark(void) {
    Item it = item_make_magic_shield();
    strncpy(it.name, "Castellan's Bulwark", sizeof(it.name) - 1);
    it.defense_bonus = 8;
    it.block_chance = 30;
    it.visual_id = ITEM_VISUAL_CASTELLAN_BULWARK;
    return it;
}

Item item_make_goblin_king_shield(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SHIELD;
    strncpy(it.name, "Goblin King's Shield", sizeof(it.name) - 1);
    it.defense_bonus = 3;
    it.block_chance = 12;
    it.value = 200;
    set_shield_metadata(&it, ITEM_RARITY_UNCOMMON,
        ITEM_CLASS_WARRIOR | ITEM_CLASS_ROGUE, ITEM_VISUAL_KITE_SHIELD);
    return it;
}

Item item_make_stoneguard_shield(void) {
    Item it = item_make_tower_shield();
    snprintf(it.name, sizeof(it.name), "Stoneguard Shield");
    it.defense_bonus = 8;
    it.max_hp_bonus = 30;
    it.value = 1250;
    return it;
}

void item_apply_legacy_shield_metadata(Item *item) {
    Item definition = {0};
    if (strcmp(item->name, "Buckler") == 0) {
        definition = item_make_buckler();
    } else if (strcmp(item->name, "Kite Shield") == 0) {
        definition = item_make_kite_shield();
    } else if (strcmp(item->name, "Tower Shield") == 0) {
        definition = item_make_tower_shield();
    } else if (strcmp(item->name, "Magic Shield") == 0) {
        definition = item_make_magic_shield();
    } else if (strcmp(item->name, "Labyrinth Aegis") == 0) {
        definition = item_make_labyrinth_aegis();
    } else if (strcmp(item->name, "Castellan's Bulwark") == 0) {
        definition = item_make_castellan_bulwark();
    } else if (strcmp(item->name, "Goblin King's Shield") == 0) {
        definition = item_make_goblin_king_shield();
    } else {
        return;
    }
    *item = definition;
}

Item item_make_scroll_magic_arrow(void) {
    Item it = {0};
    it.active   = 1;
    it.type     = ITEM_SCROLL;
    strncpy(it.name, "Scroll: Magic Arrow", sizeof(it.name) - 1);
    it.spell_id = SPELL_MAGIC_ARROW;
    it.value    = 25;
    return it;
}

Item item_make_scroll_fireball(void) {
    Item it = {0};
    it.active   = 1;
    it.type     = ITEM_SCROLL;
    strncpy(it.name, "Scroll: Fireball", sizeof(it.name) - 1);
    it.spell_id = SPELL_FIREBALL;
    it.value    = 40;
    it.class_mask = ITEM_CLASS_MAGE;
    return it;
}

Item item_make_scroll_heal(void) {
    Item it = {0};
    it.active   = 1;
    it.type     = ITEM_SCROLL;
    strncpy(it.name, "Scroll: Heal", sizeof(it.name) - 1);
    it.spell_id = SPELL_HEAL;
    it.value    = 30;
    return it;
}

Item item_make_scroll_return_to_town(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SCROLL;
    strncpy(it.name, "Scroll: Return to Town", sizeof(it.name) - 1);
    it.spell_id = SPELL_RETURN_TO_TOWN;
    it.value = 0;
    return it;
}

Item item_make_treasure_map(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_TREASURE_MAP;
    strncpy(it.name, "Island Treasure Map", sizeof(it.name) - 1);
    it.class_mask = ITEM_CLASS_ALL;
    return it;
}

Item item_make_magic_arrow_tome(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SPELL_TOME;
    strncpy(it.name, "Tome: Magic Arrow II", sizeof(it.name) - 1);
    it.spell_id = SPELL_MAGIC_ARROW;
    it.value = 180;
    it.class_mask = ITEM_CLASS_MAGE;
    it.rarity = ITEM_RARITY_UNCOMMON;
    return it;
}

Item item_make_fireball_tome(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SPELL_TOME;
    strncpy(it.name, "Tome: Fireball II", sizeof(it.name) - 1);
    it.spell_id = SPELL_FIREBALL;
    it.value = 350;
    it.class_mask = ITEM_CLASS_MAGE;
    it.rarity = ITEM_RARITY_RARE;
    return it;
}

Item item_make_heal_tome(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SPELL_TOME;
    strncpy(it.name, "Tome: Heal II", sizeof(it.name) - 1);
    it.spell_id = SPELL_HEAL;
    it.value = 500;
    it.class_mask = ITEM_CLASS_MAGE;
    it.rarity = ITEM_RARITY_RARE;
    return it;
}

Item item_make_scroll_frost_bolt(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SCROLL;
    strncpy(it.name, "Scroll: Frost Bolt", sizeof(it.name) - 1);
    it.spell_id = SPELL_FROST_BOLT;
    it.value = 140;
    it.class_mask = ITEM_CLASS_MAGE;
    it.rarity = ITEM_RARITY_UNCOMMON;
    return it;
}

Item item_make_scroll_teleport(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_SCROLL;
    strncpy(it.name, "Scroll: Teleport", sizeof(it.name) - 1);
    it.spell_id = SPELL_TELEPORT;
    it.value = 260;
    it.class_mask = ITEM_CLASS_MAGE;
    it.rarity = ITEM_RARITY_RARE;
    return it;
}
