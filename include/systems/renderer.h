#ifndef RENDERER_H
#define RENDERER_H

#include "../core/types.h"

// Rendering functions
void draw_map(Game *game);
void draw_player(Game *game);
void draw_enemy(Game *game, int enemy_index);
void place_stairs(Game *game);
void draw_ui_text(Game *game, int y, int x, const char *text);
WINDOW* get_draw_window(Game *game);

// Menu functions
void draw_title_screen(Game *game);

// Inventory view
void show_inventory(Game *game);

#endif