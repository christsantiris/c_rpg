#ifndef MINOTAUR_RENDERER_H
#define MINOTAUR_RENDERER_H

#include "game_renderer.h"

void draw_minotaur_tile(Renderer *r, const GameState *g, int sx, int sy, TileType tile);
void draw_minotaur_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_minotaur_status(Renderer *r, const GameState *g, const Viewport *v);

#endif
