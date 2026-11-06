#ifndef COASTAL_TRAVEL_H
#define COASTAL_TRAVEL_H
#include "approach.h"

// Former coastal dock coordinates retained for legacy-save fixtures.
#define COAST_DOCK_X 50
#define COAST_DOCK_Y 22
#define COAST_DOCK_ENTRY_X 52
#define COAST_DOCK_ENTRY_Y 24
#define COAST_ROWAN_X 48
#define COAST_ROWAN_Y 20

#define STILLBURY_DOCK_X 51
#define STILLBURY_DOCK_Y 4
#define STILLBURY_DOCK_ENTRY_X 53
#define STILLBURY_DOCK_ENTRY_Y 6
#define STILLBURY_ROWAN_X 49
#define STILLBURY_ROWAN_Y 4

int coastal_approach(const GameState *g);
int coastal_stillbury_docks(const GameState *g);
int coastal_at_docks(const GameState *g);
int coastal_trade_restored(const GameState *g);
void coastal_place_docks(Map *map);
void coastal_refresh_docks(GameState *g);
void coastal_return_from_island(GameState *g);
void coastal_migrate_travel(GameState *g);
void coastal_migrate_quest(GameState *g, int version);
#endif
