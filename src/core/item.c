#include "../../include/core/item.h"
#include <string.h>

// Create a weapon item
Item create_weapon(const char* name, int attack_bonus) {
    Item weapon = {0};
    strcpy(weapon.name, name);
    weapon.type = ITEM_TYPE_WEAPON;
    weapon.attack_bonus = attack_bonus;
    weapon.defense_bonus = 0;
    weapon.equipped = 0;
    return weapon;
}

// Equip a weapon to the player
// Replace the equip_weapon function with this corrected version:
void equip_weapon(Player* player, Item weapon) {
    player->weapon = weapon;
    player->weapon.equipped = 1;
    
    // Recalculate total attack (base + weapon bonus)
    player->attack = player->base_attack + player->weapon.attack_bonus;
}

// Check if player has a weapon equipped
int has_weapon_equipped(Player* player) {
    return player->weapon.equipped;
}