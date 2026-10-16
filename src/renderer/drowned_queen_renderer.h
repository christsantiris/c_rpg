#ifndef DROWNED_QUEEN_RENDERER_H
#define DROWNED_QUEEN_RENDERER_H

#include "game_renderer.h"

void draw_drowned_queen_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile);
void draw_drowned_queen_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_drowned_queen_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
