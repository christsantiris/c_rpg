#ifndef GAMBLER_RENDERER_HEADER_H
#define GAMBLER_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/gambler.h"

void gambler_draw(Renderer *r, const GameState *g, GamblerScreen *s);
SDL_Rect gambler_button_rect(const Renderer *r, int option);

#endif
