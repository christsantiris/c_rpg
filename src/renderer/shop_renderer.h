#ifndef SHOP_RENDERER_HEADER_H
#define SHOP_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/shop.h"

void shop_draw(Renderer *r, const GameState *g, const ShopScreen *s);

#endif