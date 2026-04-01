#ifndef INVENTORY_RENDERER_HEADER_H
#define INVENTORY_RENDERER_HEADER_H

#include "renderer.h"
#include "../game/game.h"
#include "../screens/inventory.h"

void inventory_draw(Renderer *r, const GameState *g, const InventoryScreen *s);

#endif