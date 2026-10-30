#ifndef WILDS_RENDERER_H
#define WILDS_RENDERER_H

#include "renderer.h"
#include "../game/game.h"

void draw_boulder(Renderer *r, int sx, int sy);
void draw_wilds_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);

#endif
