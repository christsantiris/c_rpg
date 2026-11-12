#ifndef ACT5_ROUTES_H
#define ACT5_ROUTES_H

#include "game.h"

void act5_shape_entry(Map *map, Location area);
void act5_reveal_exits(GameState *g);
void act5_routes_migrate(GameState *g);

#endif
