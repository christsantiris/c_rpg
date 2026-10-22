#ifndef TOWN_ROAD_HEADER_H
#define TOWN_ROAD_HEADER_H

#include "approach.h"

#define TOWN_ROAD_W 64
#define TOWN_ROAD_H 48
#define TOWN_SPRITE_X 24
#define TOWN_SPRITE_Y 17
#define TOWN_SPRITE_W 17
#define TOWN_SPRITE_H 15
#define TOWN_ROAD_COUNT 5

typedef enum {
    TOWN_SIDE_NORTH,
    TOWN_SIDE_EAST,
    TOWN_SIDE_SOUTH,
    TOWN_SIDE_WEST
} TownSide;

int town_road_index(Location town);
int town_road_connection(Location town, TownSide side, TownApproach *connection);
TownSide town_road_side(int x, int y);
void town_road_entrance(TownSide side, int *x, int *y);
void town_road_exit(TownSide side, int *x, int *y);
int town_road_enter(GameState *g, Location town, TownSide side, int departing);
int town_road_depart(GameState *g, int x, int y);
int town_road_travel(GameState *g, int x, int y);

#endif
