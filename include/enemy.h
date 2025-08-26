#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"

// Enemy functions
int is_valid_enemy_move(Game *game, int new_x, int new_y);
void move_enemy(Game *game, int enemy_index);

#endif