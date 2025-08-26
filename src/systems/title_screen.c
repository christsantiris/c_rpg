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