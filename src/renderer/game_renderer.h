#ifndef GAME_RENDERER_HEADER_H
#define GAME_RENDERER_HEADER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"

#define SPELL_TRAVEL_MS 240
#define SPELL_ARROW_MS 340
#define SPELL_FIREBALL_MS 460

void game_draw(Renderer *r, GameState *g, Viewport *v);

#endif
