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
            
            // Draw all enemies
            for (int i = 0; i < game.enemy_count; i++) {
                draw_enemy(&game, i);
            }
            
            if (!game.game_over) {
                attron(COLOR_PAIR(COLOR_TEXT));
                mvprintw(MAP_HEIGHT + 2, 0, "Use arrow keys to move, 'q' to quit");
                mvprintw(MAP_HEIGHT + 3, 0, "Turn: %d | Enemies Killed: %d", 
                         game.turn_count, game.enemies_killed);
                
                // Check if all enemies are defeated
                int active_enemies = 0;
                for (int i = 0; i < game.enemy_count; i++) {
                    if (game.enemies[i].active) {
                        active_enemies++;
                    }
                }
                
                if (active_enemies == 0) {
                    attron(A_BOLD);
                    mvprintw(MAP_HEIGHT + 4, 0, "Victory! You cleared the dungeon!");
                    attroff(A_BOLD);
                }
                attroff(COLOR_PAIR(COLOR_TEXT));
            }
            
            refresh();
            running = handle_input(&game);
            
            if (game.game_over) {
                // Game over screen
                clear();
                mvprintw(5, 15, "  ####    ###   #   #  ####       ####  #   #  ####  ####");
                mvprintw(6, 15, " #       #   #  ## ##  #          #  #  #   #  #     #   #");
                mvprintw(7, 15, " # ###   #####  # # #  ###        #  #  #   #  ###   ####");
                mvprintw(8, 15, " #   #   #   #  #   #  #          #  #   # #   #     #  #");
                mvprintw(9, 15, "  ####   #   #  #   #  ####       ####    #    ####  #   #");
                
                mvprintw(12, 25, "You have been defeated!");
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

    // Initialize enemy array and count
    game->enemy_count = 0;
    
    // Initialize turn counters
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

        // Create multiple enemies (3-5 enemies)
        int num_enemies = random_range(3, 5);
        for (int i = 0; i < num_enemies && i < MAX_ENEMIES; i++) {
            game->enemies[i].ID = next_id++;
            game->enemies[i].symbol = ENEMY;
            game->enemies[i].active = 1;  // Enemy starts alive
            strcpy(game->enemies[i].name, "Goblin");
            
            // Place enemy randomly in any room, but not on player or other enemies
            int placement_attempts = 0;
            int placed = 0;
            
            while (!placed && placement_attempts < 50) {
                // Pick a random room
                int room_index = random_range(0, game->room_count - 1);
                Rectangle room = game->rooms[room_index];
                
                // Generate random position within room
                int test_x = random_range(room.x + 1, room.x + room.width - 2);
                int test_y = random_range(room.y + 1, room.y + room.height - 2);
                
                // Check if position is free (not on player or other enemies)
                int position_free = 1;
                
                // Check against player
                if (test_x == game->player.x && test_y == game->player.y) {
                    position_free = 0;
                }
                
                // Check against other enemies
                for (int j = 0; j < i; j++) {
                    if (game->enemies[j].x == test_x && game->enemies[j].y == test_y) {
                        position_free = 0;
                        break;
                    }
                }
                
                if (position_free) {
                    game->enemies[i].x = test_x;
                    game->enemies[i].y = test_y;
                    placed = 1;
                }
                
                placement_attempts++;
            }
            
            // If we couldn't place the enemy after many attempts, skip it
            if (placed) {
                game->enemy_count++;
            }
        }
    } else {
        // Fallback if no rooms generated
        game->player.x = MAP_WIDTH / 2;
        game->player.y = MAP_HEIGHT / 2;
        
        // Still try to create at least one enemy near the player
        game->enemies[0].ID = next_id++;
        game->enemies[0].symbol = ENEMY;
        game->enemies[0].active = 1;
        game->enemies[0].x = game->player.x + 2;
        game->enemies[0].y = game->player.y + 2;
        strcpy(game->enemies[0].name, "Goblin");
        game->enemy_count = 1;
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