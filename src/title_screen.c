#include "../include/game.h"

void draw_title_screen(Game *game) {
    clear();
    
    // ASCII Art Title - you can customize this!
    attron(COLOR_PAIR(COLOR_PLAYER) | A_BOLD);
    mvprintw(4, 20, "  ____   ___  _   _ ____  ");
    mvprintw(5, 20, " / ___| / _ \\| \\ | |  _ \\ ");
    mvprintw(6, 20, "| |    | | | |  \\| | |_) |");
    mvprintw(7, 20, "| |___ | |_| | |\\  |  _ < ");
    mvprintw(8, 20, " \\____| \\___/|_| \\_|_| \\_\\");
    attroff(COLOR_PAIR(COLOR_PLAYER) | A_BOLD);
    
    // Subtitle
    attron(COLOR_PAIR(COLOR_TEXT));
    mvprintw(10, 18, "~ Castle of no Return ~");
    attroff(COLOR_PAIR(COLOR_TEXT));
    
    // Menu options
    int start_y = 13;
    const char* menu_items[] = {
        "New Game",
        "Load Game", 
        "Quit"
    };
    
    for (int i = 0; i < 3; i++) {
        if (i == game->selected_menu) {
            // Highlight selected option
            attron(COLOR_PAIR(COLOR_ENEMY) | A_BOLD);
            mvprintw(start_y + i * 2, 30, "> %s <", menu_items[i]);
            attroff(COLOR_PAIR(COLOR_ENEMY) | A_BOLD);
        } else {
            // Normal option
            attron(COLOR_PAIR(COLOR_TEXT));
            mvprintw(start_y + i * 2, 32, "%s", menu_items[i]);
            attroff(COLOR_PAIR(COLOR_TEXT));
        }
    }
    
    // Instructions
    attron(COLOR_PAIR(COLOR_FLOOR));
    mvprintw(22, 20, "Use UP/DOWN arrows to navigate, ENTER to select");
    attroff(COLOR_PAIR(COLOR_FLOOR));
    
    refresh();
}

void handle_menu_input(Game *game) {
    int ch = getch();
    
    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
            // Move up in menu
            game->selected_menu--;
            if (game->selected_menu < 0) {
                game->selected_menu = 2; // Wrap to bottom (Quit)
            }
            break;
            
        case KEY_DOWN:
        case 's':
        case 'S':
            // Move down in menu
            game->selected_menu++;
            if (game->selected_menu > 2) {
                game->selected_menu = 0; // Wrap to top (New Game)
            }
            break;
            
        case 10: // ENTER key
        case 13: // Alternative ENTER
        case ' ': // Spacebar as alternative
            // Handle menu selection
            switch (game->selected_menu) {
                case MENU_NEW_GAME:
                    start_new_game(game);
                    break;
                    
                case MENU_LOAD_GAME:
                    // Show "not implemented" message temporarily
                    clear();
                    attron(COLOR_PAIR(COLOR_TEXT));
                    mvprintw(MAP_HEIGHT / 2, MAP_WIDTH / 2 - 10, "Load Game - Not Implemented Yet!");
                    mvprintw(MAP_HEIGHT / 2 + 2, MAP_WIDTH / 2 - 8, "Press any key to continue...");
                    attroff(COLOR_PAIR(COLOR_TEXT));
                    refresh();
                    getch(); // Wait for key press
                    break;
                    
                case MENU_QUIT:
                    // Set game to quit
                    game->game_over = 1;
                    break;
            }
            break;
            
        case 'q':
        case 'Q':
        case KEY_ESC:
            // Quick quit
            game->game_over = 1;
            break;
    }
}

void start_new_game(Game *game) {
    // Random number generator
    srand(time(NULL));

    // Simple unique IDs for enemies
    static int next_id = 1;
    
    // Initialize player
    game->player.symbol = PLAYER;

    // Initialize enemy array and count
    game->enemy_count = 0;
    
    // Initialize game state
    game->game_over = 0;
    game->game_state = STATE_PLAYING; // Switch to playing state
    game->turn_count = 0;
    game->enemies_killed = 0;

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