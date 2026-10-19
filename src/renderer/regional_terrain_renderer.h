#ifndef REGIONAL_TERRAIN_RENDERER_H
#define REGIONAL_TERRAIN_RENDERER_H

#include "game_renderer.h"

void draw_regional_terrain_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void draw_regional_terrain_light(Renderer *r, const GameState *g, const Viewport *v);
void draw_regional_terrain_status(Renderer *r, const GameState *g);

#endif
