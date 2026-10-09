#ifndef TOWN_LIFE_H
#define TOWN_LIFE_H

#include "game.h"

#define RESIDENT_X 20
#define RESIDENT_Y 18

typedef struct {
    Location town;
    Location interior;
    int x, y, w, h;
    int style;
    int victory;
    const char *label;
    const char *speaker;
    const char *before;
    const char *after;
} TownBuilding;

const TownBuilding *town_life_building(Location location);
int town_life_is_interior(Location location);
void town_life_place(Map *map, Location town);
void town_life_generate(Map *map, Location interior, int *sx, int *sy);
void town_life_enter(GameState *g);
void town_life_leave(GameState *g);
int town_life_talk(GameState *g, int greeting);
void town_life_migrate(GameState *g);

#endif
