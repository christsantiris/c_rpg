#ifndef MANGROVE_RENDERER_H
#define MANGROVE_RENDERER_H
#include "game_renderer.h"
void mangrove_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void mangrove_draw_landmark(Renderer *r, const GameState *g, const Viewport *v);
void mangrove_draw_boat(Renderer *r, const GameState *g, const Viewport *v);
void mangrove_draw_warnings(Renderer *r, const GameState *g, const Viewport *v);
void mangrove_draw_hint(Renderer *r, const GameState *g);
void mangrove_draw_spice_quest(Renderer *r, const GameState *g, const Viewport *v);
#endif
