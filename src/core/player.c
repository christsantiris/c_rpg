#include "../../include/core/player.h"
#include "../../include/core/core.h"
#include "../../include/core/combat.h"
#include "../../include/core/dungeon.h"
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
            // Move into enemy's space since they're dead
            game->player.x = new_x;
            game->player.y = new_y;
        }
        // If enemy survives, player stays in current position
    } else {
        // Normal movement - no enemy at target position
        game->player.x = new_x;
        game->player.y = new_y;
        // Reset message when moving without combat
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
            
            // Show level transition message
            strcpy(game->recentlyDefeated, "");
            snprintf(game->recentlyDefeated, sizeof(game->recentlyDefeated), 
                     "Welcome to Level %d!", game->current_level);
            game->showMessage = 1;
        }
    }
}