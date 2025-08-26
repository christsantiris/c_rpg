#include "../include/game.h"

int is_valid_move(Game *game, int new_x, int new_y) {
    // Check bounds
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        return 0;
    }
    
    // Check if the tile is walkable (not a wall)
    if (game->map[new_y][new_x] == WALL) {
        return 0;
    }
    
    return 1; // Valid move
}

void move_player(Game *game, int dx, int dy) {
    int new_x = game->player.x + dx;
    int new_y = game->player.y + dy;
    
    if (is_valid_move(game, new_x, new_y)) {
        game->turn_count++;
        
        // Check for combat with any enemy
        int enemy_hit = -1;
        char enemyName[15];
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
            // Player attacks enemy - enemy is defeated
            game->enemies[enemy_hit].active = 0;
            game->enemies_killed++;
            game->player.x = new_x;
            game->player.y = new_y;
            strcpy(game->recentlyDefeated, enemyName);
            game->showMessage = 1;
        } else {
            // Normal movement
            game->player.x = new_x;
            game->player.y = new_y;
        }
    }

    // Move all active enemies
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active) {
            move_enemy(game, i);
        }
    }
}