#include "../include/game.h"

int main() {
    Game game;
    
    // Initialize ncurses
    init_ncurses();
    
    // Initialize game to menu state
    game.game_state = STATE_MENU;
    game.selected_menu = MENU_NEW_GAME; // Start with "New Game" selected
    game.game_over = 0;
    game.showMessage = 0;
    
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
                mvprintw(MAP_HEIGHT + 3, 0, "HP: %d/%d | Level: %d | Turn: %d | Killed: %d", 
                        game.player.current_hp, game.player.max_hp,
                        game.current_level,                   
                        game.turn_count, game.enemies_killed);
                
                if (game.showMessage) {
                    mvprintw(MAP_HEIGHT + 4, 0, "You killed %s", 
                         game.recentlyDefeated);
                }
                
                // Check if all enemies are defeated
                int active_enemies = 0;
                for (int i = 0; i < game.enemy_count; i++) {
                    if (game.enemies[i].active) {
                        active_enemies++;
                    }
                }
                
                if (active_enemies == 0 && !game.waiting_for_stairs) {
                    game.waiting_for_stairs = 1;  // Set flag to show stairs message
                    place_stairs(&game);          // Place the stairs
                }

                if (game.waiting_for_stairs) {
                    attron(A_BOLD);
                    mvprintw(MAP_HEIGHT + 4, 0, "Level %d cleared! Find the stairs '>' to descend!", game.current_level);
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
            // Check if custom quit key is configured
            if (ch == game->config.quit_key || ch == 'q' || ch == 'Q' || ch == KEY_ESC) {
                // Display quit confirmation prompt
                attron(COLOR_PAIR(COLOR_TEXT));
                mvprintw(MAP_HEIGHT + 4, 0, "Are you sure? (Y/N)");
                attroff(COLOR_PAIR(COLOR_TEXT));
                refresh(); // Show prompt immediately
                
                // Get confirmation input
                int confirm = getch();
                if (confirm == 'y' || confirm == 'Y') {
                    return 0; // Quit game
                }
                // For 'n', 'N', or any other key, continue game
            }
            break;
            
        default:
            // Invalid key, do nothing
            break;
    }
    
    // Update all enemies after player moves (if player actually moved)
    // This ensures enemies only move when the player moves (turn-based)
    if (ch == KEY_UP || ch == 'w' || ch == 'W' ||
        ch == KEY_DOWN || ch == 's' || ch == 'S' ||
        ch == KEY_LEFT || ch == 'a' || ch == 'A' ||
        ch == KEY_RIGHT || ch == 'd' || ch == 'D' ||
        ch == '1' || ch == '0' || ch == 'z' || ch == 'm') {
        
        // Move all active enemies
        for (int i = 0; i < game->enemy_count; i++) {
            if (game->enemies[i].active) {
                move_enemy(game, i);
            }
        }
    }
    
    return 1; // Continue game
}