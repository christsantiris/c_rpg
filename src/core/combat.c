#include "../../include/core/types.h"
#include "../../include/core/core.h" 
#include <stdio.h>
#include <stdlib.h>

// Calculate damage based on attacker vs defender stats
int calculate_damage(int attack, int defense) {
    int base_damage = attack - defense;
    if (base_damage < 1) {
        base_damage = 1; // Always do at least 1 damage
    }
    
    // Add some randomness (±25%)
    int variation = base_damage / 4;
    if (variation < 1) variation = 1;
    
    int final_damage = base_damage + (rand() % (variation * 2 + 1)) - variation;
    if (final_damage < 1) final_damage = 1;
    
    return final_damage;
}

// Player attacks enemy - returns 1 if enemy dies, 0 if survives
int player_attack_enemy(Player* player, Enemy* enemy) {
    int damage = calculate_damage(player->attack, enemy->defense);
    enemy->current_hp -= damage;
    
    printf("You deal %d damage to %s! ", damage, enemy->name);
    
    if (enemy->current_hp <= 0) {
        enemy->active = 0; // Enemy dies
        printf("%s is defeated!\n", enemy->name);
        
        // Get experience
        gain_experience(player, enemy->experience);    
        printf("You gain %d experience!\n", enemy->experience);
        
        // Check if we just leveled up (gain_experience handles the leveling)
        static int last_level = 1;
        if (player->level > last_level) {
            printf("*** LEVEL UP! *** You are now level %d!\n", player->level);
            printf("Attack: %d, Defense: %d, HP: %d/%d\n", 
                player->attack, player->defense, player->current_hp, player->max_hp);
            last_level = player->level;
        }
        
        return 1; // Enemy died
    } else {
        printf("%s has %d HP left.\n", enemy->name, enemy->current_hp);
        return 0; // Enemy survives
    }
}

// Enemy attacks player - returns 1 if player dies, 0 if survives  
int enemy_attack_player(Enemy* enemy, Player* player) {
    int damage = calculate_damage(enemy->attack, player->defense);
    player->current_hp -= damage;
    
    printf("%s deals %d damage to you! ", enemy->name, damage);
    
    if (player->current_hp <= 0) {
        printf("You have been defeated!\n");
        return 1; // Player died
    } else {
        printf("You have %d HP left.\n", player->current_hp);
        return 0; // Player survives
    }
}