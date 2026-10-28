#ifndef COAST_EXITS_H
#define COAST_EXITS_H

#include "game.h"

int coast_exit_point(const Map *map, int forward, int *x, int *y);
int coast_exits_valid(const Map *map);
int coast_exits_repair_map(Map *map, int level);
void coast_exits_repair(GameState *g);
void coast_exits_migrate(GameState *g);

#endif
