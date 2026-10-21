#ifndef INFO_PANEL_HEADER_H
#define INFO_PANEL_HEADER_H

#include "renderer.h"
#include "viewport.h"
#include "../game/game.h"

#define INFO_PANEL_W 200

void info_panel_draw(Renderer *r, const GameState *g, const Viewport *v);
void info_panel_draw_harbor(Renderer *r, const GameState *g);

#endif
