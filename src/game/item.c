#include "item.h"
#include <string.h>

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

Item item_make_armor(const char *name, int defense_bonus, int value) {
    Item it = {0};
    it.active        = 1;
    it.type          = ITEM_ARMOR;
    strncpy(it.name, name, sizeof(it.name) - 1);
    it.defense_bonus = defense_bonus;
    it.value         = value;
    return it;
}