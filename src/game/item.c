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

Item item_make_health_potion(void) {
    Item it = {0};
    it.active  = 1;
    it.type    = ITEM_POTION_HEALTH;
    strncpy(it.name, "Health Potion", sizeof(it.name) - 1);
    it.heal_hp = 30;
    it.value   = 10;
    return it;
}

Item item_make_mana_potion(void) {
    Item it = {0};
    it.active  = 1;
    it.type    = ITEM_POTION_MANA;
    strncpy(it.name, "Mana Potion", sizeof(it.name) - 1);
    it.heal_mp = 20;
    it.value   = 10;
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

Item item_make_staff(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Staff", sizeof(it.name) - 1);
    it.attack_bonus = 4;
    it.value = 60;
    it.spell_power_bonus = 2;
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
    set_weapon_metadata(&it, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_MAGE, ITEM_VISUAL_MAGIC_STAFF);
    return it;
}

Item item_make_bow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Bow", sizeof(it.name) - 1);
    it.attack_bonus = 3;
    it.is_ranged = 1;
    it.range = 6;
    it.value = 60;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_COMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_BOW);
    return it;
}

Item item_make_longbow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Longbow", sizeof(it.name) - 1);
    it.attack_bonus = 7;
    it.is_ranged = 1;
    it.range = 9;
    it.value = 250;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_LONGBOW);
    return it;
}

Item item_make_magic_longbow(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Longbow", sizeof(it.name) - 1);
    it.attack_bonus = 13;
    it.is_ranged = 1;
    it.range = 12;
    it.value = 1200;
    it.pierces_targets = 1;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_ROGUE, ITEM_VISUAL_MAGIC_LONGBOW);
    return it;
}

Item item_make_dagger(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Dagger", sizeof(it.name) - 1);
    it.attack_bonus = 2;
    it.value = 90;
    it.critical_chance_bonus = 25;
    set_weapon_metadata(&it, WEAPON_FAMILY_DAGGER, WEAPON_HANDS_ONE,
        ITEM_RARITY_COMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_DAGGER);
    return it;
}

Item item_make_magic_dagger(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Magic Dagger", sizeof(it.name) - 1);
    it.attack_bonus = 6;
    it.value = 500;
    it.critical_chance_bonus = 40;
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
    it.attack_bonus = 8;
    it.value = 350;
    it.critical_chance_bonus = 10;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_MAGIC_LONG_SWORD);
    return it;
}

Item item_make_goblin_king_greatsword(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Goblin King's Greatsword", sizeof(it.name) - 1);
    it.attack_bonus = 13;
    it.value = 500;
    it.cleave_percent = 55;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_TWO,
        ITEM_RARITY_RARE, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_GREATSWORD);
    return it;
}

void item_apply_legacy_metadata(Item *item) {
    Item definition = {0};
    item->critical_chance_bonus = 0;
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
    } else if (strcmp(item->name, "Magic Long Sword") == 0) {
        definition = item_make_magic_long_sword();
    } else if (strcmp(item->name, "Battle Axe") == 0) {
        definition = item_make_battle_axe();
    } else if (strcmp(item->name, "Magic Battle Axe") == 0) {
        definition = item_make_magic_battle_axe();
    } else if (strcmp(item->name, "Staff") == 0) {
        definition = item_make_staff();
    } else if (strcmp(item->name, "Runed Staff") == 0) {
        definition = item_make_runed_staff();
    } else if (strcmp(item->name, "Magic Staff") == 0) {
        definition = item_make_magic_staff();
    } else if (strcmp(item->name, "Bow") == 0) {
        definition = item_make_bow();
    } else if (strcmp(item->name, "Longbow") == 0) {
        definition = item_make_longbow();
    } else if (strcmp(item->name, "Magic Longbow") == 0) {
        definition = item_make_magic_longbow();
    } else if (strcmp(item->name, "Dagger") == 0) {
        definition = item_make_dagger();
    } else if (strcmp(item->name, "Magic Dagger") == 0) {
        definition = item_make_magic_dagger();
    } else if (strcmp(item->name, "Greatsword") == 0) {
        definition = item_make_greatsword();
    } else if (strcmp(item->name, "Magic Greatsword") == 0) {
        definition = item_make_magic_greatsword();
    } else if (strcmp(item->name, "Cryptblade") == 0) {
        definition = item_make_cryptblade();
    } else if (strcmp(item->name, "Goblin King's Greatsword") == 0) {
        definition = item_make_goblin_king_greatsword();
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
    } else if (strcmp(item->name, "Shadow Armor") == 0) {
        definition = item_make_shadow_armor();
    } else if (strcmp(item->name, "Apprentice Robes") == 0) {
        definition = item_make_apprentice_robes();
    } else if (strcmp(item->name, "Runed Robes") == 0) {
        definition = item_make_runed_robes();
    } else if (strcmp(item->name, "Enchanter Robes") == 0) {
        definition = item_make_enchanter_robes();
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
        ITEM_CLASS_ROGUE, ITEM_VISUAL_LEATHER_ARMOR);
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
