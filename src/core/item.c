#include "../../include/core/item.h"
#include <string.h>
#include <stdio.h>

static int next_item_id = 1;

// Create a weapon item
Item create_weapon(const char* name, int attack_bonus) {
    Item weapon = {0};
    strcpy(weapon.name, name);
    snprintf(weapon.description, sizeof(weapon.description), "A weapon that grants +%d attack", attack_bonus);
    weapon.type = ITEM_TYPE_WEAPON;
    weapon.attack_bonus = attack_bonus;
    weapon.defense_bonus = 0;
    weapon.equipped = 0;
    weapon.heal_amount = 0;
    weapon.use_function = NULL;
    weapon.item_id = next_item_id++;
    return weapon;
}

// Create a healing potion (single-use consumable item)
Item create_healing_potion(const char* name, int heal_amount) {
    Item potion = {0};
    strcpy(potion.name, name);
    snprintf(potion.description, sizeof(potion.description), "Restores %d HP (single use)", heal_amount);
    potion.type = ITEM_TYPE_CONSUMABLE;
    potion.attack_bonus = 0;
    potion.defense_bonus = 0;
    potion.equipped = 0;
    potion.heal_amount = heal_amount;
    potion.use_function = use_healing_potion;  // Set the function pointer
    potion.item_id = next_item_id++;
    return potion;
}

// Healing potion use function (single use - item will be removed after use)
int use_healing_potion(Item* item, Player* player) {
    if (player->current_hp >= player->max_hp) {
        printf("You are already at full health!\n");
        return 0;  // Failed to use
    }
    
    // Apply healing
    int old_hp = player->current_hp;
    player->current_hp += item->heal_amount;
    if (player->current_hp > player->max_hp) {
        player->current_hp = player->max_hp;
    }
    
    int healed = player->current_hp - old_hp;
    
    printf("You drink the %s and restore %d HP! (HP: %d/%d)\n", 
           item->name, healed, player->current_hp, player->max_hp);
    printf("The %s is consumed.\n", item->name);
    
    return 1;  // Successfully used - item will be removed by calling function
}

// Equip a weapon to the player
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

// Initialize inventory
void init_inventory(Inventory* inventory) {
    inventory->item_count = 0;
    // Initialize all items to zero
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        memset(&inventory->items[i], 0, sizeof(Item));
    }
}

// Add item to inventory
int add_item_to_inventory(Inventory* inventory, Item item) {
    if (inventory->item_count >= MAX_INVENTORY_SIZE) {
        printf("Inventory is full! Cannot add %s.\n", item.name);
        return 0;  // Failed to add
    }
    
    inventory->items[inventory->item_count] = item;
    inventory->item_count++;
    printf("Added %s to inventory.\n", item.name);
    return 1;  // Successfully added
}

// Remove item from inventory
int remove_item_from_inventory(Inventory* inventory, int item_index) {
    if (item_index < 0 || item_index >= inventory->item_count) {
        return 0;  // Invalid index
    }
    
    // Shift items down to fill the gap
    for (int i = item_index; i < inventory->item_count - 1; i++) {
        inventory->items[i] = inventory->items[i + 1];
    }
    
    inventory->item_count--;
    return 1;  // Successfully removed
}

// Use item from inventory (single-use items are automatically removed)
int use_item_from_inventory(Player* player, int item_index) {
    if (item_index < 0 || item_index >= player->inventory.item_count) {
        return 0;  // Invalid item selection
    }
    
    Item* item = &player->inventory.items[item_index];
    
    // Check if item is consumable and has a use function
    if (item->type != ITEM_TYPE_CONSUMABLE || item->use_function == NULL) {
        return 0;  // This item cannot be used
    }
    
    // Use the item
    int success = item->use_function(item, player);
    
    // Remove item after successful use (single-use consumables)
    if (success) {
        remove_item_from_inventory(&player->inventory, item_index);
    }
    
    return success;
}

// Get item from inventory
Item* get_item_from_inventory(Inventory* inventory, int item_index) {
    if (item_index < 0 || item_index >= inventory->item_count) {
        return NULL;
    }
    return &inventory->items[item_index];
}