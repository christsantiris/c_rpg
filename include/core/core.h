#ifndef CORE_H
#define CORE_H

#include "types.h"

// Core game initialization and cleanup
void init_game(Game *game);
void init_ncurses(void);
void cleanup_ncurses(void);
void start_new_game(Game *game);
void setup_enemy_by_type(Enemy* enemy, EnemyType type);
void place_stairs(Game *game);
void create_level_enemies(Game *game);

// Handle movement on map
int is_valid_move(Game *game, int new_x, int new_y);

#endif