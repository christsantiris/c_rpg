#include "item.h"
#include <string.h>
#include "spell.h"

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

Item item_make_weapon(const char *name, int attack_bonus, int value) {
    Item it = {0};
    it.active       = 1;
    it.type         = ITEM_WEAPON;
    strncpy(it.name, name, sizeof(it.name) - 1);
    it.attack_bonus = attack_bonus;
    it.value        = value;
    return it;
}

Item item_make_rusty_sword(void) {
    Item it = {0};
    it.active       = 1;
    it.type         = ITEM_WEAPON;
    strncpy(it.name, "Rusty Sword", sizeof(it.name) - 1);
    it.attack_bonus = 1;
    it.value        = 5;
    return it;
}

Item item_make_short_sword(void) {
    Item it = {0};
    it.active       = 1;
    it.type         = ITEM_WEAPON;
    strncpy(it.name, "Short Sword", sizeof(it.name) - 1);
    it.attack_bonus = 3;
    it.value        = 15;
    return it;
}

Item item_make_long_sword(void) {
    Item it = {0};
    it.active       = 1;
    it.type         = ITEM_WEAPON;
    strncpy(it.name, "Long Sword", sizeof(it.name) - 1);
    it.attack_bonus = 6;
    it.value        = 30;
    return it;
}

Item item_make_battle_axe(void) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_WEAPON;
    strncpy(it.name, "Battle Axe", sizeof(it.name) - 1);
    it.attack_bonus  = 10;
    it.is_two_handed = 1;
    it.value         = 50;
    return it;
}

Item item_make_staff(void) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_WEAPON;
    strncpy(it.name, "Staff", sizeof(it.name) - 1);
    it.attack_bonus  = 4;
    it.is_two_handed = 1;
    it.value         = 25;
    return it;
}

Item item_make_bow(void) {
    Item it = {0};
    it.active       = 1;
    it.type         = ITEM_WEAPON;
    strncpy(it.name, "Bow", sizeof(it.name) - 1);
    it.attack_bonus = 5;
    it.is_ranged    = 1;
    it.range        = 8;
    it.value        = 35;
    return it;
}

Item item_make_armor(const char *name, int defense_bonus, int value) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_ARMOR;
    strncpy(it.name, name, sizeof(it.name) - 1);
    it.defense_bonus = defense_bonus;
    it.value         = value;
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