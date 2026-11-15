#ifndef CASTLE_APARTMENTS_H
#define CASTLE_APARTMENTS_H

#include "game.h"

#define APARTMENTS_GATE_Y 39
#define APARTMENTS_LEVER_X 27
#define APARTMENTS_LEVER_Y 46

void apartments_generate(Map *m);
void apartments_supply(GameState *g);
int apartments_interact(GameState *g, int x, int y);
int apartments_migrate(GameState *g);

#endif
