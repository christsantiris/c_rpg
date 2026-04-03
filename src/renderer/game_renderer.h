#ifndef GAME_RENDERER_HEADER_H
#define GAME_RENDERER_HEADER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"

void game_draw(Renderer *r, GameState *g, Viewport *v);

#endif