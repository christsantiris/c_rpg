#ifndef STORMWATCH_RENDERER_H
#define STORMWATCH_RENDERER_H

#include "renderer.h"
#include "../game/game.h"
#include "viewport.h"

void stormwatch_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void stormwatch_draw_warnings(Renderer *r, const GameState *g, const Viewport *v);
void stormbell_draw_details(Renderer *r, const GameState *g, const Viewport *v);
void stormwatch_draw_hint(Renderer *r, const GameState *g);

#endif
