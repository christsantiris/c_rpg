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
    it.value = 5;
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
    it.value = 15;
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
    it.value = 30;
    set_weapon_metadata(&it, WEAPON_FAMILY_SWORD, WEAPON_HANDS_ONE,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_LONG_SWORD);
    return it;
}

Item item_make_battle_axe(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Battle Axe", sizeof(it.name) - 1);
    it.attack_bonus = 10;
    it.value = 50;
    set_weapon_metadata(&it, WEAPON_FAMILY_AXE, WEAPON_HANDS_TWO,
        ITEM_RARITY_UNCOMMON, ITEM_CLASS_WARRIOR,
        ITEM_VISUAL_BATTLE_AXE);
    return it;
}

Item item_make_staff(void) {
    Item it = {0};
    it.active = 1;
    it.type = ITEM_WEAPON;
    strncpy(it.name, "Staff", sizeof(it.name) - 1);
    it.attack_bonus = 4;
    it.value = 25;
    set_weapon_metadata(&it, WEAPON_FAMILY_STAFF, WEAPON_HANDS_TWO,
        ITEM_RARITY_COMMON, ITEM_CLASS_MAGE, ITEM_VISUAL_STAFF);
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
    it.value = 35;
    set_weapon_metadata(&it, WEAPON_FAMILY_BOW, WEAPON_HANDS_TWO,
        ITEM_RARITY_COMMON, ITEM_CLASS_ROGUE, ITEM_VISUAL_BOW);
    return it;
}

void item_apply_legacy_metadata(Item *item) {
    Item definition = {0};
    if (strcmp(item->name, "Rusty Sword") == 0) {
        definition = item_make_rusty_sword();
    } else if (strcmp(item->name, "Short Sword") == 0) {
        definition = item_make_short_sword();
    } else if (strcmp(item->name, "Long Sword") == 0) {
        definition = item_make_long_sword();
    } else if (strcmp(item->name, "Battle Axe") == 0) {
        definition = item_make_battle_axe();
    } else if (strcmp(item->name, "Staff") == 0) {
        definition = item_make_staff();
    } else if (strcmp(item->name, "Bow") == 0) {
        definition = item_make_bow();
    } else {
        set_weapon_metadata(item, WEAPON_FAMILY_NONE, WEAPON_HANDS_ONE,
            ITEM_RARITY_COMMON, ITEM_CLASS_ALL, ITEM_VISUAL_WEAPON_GENERIC);
        return;
    }
    item->weapon_family = definition.weapon_family;
    item->weapon_hands = definition.weapon_hands;
    item->rarity = definition.rarity;
    item->class_mask = definition.class_mask;
    item->visual_id = definition.visual_id;
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
    it.value         = 15;
    return it;
}

Item item_make_chain_mail(void) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_ARMOR;
    strncpy(it.name, "Chain Mail", sizeof(it.name) - 1);
    it.defense_bonus = 5;
    it.value         = 40;
    return it;
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
