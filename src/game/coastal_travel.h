#ifndef COASTAL_TRAVEL_H
#define COASTAL_TRAVEL_H
#include "approach.h"

#define COAST_DOCK_X 50
#define COAST_DOCK_Y 22
#define COAST_DOCK_ENTRY_X 52
#define COAST_DOCK_ENTRY_Y 24
#define COAST_ROWAN_X 48
#define COAST_ROWAN_Y 20

int coastal_approach(const GameState *g);
int coastal_at_docks(const GameState *g);
void coastal_refresh_docks(GameState *g);
void coastal_return_from_island(GameState *g);
void coastal_migrate_travel(GameState *g);
#endif
