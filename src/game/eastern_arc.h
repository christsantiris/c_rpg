#ifndef EASTERN_ARC_H
#define EASTERN_ARC_H

#include "game.h"

#define DAWNMERE_SQUAD 5
#define DAWNMERE_GARRISON (DAWNMERE_SQUAD * 3)
#define DAWNMERE_RUSK (DAWNMERE_SQUAD * 2)
#define DAWNMERE_SEREN_X 24
#define DAWNMERE_SEREN_Y 18
#define RIVER_SOUTH_X 14
#define RIVER_SOUTH_Y 32
#define RIVER_NORTH_X 46
#define RIVER_NORTH_Y 7

int eastern_town_combat(const GameState *g);
void eastern_init(GameState *g);
void eastern_migrate(GameState *g);
void eastern_reinforce(GameState *g);
void eastern_store(GameState *g);
void eastern_enter_town(GameState *g);
void eastern_refresh(GameState *g);
void eastern_update(GameState *g);
int eastern_move(GameState *g, int x, int y);
int eastern_talk(GameState *g);
int eastern_handle_key(GameState *g, int key, int repeat);
void eastern_river_map(Map *map);
void eastern_river_enter(GameState *g, int from_town);
int eastern_river_travel(GameState *g, int x, int y);
void eastern_river_docked(GameState *g);
int eastern_valid(const GameState *g);

#endif
