#include "../include/game.h"

int main() {
    Game game;
    
    // Initialize game
    init_ncurses();
    init_game(&game);
    
    // Main game loop
    int running = 1;
    while (running) {
        // Clear screen and redraw world
        clear();
        draw_map(&game);
        draw_player(&game);
        
        // Show instructions at the bottom
        mvprintw(MAP_HEIGHT + 2, 0, "Use arrow keys to move, 'q' to quit");
        
        // Refresh screen to render changes
        refresh();
        
        // Handle player input
        running = handle_input(&game);
    }
    
    // Clean up and exit
    cleanup_ncurses();
    printf("Thanks for playing!\n");
    return 0;
}

void init_ncurses() {
    // Initialize ncurses
    initscr();
    
    // Don't echo pressed keys to screen
    noecho();
    
    // Enable special keys (arrow keys, function keys, etc.)
    keypad(stdscr, TRUE);
    
    // Hide cursor
    curs_set(0);
}

void cleanup_ncurses() {
    endwin();
}

void init_game(Game *game) {
    // Initialize player starting position (center)
    game->player.x = MAP_WIDTH / 2;
    game->player.y = MAP_HEIGHT / 2;
    game->player.symbol = PLAYER;
    
    // Create a rectangular room
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Walls around the border
            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
                game->map[y][x] = WALL;
            } else {
                // Floor inside
                game->map[y][x] = FLOOR;
            }
        }
    }
}

void draw_map(Game *game) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // mvaddch moves cursor to (y,x) and adds character
            mvaddch(y, x, game->map[y][x]);
        }
    }
}

void draw_player(Game *game) {
    // Draw player on top of the map
    mvaddch(game->player.y, game->player.x, game->player.symbol);
}

int handle_input(Game *game) {
    int ch = getch(); // Get a character from user
    
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
            move_player(game, 0, -1); // Move up (y decreases)
            break;
            
        case KEY_DOWN:
        case 's':
        case 'S':
            move_player(game, 0, 1); // Move down (y increases)
            break;
            
        case KEY_LEFT:
        case 'a':
        case 'A':
            move_player(game, -1, 0); // Move left (x decreases)
            break;
            
        case KEY_RIGHT:
        case 'd':
        case 'D':
            move_player(game, 1, 0); // Move right (x increases)
            break;
            
        case 'q':
        case 'Q':
        case KEY_ESC:
            return 0; // Quit game
            
        default:
            // Unknown key, do nothing
            break;
    }
    
    return 1; // Continue game
}

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
    
    // Only move if the new position is valid
    if (is_valid_move(game, new_x, new_y)) {
        game->player.x = new_x;
        game->player.y = new_y;
    }
    // If invalid move, just ignore it (player doesn't move)
}