#ifndef GRAVE_MARSHAL_RENDERER_H
#define GRAVE_MARSHAL_RENDERER_H

#include "game_renderer.h"

void draw_grave_marshal_tile(Renderer *r, int sx, int sy, TileType tile);
void draw_grave_marshal_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_grave_marshal_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
