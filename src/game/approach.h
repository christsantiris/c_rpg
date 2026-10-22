#ifndef GAME_APPROACH_H
#define GAME_APPROACH_H

#include "game.h"

#define APPROACH_W 64
#define APPROACH_H 48
#define APPROACH_LENGTH 20

typedef struct {
    Location town;
    Location area;
    int gate_x;
    int gate_y;
} TownApproach;

extern const TownApproach town_approaches[];
extern const int town_approach_count;

const TownApproach *approach_route(const GameState *g);
const char *approach_area_name(Location area);
const char *approach_town_name(Location town);
TileType approach_floor(Location area);
void approach_refresh_dungeon(GameState *g);
int approach_enter(GameState *g, int x, int y);
int approach_travel(GameState *g, int x, int y);
void game_leave_approach(GameState *g);

#endif
