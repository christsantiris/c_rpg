#ifndef SWAMP_DEMON_RENDERER_H
#define SWAMP_DEMON_RENDERER_H

#include "game_renderer.h"

void draw_swamp_demon_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile);
void draw_swamp_demon_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_swamp_demon_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
