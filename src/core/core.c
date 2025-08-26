#include "../include/game.h"

// Move these functions from src/main.c:
void init_ncurses(void) {
    // Copy the entire function from main.c here
    initscr();
    
    if (has_colors()) {
        start_color();
        init_pair(COLOR_WALL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_FLOOR, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_PLAYER, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_ENEMY, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_TEXT, COLOR_CYAN, COLOR_BLACK);
    }
    
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

void cleanup_ncurses(void) {
    // Copy from main.c
    endwin();
}

void init_game(Game *game) {
    // Load any saved config
    config_load_from_file(&game->config, "game.cfg");
    
    config_save_to_file(&game->config, "game.cfg");

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
            snprintf(game->enemies[i].name, sizeof(game->enemies[i].name), "Goblin %d", i + 1);
            
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

// Handle movement on map
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