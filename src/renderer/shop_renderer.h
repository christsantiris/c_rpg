#ifndef SHOP_RENDERER_HEADER_H
#define SHOP_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/shop.h"

void shop_draw(Renderer *r, const GameState *g, const ShopScreen *s);
SDL_Rect shop_healer_button_rect(const Renderer *r, int option);

#endif
