#ifndef MESSAGE_BAR_HEADER_H
#define MESSAGE_BAR_HEADER_H

#include "renderer.h"
#include "../game/game.h"

#define MESSAGE_BAR_H 36

void message_bar_draw(Renderer *r, const GameState *g);

#endif