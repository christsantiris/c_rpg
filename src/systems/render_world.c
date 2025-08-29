#include "../../include/systems/renderer.h"
#include <ncurses.h>
#include <stdarg.h>

// Helper function to get the window to draw to
WINDOW* get_draw_window(Game *game) {
    if (game->double_buffer.back_buffer) {
        return game->double_buffer.back_buffer;
    }
    return stdscr;  // Fallback to direct screen drawing
}

// Display the dungeon on screen with viewport and double buffer support
void draw_map(Game *game) {
    WINDOW *win = get_draw_window(game);
    
    // If viewport is enabled, use viewport rendering
    if (game->viewport.viewport_width > 0 && game->viewport.viewport_height > 0) {
        for (int y = 0; y < game->viewport.viewport_height; y++) {
            for (int x = 0; x < game->viewport.viewport_width; x++) {
                int map_x = x + game->viewport.viewport_x;
                int map_y = y + game->viewport.viewport_y;
                
                // Make sure we're within map bounds
                if (map_x >= 0 && map_x < MAP_WIDTH && map_y >= 0 && map_y < MAP_HEIGHT) {
                    if (game->map[map_y][map_x] == WALL) {
                        wattron(win, COLOR_PAIR(COLOR_WALL));
                        mvwaddch(win, y, x, game->map[map_y][map_x]);
                        wattroff(win, COLOR_PAIR(COLOR_WALL));
                    } else {
                        wattron(win, COLOR_PAIR(COLOR_FLOOR));
                        mvwaddch(win, y, x, game->map[map_y][map_x]);
                        wattroff(win, COLOR_PAIR(COLOR_FLOOR));
                    }
                }
            }
        }
    } else {
        // Fallback to original full-map rendering
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (game->map[y][x] == WALL) {
                    wattron(win, COLOR_PAIR(COLOR_WALL));
                    mvwaddch(win, y, x, game->map[y][x]);
                    wattroff(win, COLOR_PAIR(COLOR_WALL));
                } else {
                    wattron(win, COLOR_PAIR(COLOR_FLOOR));
                    mvwaddch(win, y, x, game->map[y][x]);
                    wattroff(win, COLOR_PAIR(COLOR_FLOOR));
                }
            }
        }
    }
}

// Draw the player on screen with double buffer support
void draw_player(Game *game) {
    WINDOW *win = get_draw_window(game);
    int screen_x, screen_y;
    
    // If viewport is enabled, convert world coordinates to screen coordinates
    if (game->viewport.viewport_width > 0 && game->viewport.viewport_height > 0) {
        screen_x = game->player.x - game->viewport.viewport_x;
        screen_y = game->player.y - game->viewport.viewport_y;
        
        // Only draw if player is visible in viewport
        if (screen_x >= 0 && screen_x < game->viewport.viewport_width &&
            screen_y >= 0 && screen_y < game->viewport.viewport_height) {
            wattron(win, COLOR_PAIR(COLOR_PLAYER));
            mvwaddch(win, screen_y, screen_x, game->player.symbol);
            wattroff(win, COLOR_PAIR(COLOR_PLAYER));
        }
    } else {
        // Fallback to original rendering
        wattron(win, COLOR_PAIR(COLOR_PLAYER));
        mvwaddch(win, game->player.y, game->player.x, game->player.symbol);
        wattroff(win, COLOR_PAIR(COLOR_PLAYER));
    }
}

// Draw enemies with double buffer support
// Complete draw_enemy function for src/systems/render_world.c
void draw_enemy(Game *game, int enemy_index) {
    // Don't draw inactive (dead) enemies
    if (!game->enemies[enemy_index].active) return;
    
    WINDOW *win = get_draw_window(game);
    int screen_x, screen_y;
    
    // If viewport is enabled, convert world coordinates to screen coordinates
    if (game->viewport.viewport_width > 0 && game->viewport.viewport_height > 0) {
        screen_x = game->enemies[enemy_index].x - game->viewport.viewport_x;
        screen_y = game->enemies[enemy_index].y - game->viewport.viewport_y;
        
        // Only draw if enemy is visible in viewport
        if (screen_x < 0 || screen_x >= game->viewport.viewport_width ||
            screen_y < 0 || screen_y >= game->viewport.viewport_height) {
            return; // Enemy not visible, don't draw
        }
    } else {
        // Fallback to original rendering (no viewport)
        screen_x = game->enemies[enemy_index].x;
        screen_y = game->enemies[enemy_index].y;
    }
    
    // Choose color based on enemy type
    int color_pair = COLOR_ENEMY;
    switch (game->enemies[enemy_index].type) {
        case ENEMY_GOBLIN: 
            color_pair = COLOR_ENEMY; 
            break;
        case ENEMY_ORC: 
            color_pair = COLOR_TEXT; 
            break;
        case ENEMY_SKELETON: 
            color_pair = COLOR_FLOOR; 
            break;
        case ENEMY_TROLL: 
            color_pair = COLOR_WALL; 
            break;
        // Boss enemies get special bright green color to stand out
        case ENEMY_DRAGON: 
        case ENEMY_DEMON_LORD:
        case ENEMY_LICH_KING:
            color_pair = COLOR_PLAYER; // Bright green like player
            break;
        default:
            color_pair = COLOR_ENEMY; // Fallback color
            break;
    }
    
    // Check if this enemy is a boss
    if (game->enemies[enemy_index].type == ENEMY_DRAGON ||
        game->enemies[enemy_index].type == ENEMY_DEMON_LORD ||
        game->enemies[enemy_index].type == ENEMY_LICH_KING) {
        
        // Bosses get BOLD styling to make them extra visible
        wattron(win, COLOR_PAIR(color_pair) | A_BOLD);
        mvwaddch(win, screen_y, screen_x, game->enemies[enemy_index].symbol);
        wattroff(win, COLOR_PAIR(color_pair) | A_BOLD);
        
    } else {       
       // Regular enemies get normal styling
        wattron(win, COLOR_PAIR(color_pair));
        mvwaddch(win, screen_y, screen_x, game->enemies[enemy_index].symbol);
        wattroff(win, COLOR_PAIR(color_pair));
    }
}

// Draw UI text with double buffer support
void draw_ui_text(Game *game, int y, int x, const char *text) {
    WINDOW *win = get_draw_window(game);
    wattron(win, COLOR_PAIR(COLOR_TEXT));
    mvwprintw(win, y, x, "%s", text);
    wattroff(win, COLOR_PAIR(COLOR_TEXT));
}

// Draw formatted UI text with double buffer support
void draw_ui_textf(Game *game, int y, int x, const char *format, ...) {
    WINDOW *win = get_draw_window(game);
    va_list args;
    va_start(args, format);
    
    wattron(win, COLOR_PAIR(COLOR_TEXT));
    wmove(win, y, x);
    vw_printw(win, format, args);
    wattroff(win, COLOR_PAIR(COLOR_TEXT));
    
    va_end(args);
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