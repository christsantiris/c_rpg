#ifndef EQUIPMENT_COMPARE_RENDERER_HEADER_H
#define EQUIPMENT_COMPARE_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"

void draw_weapon_comparison(Renderer *r, const GameState *g, const Item *selected, const Item *equipped, int y);
void draw_armor_comparison(Renderer *r, const GameState *g, const Item *selected, const Item *equipped, int y);

#endif
