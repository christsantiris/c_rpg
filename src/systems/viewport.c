#include "../../include/systems/viewport.h"
#include "../../include/systems/double_buffer.h"
#include <ncurses.h>
#include <signal.h>

// Global pointer for handling terminal resize
static Game* global_game_ptr = NULL;

// This function runs when the user resizes their terminal
void handle_resize(int sig) {
    (void)sig;
    
    endwin();
    refresh();
    
    if (global_game_ptr) {
        update_viewport_info(global_game_ptr);
        mark_buffer_resize_needed(&global_game_ptr->double_buffer);  // Add this line
    }
}

// Calculate how much of the map we can show based on terminal size
void update_viewport_info(Game *game) {
    // Get current terminal size
    getmaxyx(stdscr, game->viewport.terminal_height, game->viewport.terminal_width);
    
    // Calculate viewport size (leave room for UI elements)
    game->viewport.viewport_width = game->viewport.terminal_width - 2;   // 2 for borders
    game->viewport.viewport_height = game->viewport.terminal_height - 8; // 8 for UI at bottom
    
    // Don't try to show more than the actual map
    if (game->viewport.viewport_width > MAP_WIDTH) {
        game->viewport.viewport_width = MAP_WIDTH;
    }
    if (game->viewport.viewport_height > MAP_HEIGHT) {
        game->viewport.viewport_height = MAP_HEIGHT;
    }
    
    // Make sure viewport is at least 10x10 (minimum playable size)
    if (game->viewport.viewport_width < 10) game->viewport.viewport_width = 10;
    if (game->viewport.viewport_height < 10) game->viewport.viewport_height = 10;
    
    // Update viewport position to keep player centered
    center_viewport_on_player(game);
}

// Move the "camera" to keep the player in the center of the screen
void center_viewport_on_player(Game *game) {
    // Center viewport on player
    game->viewport.viewport_x = game->player.x - game->viewport.viewport_width / 2;
    game->viewport.viewport_y = game->player.y - game->viewport.viewport_height / 2;
    
    // Don't let viewport go outside the map boundaries
    if (game->viewport.viewport_x < 0) {
        game->viewport.viewport_x = 0;
    }
    if (game->viewport.viewport_y < 0) {
        game->viewport.viewport_y = 0;
    }
    if (game->viewport.viewport_x + game->viewport.viewport_width > MAP_WIDTH) {
        game->viewport.viewport_x = MAP_WIDTH - game->viewport.viewport_width;
    }
    if (game->viewport.viewport_y + game->viewport.viewport_height > MAP_HEIGHT) {
        game->viewport.viewport_y = MAP_HEIGHT - game->viewport.viewport_height;
    }
}

// Enhanced initialization that handles viewport
void init_ncurses_with_viewport(Game *game) {
    global_game_ptr = game;              // Store for resize handler
    signal(SIGWINCH, handle_resize);     // Handle terminal resize
    
    initscr();  // Initialize ncurses
    
    // Your existing color setup
    if (has_colors()) {
        start_color();
        init_pair(COLOR_WALL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_FLOOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_PLAYER, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_ENEMY, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_TEXT, COLOR_CYAN, COLOR_BLACK);
    }
    
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // Calculate initial viewport
    update_viewport_info(game);
}

// Convert world coordinates to screen coordinates
int world_to_screen_x(Game *game, int world_x) {
    return world_x - game->viewport.viewport_x;
}

int world_to_screen_y(Game *game, int world_y) {
    return world_y - game->viewport.viewport_y;
}

// Check if something is visible on screen
int is_in_viewport(Game *game, int world_x, int world_y) {
    int screen_x = world_to_screen_x(game, world_x);
    int screen_y = world_to_screen_y(game, world_y);
    
    return (screen_x >= 0 && screen_x < game->viewport.viewport_width &&
            screen_y >= 0 && screen_y < game->viewport.viewport_height);
}

// Check if player is getting close to viewport edges
int needs_viewport_update(Game *game) {
    int margin = 5; // How close to edge before we scroll
    
    int player_screen_x = game->player.x - game->viewport.viewport_x;
    int player_screen_y = game->player.y - game->viewport.viewport_y;
    
    // Check if player is too close to edges
    if (player_screen_x < margin || 
        player_screen_x >= game->viewport.viewport_width - margin ||
        player_screen_y < margin || 
        player_screen_y >= game->viewport.viewport_height - margin) {
        return 1;
    }
    
    // Check if player is outside viewport (shouldn't happen but safety check)
    if (player_screen_x < 0 || player_screen_x >= game->viewport.viewport_width ||
        player_screen_y < 0 || player_screen_y >= game->viewport.viewport_height) {
        return 1;
    }
    
    return 0;
}

// Optimized centering that only updates when needed
void center_viewport_on_player_if_needed(Game *game) {
    if (needs_viewport_update(game)) {
        center_viewport_on_player(game);
    }
}