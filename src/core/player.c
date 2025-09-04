#include "../../include/core/player.h"
#include "../../include/core/core.h"
#include "../../include/core/combat.h"
#include "../../include/core/dungeon.h"
#include "../../include/systems/viewport.h"
#include <string.h>
#include <ncurses.h>

int is_valid_player_move(Game *game, int new_x, int new_y) {
    return is_valid_move(game, new_x, new_y);
}

void move_player(Game *game, int dx, int dy) {
    int new_x = game->player.x + dx;
    int new_y = game->player.y + dy;
    
    // Check bounds and walls first
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        return; // Out of bounds
    }
    
    if (game->map[new_y][new_x] == WALL) {
        return; // Hit a wall
    }
    
    game->turn_count++;
    
    // Check for combat with any enemy at target position
    int enemy_hit = -1;
    char enemyName[32];
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active && 
            new_x == game->enemies[i].x && 
            new_y == game->enemies[i].y) {
            enemy_hit = i;
            strcpy(enemyName, game->enemies[i].name);
            break;
        }
    }
    
    if (enemy_hit != -1) {
        // Attack enemy, but don't move into their space unless they die
        int enemy_died = player_attack_enemy(&game->player, &game->enemies[enemy_hit]);
        
        if (enemy_died) {
            game->enemies_killed++;
            strcpy(game->recentlyDefeated, enemyName);
            game->showMessage = 1;
            game->showLevelMessage = 0;  // Clear level message when combat happens
            // Move into enemy's space since they're dead
            game->player.x = new_x;
            game->player.y = new_y;
        }
        // If enemy survives, player stays in current position
    } else {
        // Normal movement - no enemy at target position
        game->player.x = new_x;
        game->player.y = new_y;
        // Reset combat message when moving without combat
        game->showMessage = 0;
        
        // Check if player stepped on stairs
        if (game->map[new_y][new_x] == STAIRS_DOWN && game->waiting_for_stairs) {
            // Go to next level!
            game->current_level++;
            game->waiting_for_stairs = 0;
            
            // Regenerate dungeon and enemies for new level
            generate_dungeon(game);
            
            // Place player in first room of new level
            if (game->room_count > 0) {
                game->player.x = game->rooms[0].x + game->rooms[0].width / 2;
                game->player.y = game->rooms[0].y + game->rooms[0].height / 2;
            }
            
            // Create new enemies (more enemies on deeper levels)
            create_level_enemies(game);
            
            // Show level transition message using separate message system
            if (is_boss_level(game->current_level)) {
                snprintf(game->levelMessage, sizeof(game->levelMessage), 
                "BOSS LEVEL %d - Prepare for Battle!", game->current_level);
            } else {
                snprintf(game->levelMessage, sizeof(game->levelMessage), 
                "Dungeon Level %d!", game->current_level);
            }

            game->showLevelMessage = 1;
            game->showMessage = 0;  // Clear combat message
        }
    }
    
    // Center viewport on player after movement
    center_viewport_on_player(game);
}

int calculate_experience_needed(int level) {
    // Formula: Level * 100 experience needed to reach next level
    // Level 1->2: 100 XP, Level 2->3: 200 XP, etc.
    return level * 100;
}

int check_level_up(Player* player) {
    // Check if player has enough experience to level up
    while (player->experience >= player->experience_to_next && player->level < 18) {
        // Level up!
        player->experience -= player->experience_to_next;  // Subtract used XP
        player->level++;
        
        // Increase stats
        player->attack += 2;
        player->defense += 1;
        player->max_hp += 10;
        player->current_hp = player->max_hp;  // Full heal on level up
        
        // Calculate next level requirement
        if (player->level < 18) {
            player->experience_to_next = calculate_experience_needed(player->level);
        } else {
            player->experience_to_next = 0;  // Max level reached
        }
        
        return 1;  // Leveled up
    }
    
    return 0;  // No level up
}

void gain_experience(Player* player, int exp) {
    player->experience += exp;
    check_level_up(player);  // Automatically check for level up
}