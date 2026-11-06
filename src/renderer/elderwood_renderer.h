#ifndef ELDERWOOD_RENDERER_H
#define ELDERWOOD_RENDERER_H
#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"
void draw_elderwood_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile);
void draw_northern_road(Renderer *r, const GameState *g, const Viewport *v);
void draw_elderwood_warnings(Renderer *r, const GameState *g, const Viewport *v);
#endif
