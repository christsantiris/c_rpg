#include "../include/game.h"

int main() {
    Game game;
    
    // Initialize ncurses
    init_ncurses();
    
    // Initialize game to menu state
    game.game_state = STATE_MENU;
    game.selected_menu = MENU_NEW_GAME; // Start with "New Game" selected
    game.game_over = 0;
    
    srand(time(NULL));

    // Main game loop
    int running = 1;
    while (running && !game.game_over) {
        
        if (game.game_state == STATE_MENU) {
            // Show title screen
            draw_title_screen(&game);
            handle_menu_input(&game);
            
        } else if (game.game_state == STATE_PLAYING) {
            // Original game loop
            clear();
            draw_map(&game);
            draw_player(&game);
            draw_enemy(&game);
            
            if (!game.game_over) {
                attron(COLOR_PAIR(COLOR_TEXT));
                mvprintw(MAP_HEIGHT + 2, 0, "Use arrow keys to move, 'q' to quit");
                mvprintw(MAP_HEIGHT + 3, 0, "Turn: %d | Enemies Killed: %d", 
                         game.turn_count, game.enemies_killed);
                
                if (!game.enemy.active) {
                    attron(A_BOLD);
                    mvprintw(MAP_HEIGHT + 4, 0, "Victory! You killed the %s! (ID: %d)", game.enemy.name, game.enemy.ID);
                    attroff(A_BOLD);
                }
                attroff(COLOR_PAIR(COLOR_TEXT));
            }
            
            refresh();
            running = handle_input(&game);
            
            if (game.game_over) {
                // Your existing game over screen code
                clear();
                mvprintw(5, 15, "  ####    ###   #   #  ####       ####  #   #  ####  ####");
                mvprintw(6, 15, " #       #   #  ## ##  #          #  #  #   #  #     #   #");
                mvprintw(7, 15, " # ###   #####  # # #  ###        #  #  #   #  ###   ####");
                mvprintw(8, 15, " #   #   #   #  #   #  #          #  #   # #   #     #  #");
                mvprintw(9, 15, "  ####   #   #  #   #  ####       ####    #    ####  #   #");
                
                mvprintw(12, 25, "The %s has defeated you!", game.enemy.name);
                mvprintw(14, 30, "Press any key to exit...");
                
                refresh();
                getch();
                running = 0;
            }
        }
    }
    
    cleanup_ncurses();
    printf("Thanks for playing!\n");
    return 0;
}

void init_ncurses() {
    // Initialize ncurses
    initscr();
    

    // Initialize colors
    if (has_colors()) {
        start_color();
        init_pair(COLOR_WALL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_FLOOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_PLAYER, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_ENEMY, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_TEXT, COLOR_CYAN, COLOR_BLACK);
    }
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

    // Simple unique IDs for enemies
    static int next_id = 1;
    
    // Initialize player
    game->player.symbol = PLAYER;

    // Initialize enemy
    game->enemy.ID = next_id;
    game->enemy.symbol = ENEMY;
    game->enemy.active = 1;  // Enemy starts alive
    
    game->turn_count = 0;
    game->enemies_killed = 0;

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

void draw_player(Game *game) {
    attron(COLOR_PAIR(COLOR_PLAYER));
    mvaddch(game->player.y, game->player.x, game->player.symbol); // draw player on map
    attroff(COLOR_PAIR(COLOR_PLAYER));
}

void draw_enemy(Game *game) {
    if (game->enemy.active) { // if enemy is inactive (dead) don't draw 
        attron(COLOR_PAIR(COLOR_ENEMY));
        mvaddch(game->enemy.y, game->enemy.x, game->enemy.symbol);
        attroff(COLOR_PAIR(COLOR_ENEMY));
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
