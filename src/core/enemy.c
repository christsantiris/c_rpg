#include "../../include/core/enemy.h"
#include "../../include/core/core.h"
#include "../../include/core/combat.h"

int is_valid_enemy_move(Game *game, int new_x, int new_y) {
    return is_valid_move(game, new_x, new_y);
}

// Move enemy
void move_enemy(Game *game, int enemy_index) {
    if (!game->enemies[enemy_index].active || game->game_over) return;
    
    int enemy_x = game->enemies[enemy_index].x;
    int enemy_y = game->enemies[enemy_index].y;
    int player_x = game->player.x;
    int player_y = game->player.y;
    
    int dx = 0, dy = 0;
    
    // Move towards player
    if (enemy_x < player_x) dx = 1;
    else if (enemy_x > player_x) dx = -1;
    
    if (enemy_y < player_y) dy = 1;
    else if (enemy_y > player_y) dy = -1;
    
    int new_x = enemy_x + dx;
    int new_y = enemy_y + dy;
    
    // Check if target position is the player
    if (new_x == player_x && new_y == player_y) {
        // Attack player, but don't move into their space
        int player_died = enemy_attack_player(&game->enemies[enemy_index], &game->player);
        if (player_died) {
            game->game_over = 1;
        }
        return; // Don't move, just attack
    }
    
    // Normal movement - use is_valid_move for terrain and entity blocking
    if (is_valid_move(game, new_x, new_y)) {
        game->enemies[enemy_index].x = new_x;
        game->enemies[enemy_index].y = new_y;
    }
}