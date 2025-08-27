#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../core/types.h"

// Core viewport functions
void update_viewport_info(Game *game);
void center_viewport_on_player(Game *game);
void init_ncurses_with_viewport(Game *game);

// Utility functions
int world_to_screen_x(Game *game, int world_x);
int world_to_screen_y(Game *game, int world_y);
int is_in_viewport(Game *game, int world_x, int world_y);

#endif