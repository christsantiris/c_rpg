#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

// Player movement and validation functions
int is_valid_move(Game *game, int new_x, int new_y);
void move_player(Game *game, int dx, int dy);

#endif