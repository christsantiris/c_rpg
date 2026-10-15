#ifndef THORN_REGENT_RENDERER_H
#define THORN_REGENT_RENDERER_H

#include "game_renderer.h"

void draw_thorn_regent_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_thorn_regent_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_thorn_regent_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
