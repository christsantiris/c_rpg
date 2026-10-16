#ifndef GOBLIN_KING_RENDERER_H
#define GOBLIN_KING_RENDERER_H

#include "game_renderer.h"

void draw_goblin_king_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile);
void draw_goblin_king_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_goblin_king_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
