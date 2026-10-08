#ifndef JAIL_RENDERER_H
#define JAIL_RENDERER_H

#include "game_renderer.h"

void jail_draw_person(Renderer *r, int sx, int sy, int prisoner);
void jail_draw_building(Renderer *r, const Viewport *v);
void jail_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);

#endif
