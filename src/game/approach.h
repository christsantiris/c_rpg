#ifndef GAME_APPROACH_H
#define GAME_APPROACH_H

#include "game.h"

#define APPROACH_W 64
#define APPROACH_H 48
#define APPROACH_LENGTH 20
#define APPROACH_FORK_DISTANCE 8
#define APPROACH_BRANCH_LENGTH 8
#define APPROACH_LABYRINTH_X (APPROACH_W / 2 - 8)
#define APPROACH_LABYRINTH_Y (APPROACH_H / 2 - 2)

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
TileType approach_coast_ground(int x, int y);
void approach_refresh_dungeon(GameState *g);
void approach_refresh_forest(GameState *g);
int approach_forest_town_exit(const GameState *g, int *x, int *y);
void approach_refresh_mountains(GameState *g);
void approach_refresh_swamp(GameState *g);
int approach_swamp_town_exit(const GameState *g, int *x, int *y);
int approach_entrance_rows(Location area);
void approach_refresh_entrance(GameState *g);
int approach_mountain_town_exit(const GameState *g, int *x, int *y);
void approach_relocate_ridgeshire(GameState *g);
int approach_dungeon_town_exit(const GameState *g, int *x, int *y);
int approach_shortcut_exit(const GameState *g, int *x, int *y);
int approach_boss_exit(const GameState *g, int *x, int *y);
Location approach_shortcut_town(const TownApproach *route);
void approach_refresh_fork(GameState *g);
void approach_return_from_shortcut(GameState *g, Location area);
void approach_return_from_boss(GameState *g, Location area);
void approach_migrate_roads(GameState *g);
int approach_enter(GameState *g, int x, int y);
int approach_travel(GameState *g, int x, int y);
void game_leave_approach(GameState *g);
void game_leave_approach_to_town(GameState *g, Location town);

#endif
