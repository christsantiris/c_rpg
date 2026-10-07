#ifndef CASTLE_RENDERER_H
#define CASTLE_RENDERER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/castle.h"

void castle_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void castle_draw_enemy(Renderer *r, int sx, int sy, const Enemy *e);
void castle_draw_warnings(Renderer *r, const GameState *g, const Viewport *v);

#endif
