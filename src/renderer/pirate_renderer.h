#ifndef PIRATE_RENDERER_H
#define PIRATE_RENDERER_H
#include "renderer.h"
#include "viewport.h"
#include "../game/pirate.h"
void draw_pirate_tile(Renderer *r, const Map *map, int sx, int sy, int x, int y, TileType tile);
void draw_pirate_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_pirate_status(Renderer *r, const GameState *g, const Viewport *v);
#endif
