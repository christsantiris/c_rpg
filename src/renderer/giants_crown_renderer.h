#ifndef GIANTS_CROWN_RENDERER_H
#define GIANTS_CROWN_RENDERER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"

void giants_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile);
void giants_draw_details(Renderer *r, const GameState *g, const Viewport *v);
void giants_draw_hint(Renderer *r, const GameState *g);
void giants_draw_enemy(Renderer *r, int sx, int sy, EnemyType type);

#endif
