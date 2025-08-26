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