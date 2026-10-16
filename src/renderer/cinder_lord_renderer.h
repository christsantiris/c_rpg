#ifndef CINDER_LORD_RENDERER_H
#define CINDER_LORD_RENDERER_H

#include "game_renderer.h"

void draw_cinder_lord_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_cinder_lord_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_cinder_lord_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
