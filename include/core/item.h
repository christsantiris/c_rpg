#ifndef ITEM_H
#define ITEM_H

#include "types.h"

// Item creation functions
Item create_weapon(const char* name, int attack_bonus);

// Equipment functions
void equip_weapon(Player* player, Item weapon);
int has_weapon_equipped(Player* player);

#endif