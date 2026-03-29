#ifndef INFO_PANEL_HEADER_H
#define INFO_PANEL_HEADER_H

#include "renderer.h"
#include "../game/game.h"

#define INFO_PANEL_W 161

void info_panel_draw(Renderer *r, const GameState *g);

#endif