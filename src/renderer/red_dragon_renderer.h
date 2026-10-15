#ifndef RED_DRAGON_RENDERER_H
#define RED_DRAGON_RENDERER_H

#include "game_renderer.h"

void draw_red_dragon_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_red_dragon_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_red_dragon_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
