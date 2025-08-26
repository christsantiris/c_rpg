#ifndef CORE_H
#define CORE_H

#include "types.h"

// Core game initialization and cleanup
void init_game(Game *game);
void init_ncurses(void);
void cleanup_ncurses(void);
void start_new_game(Game *game);

// Handle movement on map
int is_valid_move(Game *game, int new_x, int new_y);

#endif