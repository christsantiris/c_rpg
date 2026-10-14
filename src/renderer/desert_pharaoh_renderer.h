#ifndef DESERT_PHARAOH_RENDERER_H
#define DESERT_PHARAOH_RENDERER_H

#include "game_renderer.h"

void draw_desert_pharaoh_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_desert_pharaoh_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_desert_pharaoh_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
