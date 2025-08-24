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
        draw_enemy(&game);
        
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
    
    // Enable special keys
    keypad(stdscr, TRUE);
    
    // Hide cursor
    curs_set(0);
}

void cleanup_ncurses() {
    endwin();
}

void init_game(Game *game) {
    // Random number generator
    srand(time(NULL));
    
    // Initialize player
    game->player.symbol = PLAYER;

    // Initiaize enemy
    game->enemy.symbol = ENEMY;

    // Initialize room count
    game->room_count = 0;
    
    // Generate the dungeon
    generate_dungeon(game);
    
    // Place player in the center of the first room
    if (game->room_count > 0) {
        game->player.x = game->rooms[0].x + game->rooms[0].width / 2;
        game->player.y = game->rooms[0].y + game->rooms[0].height / 2;

        // Place enemy randomly in the first room
        game->enemy.x = random_range(game->rooms[0].x + 1, game->rooms[0].x + game->rooms[0].width - 2);
        game->enemy.y = random_range(game->rooms[0].y + 1, game->rooms[0].y + game->rooms[0].height - 2);
        game->enemy.symbol = ENEMY;
        strcpy(game->enemy.name, "Goblin");
    } else {
        // Fallback if no rooms generated
        game->player.x = MAP_WIDTH / 2;
        game->player.y = MAP_HEIGHT / 2;
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

void draw_enemy(Game *game) {
    // Draw player on top of the map
    mvaddch(game->enemy.y, game->enemy.x, game->enemy.symbol);
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
            // Invalid key, do nothing
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