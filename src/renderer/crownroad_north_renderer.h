#ifndef CROWNROAD_NORTH_RENDERER_H
#define CROWNROAD_NORTH_RENDERER_H
#include "game_renderer.h"
void northroad_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void northroad_draw_details(Renderer *r, const GameState *g, const Viewport *v);
void northroad_draw_hint(Renderer *r, const GameState *g);
#endif
