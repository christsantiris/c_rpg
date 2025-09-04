#ifndef CORE_H
#define CORE_H

#include "types.h"

// Core game initialization and cleanup
void init_game(Game *game);
void init_ncurses(void); // deprecated function
void cleanup_ncurses(void);
void start_new_game(Game *game);
void setup_enemy_by_type(Enemy* enemy, EnemyType type);
void place_stairs(Game *game);
void create_level_enemies(Game *game);

// Handle movement on map
int is_valid_move(Game *game, int new_x, int new_y);

// Handle experience
int calculate_experience_needed(int level);  
int check_level_up(Player* player);           
void gain_experience(Player* player, int exp);

// Boss logic
int is_boss_level(int level);
void create_boss_level(Game *game);

#endif