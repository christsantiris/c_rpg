#ifndef ELDERWOOD_HEADER_H
#define ELDERWOOD_HEADER_H

#include "game.h"

#define NORTH_ROAD_X 32
#define NORTH_ELDER_X 40
#define NORTH_ELDER_Y 32
#define NORTH_CAT_X 24
#define NORTH_CAT_Y 15
#define NORTH_FOREST_BARRIER_Y 30

enum { ELDER_CHARGE = 1, ELDER_ROOTS, ELDER_SUMMON };

int northern_approach(const GameState *g);
void northern_refresh(GameState *g);
int northern_travel(GameState *g, int x, int y);
void northern_return(GameState *g, Location town, int catacombs);
void elderwood_enter(GameState *g);
void elderwood_store(GameState *g);
void elderwood_unpack(const ElderwoodCache *cache, Map *map);
void elderwood_pack(ElderwoodCache *cache, const Map *map);
int elderwood_travel(GameState *g, int x, int y);
void elderwood_change_level(GameState *g, int descending);
void elderwood_leave(GameState *g);
int elderwood_interact(GameState *g);
int elderwood_tick(GameState *g, Enemy *enemy, int *damage);
int elderwood_marks(const Enemy *enemy, int x, int y);
void elderwood_defeat(GameState *g);
void elderwood_portal(GameState *g);
void elderwood_hide_portal(GameState *g);

#endif
