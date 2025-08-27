#include "../include/systems/renderer.h"
#include "../include/systems/viewport.h" 
#include <ncurses.h>

// Display the dungeon on screen
void draw_map(Game *game) {
    // Check if viewport is initialized
    if (game->viewport.viewport_width > 0 && game->viewport.viewport_height > 0) {
        // NEW: Viewport-based rendering
        for (int screen_y = 0; screen_y < game->viewport.viewport_height; screen_y++) {
            for (int screen_x = 0; screen_x < game->viewport.viewport_width; screen_x++) {
                // Convert screen coordinates to world coordinates
                int world_x = screen_x + game->viewport.viewport_x;
                int world_y = screen_y + game->viewport.viewport_y;
                
                // Make sure we're within the actual map bounds
                if (world_x >= 0 && world_x < MAP_WIDTH && world_y >= 0 && world_y < MAP_HEIGHT) {
                    if (game->map[world_y][world_x] == WALL) {
                        attron(COLOR_PAIR(COLOR_WALL));
                        mvaddch(screen_y, screen_x, WALL);  // Draw at screen coordinates
                        attroff(COLOR_PAIR(COLOR_WALL));
                    } else {
                        attron(COLOR_PAIR(COLOR_FLOOR));
                        mvaddch(screen_y, screen_x, game->map[world_y][world_x]);
                        attroff(COLOR_PAIR(COLOR_FLOOR));
                    }
                }
            }
        }
    } else {
        // FALLBACK: Original full-screen rendering
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (game->map[y][x] == WALL) {
                    attron(COLOR_PAIR(COLOR_WALL));
                    mvaddch(y, x, game->map[y][x]);
                    attroff(COLOR_PAIR(COLOR_WALL));
                } else {
                    attron(COLOR_PAIR(COLOR_FLOOR));
                    mvaddch(y, x, game->map[y][x]);
                    attroff(COLOR_PAIR(COLOR_FLOOR));
                }
            }
        }
    }
}

// Draw the player on the screen
void draw_player(Game *game) {
    if (game->viewport.viewport_width > 0) {
        // Convert player's world position to screen position
        int screen_x = world_to_screen_x(game, game->player.x);
        int screen_y = world_to_screen_y(game, game->player.y);
        
        // Only draw if player is visible
        if (is_in_viewport(game, game->player.x, game->player.y)) {
            attron(COLOR_PAIR(COLOR_PLAYER));
            mvaddch(screen_y, screen_x, game->player.symbol);
            attroff(COLOR_PAIR(COLOR_PLAYER));
        }
    } else {
        // Fallback to original
        attron(COLOR_PAIR(COLOR_PLAYER));
        mvaddch(game->player.y, game->player.x, game->player.symbol);
        attroff(COLOR_PAIR(COLOR_PLAYER));
    }
}

// Draw the enemies on the screen (this will happen in a loop on init)
void draw_enemy(Game *game, int enemy_index) {
    if (!game->enemies[enemy_index].active) return;
    
    if (game->viewport.viewport_width > 0) {
        // Only draw if enemy is visible
        if (is_in_viewport(game, game->enemies[enemy_index].x, game->enemies[enemy_index].y)) {
            int screen_x = world_to_screen_x(game, game->enemies[enemy_index].x);
            int screen_y = world_to_screen_y(game, game->enemies[enemy_index].y);
            
            // Your existing color logic
            int color_pair = COLOR_ENEMY;
            switch (game->enemies[enemy_index].type) {
                case ENEMY_GOBLIN: color_pair = COLOR_ENEMY; break;
                case ENEMY_ORC: color_pair = COLOR_TEXT; break;
                case ENEMY_SKELETON: color_pair = COLOR_FLOOR; break;
                case ENEMY_TROLL: color_pair = COLOR_WALL; break;
            }
            
            attron(COLOR_PAIR(color_pair));
            mvaddch(screen_y, screen_x, game->enemies[enemy_index].symbol);
            attroff(COLOR_PAIR(color_pair));
        }
    } else {
        if (game->enemies[enemy_index].active) {
            // Choose color based on enemy type
            int color_pair = COLOR_ENEMY; // default red
            
            switch (game->enemies[enemy_index].type) {
                case ENEMY_GOBLIN:
                    color_pair = COLOR_ENEMY;    // Red
                    break;
                case ENEMY_ORC:
                    color_pair = COLOR_TEXT;     // Cyan  
                    break;
                case ENEMY_SKELETON:
                    color_pair = COLOR_FLOOR;    // Yellow
                    break;
                case ENEMY_TROLL:
                    color_pair = COLOR_WALL;     // White
                    break;
            }
            
            attron(COLOR_PAIR(color_pair));
            mvaddch(game->enemies[enemy_index].y, game->enemies[enemy_index].x, 
                    game->enemies[enemy_index].symbol);
            attroff(COLOR_PAIR(color_pair));
        }
    }
}

void place_stairs(Game *game) {
    // Place stairs in the last room (farthest from player)
    if (game->room_count > 0) {
        Rectangle last_room = game->rooms[game->room_count - 1];
        
        // Put stairs in center of last room
        int stairs_x = last_room.x + last_room.width / 2;
        int stairs_y = last_room.y + last_room.height / 2;
        
        // Make sure stairs aren't on player or enemy
        int position_free = 1;
        if (stairs_x == game->player.x && stairs_y == game->player.y) {
            position_free = 0;
        }
        
        for (int i = 0; i < game->enemy_count; i++) {
            if (game->enemies[i].active && 
                game->enemies[i].x == stairs_x && 
                game->enemies[i].y == stairs_y) {
                position_free = 0;
                break;
            }
        }
        
        if (position_free) {
            game->map[stairs_y][stairs_x] = STAIRS_DOWN;
        }
    }
}