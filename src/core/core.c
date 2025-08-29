#include "../../include/core/core.h"
#include "../../include/utils/config.h"
#include "../../include/core/dungeon.h"
#include "../../include/core/item.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>

// init_ncurses is deprecated for init_ncurses_with_viewport
void init_ncurses(void) {
    // Initialize ncurses
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
    endwin();
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
    
    // Check if player is at this position
    if (game->player.x == new_x && game->player.y == new_y) {
        return 0; // Can't move into player's space
    }
    
    // Check if any active enemy is at this position
    for (int i = 0; i < game->enemy_count; i++) {
        if (game->enemies[i].active && 
            game->enemies[i].x == new_x && 
            game->enemies[i].y == new_y) {
            return 0; // Can't move into enemy's space
        }
    }
    
    return 1; // Valid move
}

void init_game(Game *game) {
    // Load configuration first
    config_load_from_file(&game->config, "assets/game.cfg");
    config_save_to_file(&game->config, "assets/game.cfg");
    
    game->current_level = 1;        
    game->waiting_for_stairs = 0;

    // Random number generator
    srand(time(NULL));

    // Simple unique IDs for enemies
    static int next_id = 1;
    
    // Initialize player
    game->player.symbol = PLAYER;
    game->player.level = 1;                                       
    game->player.experience = 0;                                
    game->player.experience_to_next = calculate_experience_needed(1);

    // Set base stats
    game->player.base_attack = 10;
    game->player.base_defense = 2;
    game->player.max_hp = 100;
    game->player.current_hp = 100;

    // Initialize calculated stats to base values first
    game->player.attack = game->player.base_attack;   // Start with base attack
    game->player.defense = game->player.base_defense; // Start with base defense

    // Give player starting weapon and recalculate stats
    Item starting_weapon = create_weapon("Rusty Sword", 2);
    equip_weapon(&game->player, starting_weapon);

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
            game->enemies[i].active = 1;  // Enemy starts alive
            
            // Level 1: Only weak enemies for level 1
            EnemyType enemy_type;
            int type_roll = rand() % 100;
            if (type_roll < 80) {
                enemy_type = ENEMY_GOBLIN;    // 80% goblins
            } else {
                enemy_type = ENEMY_SKELETON;  // 20% skeletons
            }
            setup_enemy_by_type(&game->enemies[i], enemy_type);
            
            // Add unique number to name
            char temp_name[32];
            strcpy(temp_name, game->enemies[i].name);
            snprintf(game->enemies[i].name, sizeof(game->enemies[i].name), "%s %d", temp_name, i + 1);
            
            // Rest of placement code stays the same...
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
        game->enemies[0].active = 1;
        game->enemies[0].x = game->player.x + 2;
        game->enemies[0].y = game->player.y + 2;
        strcpy(game->enemies[0].name, "Goblin");
        
        // Initialize enemy combat stats
        game->enemies[0].max_hp = 20;
        game->enemies[0].current_hp = 20;
        game->enemies[0].attack = 5;
        game->enemies[0].defense = 1;
        
        game->enemy_count = 1;
    }
}

void start_new_game(Game *game) {
    // Set game state first
    game->game_state = STATE_PLAYING;
    
    // Use existing initialization logic
    init_game(game);
}

void setup_enemy_by_type(Enemy* enemy, EnemyType type) {
    enemy->type = type;
    
    switch (type) {
        case ENEMY_GOBLIN:
            enemy->symbol = 'G';
            strcpy(enemy->name, "Goblin");
            enemy->max_hp = 15;
            enemy->current_hp = 15;
            enemy->attack = 4;
            enemy->defense = 1;
            enemy->experience = 10;
            break;
            
        case ENEMY_ORC:
            enemy->symbol = 'O';
            strcpy(enemy->name, "Orc");
            enemy->max_hp = 25;
            enemy->current_hp = 25;
            enemy->attack = 7;
            enemy->defense = 2;
            enemy->experience = 20;
            break;
            
        case ENEMY_SKELETON:
            enemy->symbol = 'S';
            strcpy(enemy->name, "Skeleton");
            enemy->max_hp = 20;
            enemy->current_hp = 20;
            enemy->attack = 6;
            enemy->defense = 0;  // No armor, but harder to hit
            enemy->experience = 15;
            break;
            
        case ENEMY_TROLL:
            enemy->symbol = 'T';
            strcpy(enemy->name, "Troll");
            enemy->max_hp = 40;
            enemy->current_hp = 40;
            enemy->attack = 10;
            enemy->defense = 4;
            enemy->experience = 30;
            break;

        case ENEMY_DRAGON:
            enemy->symbol = 'D';
            strcpy(enemy->name, "Ancient Dragon");
            enemy->max_hp = 120;
            enemy->current_hp = 120;
            enemy->attack = 18;
            enemy->defense = 8;
            enemy->experience = 200;
            break;
            
        case ENEMY_DEMON_LORD:
            enemy->symbol = 'L';
            strcpy(enemy->name, "Demon Lord");
            enemy->max_hp = 180;
            enemy->current_hp = 180;
            enemy->attack = 25;
            enemy->defense = 12;
            enemy->experience = 400;
            break;
            
        case ENEMY_LICH_KING:
            enemy->symbol = 'K';
            strcpy(enemy->name, "Lich King");
            enemy->max_hp = 250;
            enemy->current_hp = 250;
            enemy->attack = 35;
            enemy->defense = 15;
            enemy->experience = 600;
            break;
            
        default:
            // Fallback to goblin
            enemy->symbol = 'G';
            strcpy(enemy->name, "Goblin");
            enemy->max_hp = 15;
            enemy->current_hp = 15;
            enemy->attack = 4;
            enemy->defense = 1;
            break;
    }
}

