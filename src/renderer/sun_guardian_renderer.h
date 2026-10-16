#ifndef SUN_GUARDIAN_RENDERER_H
#define SUN_GUARDIAN_RENDERER_H

#include "game_renderer.h"

void draw_sun_guardian_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile);
void draw_sun_guardian_warnings(Renderer *r, const GameState *g, const Viewport *v);
void draw_sun_guardian_status(Renderer *r, const GameState *g, const Viewport *v);
int sun_guardian_compact_status(const Renderer *r, const GameState *g, const Viewport *v);

#endif
