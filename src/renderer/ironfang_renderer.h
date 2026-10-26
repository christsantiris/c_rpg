#ifndef IRONFANG_RENDERER_H
#define IRONFANG_RENDERER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"

void ironfang_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void ironfang_draw_details(Renderer *r, const GameState *g, const Viewport *v);
void ironfang_draw_hint(Renderer *r, const GameState *g);

#endif
