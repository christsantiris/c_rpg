#include "../include/systems/renderer.h"
#include <ncurses.h>

// Display the dungeon on screen
void draw_map(Game *game) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (game->map[y][x] == WALL) {
                attron(COLOR_PAIR(COLOR_WALL));
                mvaddch(y, x, game->map[y][x]); // move cursor to x,y and places character
                attroff(COLOR_PAIR(COLOR_WALL));
            } else {
                attron(COLOR_PAIR(COLOR_FLOOR));
                mvaddch(y, x, game->map[y][x]);
                attroff(COLOR_PAIR(COLOR_FLOOR));
            }
        }
    }
}

// Draw the player on the screen
void draw_player(Game *game) {
    attron(COLOR_PAIR(COLOR_PLAYER));
    mvaddch(game->player.y, game->player.x, game->player.symbol); // draw player on map
    attroff(COLOR_PAIR(COLOR_PLAYER));
}

// Draw the enemies on the screen (this will happen in a loop on init)
void draw_enemy(Game *game, int enemy_index) {
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