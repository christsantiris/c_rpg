#include "../include/game.h"
#include "../include/systems/viewport.h"
#include "../include/systems/renderer.h"
#include "../include/systems/double_buffer.h"
#include "../include/systems/save_load.h"

int main() {
    Game game;
    
    // Initialize ncurses with viewport support
    init_ncurses_with_viewport(&game);
    
    // Initialize double buffer
    init_double_buffer(&game.double_buffer);
    
    // Initialize game to menu state
    game.game_state = STATE_MENU;
    game.selected_menu = MENU_NEW_GAME; // Start with "New Game" selected
    game.game_over = 0;
    game.showMessage = 0;
    game.showLevelMessage = 0;     // Initialize new message field
    strcpy(game.levelMessage, ""); // Initialize level message buffer
    
    srand(time(NULL));

    // Main game loop
    int running = 1;
    while (running && !game.game_over) {
        
        if (game.game_state == STATE_MENU) {
            // Show title screen (not double buffered for simplicity)
            draw_title_screen(&game);
            handle_menu_input(&game);
            
        } else if (game.game_state == STATE_PLAYING) {
            // START DOUBLE BUFFER FRAME
            begin_draw(&game.double_buffer);
            
            // Update viewport if needed (only when player gets close to edges)
            center_viewport_on_player(&game);
            
            // Draw everything to back buffer
            draw_map(&game);
            draw_player(&game);
            
            // Draw all enemies
            for (int i = 0; i < game.enemy_count; i++) {
                draw_enemy(&game, i);
            }
            
            if (!game.game_over) {
                // Draw UI elements
                draw_ui_text(&game, game.viewport.viewport_height + 2, 0, 
                           "Arrow keys/WASD: move, 'V': inventory, 'U': use, 'S': save, 'L': load, 'Q': quit");
                
                // Create status line string
                char status_line[256];
                snprintf(status_line, sizeof(status_line),
                        "Lv: %d | XP: %d/%d | Turn: %d | Killed: %d", 
                        game.player.level, game.player.experience, 
                        game.player.experience_to_next, game.turn_count, game.enemies_killed);

                // Draw colored HP first
                WINDOW *win = get_draw_window(&game);
                int hp_color = get_hp_color(game.player.current_hp, game.player.max_hp);
                wattron(win, COLOR_PAIR(hp_color) | A_BOLD);
                mvwprintw(win, game.viewport.viewport_height + 3, 0, "HP: %d/%d", 
                        game.player.current_hp, game.player.max_hp);
                wattroff(win, COLOR_PAIR(hp_color) | A_BOLD);

                // Then draw rest with normal function
                draw_ui_text(&game, game.viewport.viewport_height + 3, 15, status_line);
                
                // Handle combat message
                if (game.showMessage && strlen(game.recentlyDefeated) > 0) {
                    char combat_msg[64];
                    snprintf(combat_msg, sizeof(combat_msg), "You killed %s", game.recentlyDefeated);
                    draw_ui_text(&game, game.viewport.viewport_height + 4, 0, combat_msg);
                }
                
                // Handle level message (shows on line 5 if combat message is showing, otherwise line 4)
                if (game.showLevelMessage) {
                    int line = game.showMessage ? 5 : 4;
                    // Use bold for level messages
                    WINDOW *win = get_draw_window(&game);
                    wattron(win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
                    mvwprintw(win, game.viewport.viewport_height + line, 0, "%s", game.levelMessage);
                    wattroff(win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
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

                // Level completion message (takes priority over other messages)
                if (game.waiting_for_stairs) {
                    char stairs_msg[128];
                    snprintf(stairs_msg, sizeof(stairs_msg), 
                           "Dungeon Level %d cleared! Find the stairs '>' to descend!", 
                           game.current_level);
                    WINDOW *win = get_draw_window(&game);
                    wattron(win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
                    mvwprintw(win, game.viewport.viewport_height + 4, 0, "%s", stairs_msg);
                    wattroff(win, COLOR_PAIR(COLOR_TEXT) | A_BOLD);
                }
            }
            
            // PRESENT THE COMPLETE FRAME (no flicker!)
            end_draw_and_present(&game.double_buffer);
            
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
    
    cleanup_double_buffer(&game.double_buffer);
    cleanup_ncurses();
    printf("Thanks for playing!\n");
    return 0;
}

int handle_input(Game *game) {
    int ch = getch(); // Get a character from user
    
    switch (ch) {
        case 's': // Both lowercase and uppercase 's' for save
        case 'S':
            // Save the game
            clear();
            mvprintw(5, 15, "=== SAVE GAME ===");
            mvprintw(7, 15, "Saving game to: %s", SAVE_FILENAME);
            mvprintw(8, 15, "Creating saves directory...");
            refresh();
            
            // Create directory first
            create_save_directory();
            
            mvprintw(9, 15, "Attempting to save...");
            refresh();
            
            if (save_game(game, SAVE_FILENAME)) {
                mvprintw(11, 15, "Game saved successfully!");
            } else {
                mvprintw(11, 15, "Error: Could not save game!");
            }
            
            mvprintw(13, 15, "Check the saves/ directory for savegame.dat");
            mvprintw(15, 15, "Press any key to continue...");
            refresh();
            getch();
            break;
            
        case KEY_UP:
            move_player(game, 0, -1); // Move up (y decreases)
            break;
            
        case KEY_DOWN:
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

        case 'v':
        case 'V':
            show_inventory(game);
            break;

        case 'u':
        case 'U':
            // Use item from inventory
            if (game->player.inventory.item_count > 0) {
                // Show available consumable items
                clear();
                mvprintw(2, 2, "=== USE ITEM ===");
                mvprintw(4, 2, "Select an item to use:");
                
                int consumable_count = 0;
                int consumable_indices[MAX_INVENTORY_SIZE];
                
                // Show only consumable items
                for (int i = 0; i < game->player.inventory.item_count; i++) {
                    Item* item = &game->player.inventory.items[i];
                    if (item->type == ITEM_TYPE_CONSUMABLE) {
                        mvprintw(6 + consumable_count, 4, "%d. %s", 
                                consumable_count + 1, item->name);
                        consumable_indices[consumable_count] = i;
                        consumable_count++;
                    }
                }
                
                if (consumable_count == 0) {
                    mvprintw(6, 4, "No usable items in inventory.");
                    mvprintw(8, 2, "Press any key to continue...");
                    refresh();
                    getch();
                } else {
                    mvprintw(6 + consumable_count + 1, 2, "Press 1-%d to use item, any other key to cancel", consumable_count);
                    refresh();
                    
                    int item_choice = getch();
                    int choice_num = item_choice - '1';  // Convert '1' to 0, '2' to 1, etc.
                    
                    if (choice_num >= 0 && choice_num < consumable_count) {
                        int actual_index = consumable_indices[choice_num];
                        Item* selected_item = &game->player.inventory.items[actual_index];
                        
                        // Clear screen and show result
                        clear();
                        mvprintw(2, 2, "=== ITEM USED ===");
                        
                        // Check if item can be used
                        if (selected_item->type == ITEM_TYPE_CONSUMABLE && selected_item->use_function) {
                            if (game->player.current_hp >= game->player.max_hp) {
                                mvprintw(4, 2, "You are already at full health!");
                            } else {
                                // Apply healing
                                int old_hp = game->player.current_hp;
                                game->player.current_hp += selected_item->heal_amount;
                                if (game->player.current_hp > game->player.max_hp) {
                                    game->player.current_hp = game->player.max_hp;
                                }
                                
                                int healed = game->player.current_hp - old_hp;
                                
                                // Show result with proper formatting
                                mvprintw(4, 2, "You drink the %s and restore %d HP!", 
                                        selected_item->name, healed);
                                mvprintw(5, 2, "HP: %d/%d", 
                                        game->player.current_hp, game->player.max_hp);
                                mvprintw(6, 2, "The %s is consumed.", selected_item->name);
                                
                                // Remove item from inventory
                                remove_item_from_inventory(&game->player.inventory, actual_index);
                                
                                // This counts as a turn - move all active enemies
                                for (int i = 0; i < game->enemy_count; i++) {
                                    if (game->enemies[i].active) {
                                        move_enemy(game, i);
                                    }
                                }
                                game->turn_count++;
                            }
                        } else {
                            mvprintw(4, 2, "This item cannot be used.");
                        }
                        
                        mvprintw(8, 2, "Press any key to continue...");
                        refresh();
                        getch();
                    }
                }
                
                // Clear messages when returning to game
                game->showMessage = 0;
            } else {
                // No items in inventory
                clear();
                mvprintw(5, 15, "Your inventory is empty!");
                mvprintw(7, 15, "Press any key to continue...");
                refresh();
                getch();
            }
            break;
            
        case 'q':
        case 'Q':
        case KEY_ESC:
            // Check if custom quit key is configured
            if (ch == game->config.quit_key || ch == 'q' || ch == 'Q' || ch == KEY_ESC) {
                // Clear screen and show quit confirmation
                clear();
                mvprintw(5, 15, "=== QUIT GAME ===");
                mvprintw(7, 15, "Are you sure you want to quit?");
                mvprintw(9, 15, "Press 'Y' to quit, any other key to continue");
                refresh();
                
                // Get confirmation input
                int confirm = getch();
                if (confirm == 'y' || confirm == 'Y') {
                    return 0; // Quit game
                }
                // For 'n', 'N', or any other key, continue game
                // Screen will be redrawn automatically on next frame
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