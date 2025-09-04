#ifndef ITEM_H
#define ITEM_H

#include "types.h"

// Item creation functions
Item create_weapon(const char* name, int attack_bonus);
Item create_healing_potion(const char* name, int heal_amount);

// Equipment functions
void equip_weapon(Player* player, Item weapon);
int has_weapon_equipped(Player* player);

// Consumable functions
int use_healing_potion(Item* item, Player* player);

// Inventory management functions
void init_inventory(Inventory* inventory);
int add_item_to_inventory(Inventory* inventory, Item item);
int remove_item_from_inventory(Inventory* inventory, int item_index);
int use_item_from_inventory(Player* player, int item_index);
Item* get_item_from_inventory(Inventory* inventory, int item_index);

#endif