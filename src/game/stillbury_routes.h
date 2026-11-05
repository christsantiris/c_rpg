#ifndef GAME_STILLBURY_ROUTES_H
#define GAME_STILLBURY_ROUTES_H

#include "game.h"

void stillbury_routes_prepare(GameState *g);
int stillbury_routes_travel(GameState *g, int x, int y);
void stillbury_routes_migrate(GameState *g, int version);

#endif
