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
        
        // Show instructions and game status at the bottom
        mvprintw(MAP_HEIGHT + 2, 0, "Use arrow keys to move, 'q' to quit");
        if (!game.enemy.active) {
            mvprintw(MAP_HEIGHT + 3, 0, "Victory! You defeated the %s!", game.enemy.name);
        }
        if (game.game_over) {
            mvprintw(MAP_HEIGHT + 3, 0, "Game Over! The %s defeated you!", game.enemy.name);
            mvprintw(MAP_HEIGHT + 4, 0, "Press any key to exit...");
        }
        
        // Refresh screen to render changes
        refresh();
        
        // Handle player input
        running = handle_input(&game);
        
        // Check for game over
        if (game.game_over) {
            // Wait for any key press before exiting
            getch();
            running = 0;
        }
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

    // Initialize enemy
    game->enemy.symbol = ENEMY;
    game->enemy.active = 1;  // Enemy starts alive
    
    // Initialize game state
    game->game_over = 0;  // Game starts running

    // Initialize room count
    game->room_count = 0;
    
    // Generate the dungeon
    generate_dungeon(game);
    
    // Place player in the center of the first room
    if (game->room_count > 0) {
        game->player.x = game->rooms[0].x + game->rooms[0].width / 2;
        game->player.y = game->rooms[0].y + game->rooms[0].height / 2;

        // Place enemy randomly in the first room, but not on player
        do {
            game->enemy.x = random_range(game->rooms[0].x + 1, game->rooms[0].x + game->rooms[0].width - 2);
            game->enemy.y = random_range(game->rooms[0].y + 1, game->rooms[0].y + game->rooms[0].height - 2);
        } while (game->enemy.x == game->player.x && game->enemy.y == game->player.y);
        
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
    // Only draw enemy if it's active (alive)
    if (game->enemy.active) {
        mvaddch(game->enemy.y, game->enemy.x, game->enemy.symbol);
    }
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

        case '1':  // Up-left diagonal
            move_player(game, -1, -1);
            break;

        case '0':  // Up-right diagonal  
            move_player(game, 1, -1);
            break;

        case 'z':  // Down-left diagonal
            move_player(game, -1, 1);
            break;

        case 'm':  // Down-right diagonal
            move_player(game, 1, 1);
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