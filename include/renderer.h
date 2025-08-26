#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

// Rendering functions
void draw_map(Game *game);
void draw_player(Game *game);
void draw_enemy(Game *game, int enemy_index);

// Menu functions
void draw_title_screen(Game *game);

#endif