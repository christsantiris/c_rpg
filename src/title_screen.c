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
    // Initialize player
    game->player.symbol = PLAYER;

    // Initialize enemy
    game->enemy.symbol = ENEMY;
    game->enemy.active = 1;
    
    // Initialize game state
    game->game_over = 0;
    game->game_state = STATE_PLAYING; // Switch to playing state
    game->turn_count = 0;
    game->enemies_killed = 0;

    // Initialize room count
    game->room_count = 0;
    
    // Generate the dungeon
    generate_dungeon(game);
    
    // Place player and enemy (your existing placement code)
    if (game->room_count > 0) {
        game->player.x = game->rooms[0].x + game->rooms[0].width / 2;
        game->player.y = game->rooms[0].y + game->rooms[0].height / 2;

        do {
            game->enemy.x = random_range(game->rooms[0].x + 1, game->rooms[0].x + game->rooms[0].width - 2);
            game->enemy.y = random_range(game->rooms[0].y + 1, game->rooms[0].y + game->rooms[0].height - 2);
        } while (game->enemy.x == game->player.x && game->enemy.y == game->player.y);
        
        strcpy(game->enemy.name, "Goblin");
    } else {
        game->player.x = MAP_WIDTH / 2;
        game->player.y = MAP_HEIGHT / 2;
    }
}