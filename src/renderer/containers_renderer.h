#ifndef CONTAINERS_RENDERER_H
#define CONTAINERS_RENDERER_H
#include "game_renderer.h"
void containers_draw(Renderer *r, const GameState *g, const Viewport *v);
void containers_draw_hint(Renderer *r, const GameState *g);
#endif
