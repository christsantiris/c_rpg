#ifndef PRISM_SOVEREIGN_RENDERER_H
#define PRISM_SOVEREIGN_RENDERER_H

#include "game_renderer.h"

void draw_prism_sovereign_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_prism_sovereign_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_prism_sovereign_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
