#ifndef HEARTSEED_H
#define HEARTSEED_H

#include "game.h"

#define TAMSIN_X 17
#define TAMSIN_Y 4
#define HEARTSEED_ORDERS_X 9
#define HEARTSEED_ORDERS_Y 9
#define HEARTSEED_X 53
#define HEARTSEED_Y 33

enum { HEARTSEED_ORDERS = 1, HEARTSEED_BINDINGS = 2, HEARTSEED_TAKEN = 4 };
enum { HEARTSEED_UNPLACED, HEARTSEED_BOUND, HEARTSEED_SLAIN, HEARTSEED_FREED };

// Retired quest definitions above remain for loading existing saves.
void heartseed_carve_grove(Map *map);
void heartseed_migrate(GameState *g);

#endif
