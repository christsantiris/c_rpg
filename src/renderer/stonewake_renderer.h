#ifndef STONEWAKE_RENDERER_H
#define STONEWAKE_RENDERER_H
#include "game_renderer.h"

void draw_stonewake_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_stonewake_props(Renderer *r, const GameState *g, const Viewport *v);
void draw_stonewake_machinery(Renderer *r, int sx, int sy, int repaired);
int draw_stonewake_enemy(Renderer *r, int sx, int sy, EnemyType type);
#endif
