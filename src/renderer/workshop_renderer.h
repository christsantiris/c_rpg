#ifndef WORKSHOP_RENDERER_HEADER_H
#define WORKSHOP_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/workshop.h"

void workshop_draw(Renderer *r, const GameState *g, const WorkshopScreen *s);

#endif
