#include "../include/game.h"
#include "../include/systems/save_load.h"

void draw_title_screen(Game *game) {
    clear();
    
    // ASCII Art Title - you can customize this!
    attron(COLOR_PAIR(COLOR_PLAYER) | A_BOLD);
    mvprintw(4, 28, "  ____   ___  _   _ ____  ");
    mvprintw(5, 28, " / ___| / _ \\| \\ | |  _ \\ ");
    mvprintw(6, 28, "| |    | | | |  \\| | |_) |");
    mvprintw(7, 28, "| |___ | |_| | |\\  |  _ < ");
    mvprintw(8, 28, " \\____| \\___/|_| \\_|_| \\_\\");
    attroff(COLOR_PAIR(COLOR_PLAYER) | A_BOLD);
    
    // Subtitle
    attron(COLOR_PAIR(COLOR_TEXT));
    mvprintw(10, 30, "~ Castle of no Return ~");
    attroff(COLOR_PAIR(COLOR_TEXT));
    
    // Check if save file exists
    int has_save = save_file_exists(SAVE_FILENAME);
    
    // Menu options
    int start_y = 13;
    const char* menu_items[] = {
        "New Game",
        has_save ? "Load Game (save found)" : "Load Game (no save)", 
        "Quit"
    };
    
    for (int i = 0; i < 3; i++) {
        if (i == game->selected_menu) {
            // Highlight selected option
            attron(COLOR_PAIR(COLOR_ENEMY) | A_BOLD);
            mvprintw(start_y + i * 2, 30, "> %s <", menu_items[i]);
            attroff(COLOR_PAIR(COLOR_ENEMY) | A_BOLD);
        } else {
            // Normal option - but dim the load option if no save exists
            if (i == MENU_LOAD_GAME && !has_save) {
                attron(COLOR_PAIR(COLOR_WALL)); // Dimmed color
            } else {
                attron(COLOR_PAIR(COLOR_TEXT));
            }
            mvprintw(start_y + i * 2, 32, "%s", menu_items[i]);
            attroff(COLOR_PAIR(COLOR_TEXT));
            attroff(COLOR_PAIR(COLOR_WALL));
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
            // Move up in menu
            game->selected_menu--;
            if (game->selected_menu < 0) {
                game->selected_menu = 2; // Wrap to bottom (Quit)
            }
            break;
            
        case KEY_DOWN:
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
                    if (save_file_exists(SAVE_FILENAME)) {
                        if (load_game(game, SAVE_FILENAME)) {
                            // Game loaded successfully
                            // The load_game function sets game_state to STATE_PLAYING
                        } else {
                            // Show error message
                            clear();
                            attron(COLOR_PAIR(COLOR_TEXT));
                            mvprintw(MAP_HEIGHT / 2, MAP_WIDTH / 2 - 15, "Error: Could not load saved game!");
                            mvprintw(MAP_HEIGHT / 2 + 2, MAP_WIDTH / 2 - 8, "Press any key to continue...");
                            attroff(COLOR_PAIR(COLOR_TEXT));
                            refresh();
                            getch();
                        }
                    } else {
                        // No save file exists
                        clear();
                        attron(COLOR_PAIR(COLOR_TEXT));
                        mvprintw(MAP_HEIGHT / 2, MAP_WIDTH / 2 - 10, "No saved game found!");
                        mvprintw(MAP_HEIGHT / 2 + 2, MAP_WIDTH / 2 - 8, "Press any key to continue...");
                        attroff(COLOR_PAIR(COLOR_TEXT));
                        refresh();
                        getch();
                    }
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