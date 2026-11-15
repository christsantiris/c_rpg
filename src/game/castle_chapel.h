#ifndef CASTLE_CHAPEL_H
#define CASTLE_CHAPEL_H

#include "game.h"

#define CHAPEL_GATE_X 26
#define CHAPEL_GATE_Y 39
#define CHAPEL_LEVER_X 27
#define CHAPEL_LEVER_Y 44

void chapel_generate(Map *m);
int chapel_interact(GameState *g, int x, int y);
int chapel_migrate(GameState *g);

#endif
