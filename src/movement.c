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

int is_valid_player_move(Game *game, int new_x, int new_y) {
    // First check if it's a valid move (not wall, in bounds)
    if (!is_valid_move(game, new_x, new_y)) {
        return 0;
    }
    
    // If enemy is at the target position and enemy is active, this will trigger combat
    // We allow the move so combat can be handled in move_player()
    return 1; // Valid move (combat will be resolved if needed)
}

int is_valid_enemy_move(Game *game, int new_x, int new_y) {
    // First check if it's a valid move (not wall, in bounds)
    if (!is_valid_move(game, new_x, new_y)) {
        return 0;
    }
    
    // If player is at the target position, this will trigger combat
    // We allow the move so combat can be handled in move_enemy()
    return 1; // Valid move (combat will be resolved if needed)
}

void move_player(Game *game, int dx, int dy) {
    int new_x = game->player.x + dx;
    int new_y = game->player.y + dy;
    
    // Only move if the new position is valid (terrain-wise)
    if (is_valid_player_move(game, new_x, new_y)) {
        // Increment turn counter
        game->turn_count++;
        // Check for combat with enemy
        if (game->enemy.active && new_x == game->enemy.x && new_y == game->enemy.y) {
            // Player attacks enemy - enemy is defeated
            game->enemy.active = 0;
            // Player moves to enemy's position
            game->player.x = new_x;
            game->player.y = new_y;
        } else {
            // Normal movement
            game->player.x = new_x;
            game->player.y = new_y;
        }
    }
    // If invalid move, just ignore it (player doesn't move)

    // Move enemy towards player (only if enemy is still active)
    if (game->enemy.active) {
        move_enemy(game);
    }
}

// Move enemy
void move_enemy(Game *game) {
    // Don't move if enemy is not active or game is over
    if (!game->enemy.active || game->game_over) return;
    
    int enemy_x = game->enemy.x;
    int enemy_y = game->enemy.y;
    int player_x = game->player.x;
    int player_y = game->player.y;
    
    int dx = 0, dy = 0;
    
    // Move horizontally towards player
    if (enemy_x < player_x) {
        dx = 1;  // Move right
    } else if (enemy_x > player_x) {
        dx = -1; // Move left
    }
    
    // Move vertically towards player
    if (enemy_y < player_y) {
        dy = 1;  // Move down
    } else if (enemy_y > player_y) {
        dy = -1; // Move up
    }
    
    // Try to move (check if new position is valid terrain-wise)
    int new_x = enemy_x + dx;
    int new_y = enemy_y + dy;
    
    if (is_valid_enemy_move(game, new_x, new_y)) {
        // Check for combat with player
        if (new_x == player_x && new_y == player_y) {
            // Enemy attacks player - player is defeated, game over
            game->game_over = 1;
            // Enemy moves to player's position
            game->enemy.x = new_x;
            game->enemy.y = new_y;
        } else {
            // Normal movement
            game->enemy.x = new_x;
            game->enemy.y = new_y;
        }
    }
}