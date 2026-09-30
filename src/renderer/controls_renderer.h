#ifndef CONTROLS_RENDERER_HEADER_H
#define CONTROLS_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/controls_screen.h"

const char *controls_key_name(int scancode);
void controls_draw(Renderer *r, const GameState *g, const ControlsScreen *s);

#endif
