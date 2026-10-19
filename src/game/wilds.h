#ifndef WILDS_H
#define WILDS_H

#include "game.h"

int wilds_tile(TileType tile);
int wilds_has_interaction(const GameState *g);
int wilds_interact(GameState *g);
int wilds_conceals(const GameState *g, const Enemy *e);
void wilds_migrate(GameState *g);

#endif
