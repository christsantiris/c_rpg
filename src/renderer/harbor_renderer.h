#ifndef HARBOR_RENDERER_HEADER_H
#define HARBOR_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/harbor.h"

void harbor_draw(Renderer *r, const GameState *g, const HarborScreen *s);
SDL_Rect harbor_button_rect(const Renderer *r, int option);

#endif