void create_level_enemies(Game *game) {

    // If boss level don't create enemies and only create the boss
    if (is_boss_level(game->current_level)) {
        create_boss_level(game);
        return; // Remove this return to give the boss minions
    }
    // Clear existing enemies
    game->enemy_count = 0;
    
    // More enemies on deeper levels
    int base_enemies = 3;
    int bonus_enemies = (game->current_level - 1) / 2; // +1 enemy every 2 levels
    int num_enemies = base_enemies + bonus_enemies;
    
    if (num_enemies > MAX_ENEMIES) {
        num_enemies = MAX_ENEMIES;
    }
    
    static int next_id = 100; // Different from initial enemies
    
    for (int i = 0; i < num_enemies; i++) {
        game->enemies[i].ID = next_id++;
        game->enemies[i].active = 1;
        
        // Higher level = stronger enemies more likely
        EnemyType enemy_type;
        int type_roll = rand() % 100;
        
        if (game->current_level == 1) {
            // Level 1: Mostly goblins
            if (type_roll < 70) enemy_type = ENEMY_GOBLIN;
            else if (type_roll < 90) enemy_type = ENEMY_SKELETON;
            else enemy_type = ENEMY_ORC;
        } else if (game->current_level <= 3) {
            // Levels 2-3: Mix of weak and medium
            if (type_roll < 40) enemy_type = ENEMY_GOBLIN;
            else if (type_roll < 70) enemy_type = ENEMY_SKELETON;
            else if (type_roll < 95) enemy_type = ENEMY_ORC;
            else enemy_type = ENEMY_TROLL;
        } else {
            // Level 4+: Mostly strong enemies
            if (type_roll < 20) enemy_type = ENEMY_GOBLIN;
            else if (type_roll < 40) enemy_type = ENEMY_SKELETON;
            else if (type_roll < 70) enemy_type = ENEMY_ORC;
            else enemy_type = ENEMY_TROLL;
        }
        
        setup_enemy_by_type(&game->enemies[i], enemy_type);
        
        // Add level number to name
        char temp_name[32];
        strcpy(temp_name, game->enemies[i].name);
        snprintf(game->enemies[i].name, sizeof(game->enemies[i].name), 
                 "%s %d", temp_name, i + 1);
        
        // Place enemy in random room (same placement logic as before)
        int placement_attempts = 0;
        int placed = 0;
        
        while (!placed && placement_attempts < 50) {
            int room_index = random_range(0, game->room_count - 1);
            Rectangle room = game->rooms[room_index];
            
            int test_x = random_range(room.x + 1, room.x + room.width - 2);
            int test_y = random_range(room.y + 1, room.y + room.height - 2);
            
            // Check if position is free
            int position_free = 1;
            
            if (test_x == game->player.x && test_y == game->player.y) {
                position_free = 0;
            }
            
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
        
        if (placed) {
            game->enemy_count++;
        }
    }
}

// Check if current level is a boss level (every 5th level)
int is_boss_level(int level) {
    return (level % 5 == 0);
}

// Create a special boss level with one powerful enemy
void create_boss_level(Game *game) {
    // Clear existing enemies
    game->enemy_count = 0;
    
    static int next_id = 500; // Different ID range for bosses
    
    // Create exactly one boss enemy
    game->enemies[0].ID = next_id++;
    game->enemies[0].active = 1;
    
    // Determine boss type based on level
    EnemyType boss_type;
    if (game->current_level <= 10) {
        boss_type = ENEMY_DRAGON;
    } else if (game->current_level <= 20) {
        boss_type = ENEMY_DEMON_LORD;
    } else {
        boss_type = ENEMY_LICH_KING;
    }
    
    setup_enemy_by_type(&game->enemies[0], boss_type);
    
    // Add level indicator to name
    char temp_name[32];
    strcpy(temp_name, game->enemies[0].name);
    snprintf(game->enemies[0].name, sizeof(game->enemies[0].name), 
             "%s (Lv.%d Boss)", temp_name, game->current_level);
    
    // Place boss in the center of the largest room
    if (game->room_count > 0) {
        // Find the largest room
        int largest_room = 0;
        int max_area = game->rooms[0].width * game->rooms[0].height;
        
        for (int i = 1; i < game->room_count; i++) {
            int area = game->rooms[i].width * game->rooms[i].height;
            if (area > max_area) {
                max_area = area;
                largest_room = i;
            }
        }
        
        Rectangle boss_room = game->rooms[largest_room];
        game->enemies[0].x = boss_room.x + boss_room.width / 2;
        game->enemies[0].y = boss_room.y + boss_room.height / 2;
        
        game->enemy_count = 1;
    }
}
