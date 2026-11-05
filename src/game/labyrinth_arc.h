#ifndef LABYRINTH_ARC_H
#define LABYRINTH_ARC_H

#include "game.h"

void labyrinth_prepare_road(GameState *g);
void labyrinth_simplify_map(Map *map, int level);
void labyrinth_complete_quest(GameState *g);
void labyrinth_migrate(GameState *g, int version);

#endif
