#ifndef GAME_APPROACH_H
#define GAME_APPROACH_H

#include "game.h"

#define APPROACH_W 64
#define APPROACH_H 48
#define APPROACH_LENGTH 20
#define APPROACH_FORK_DISTANCE 8
#define APPROACH_BRANCH_LENGTH 8
#define APPROACH_MOUNTAIN_ROWS 9
#define APPROACH_LABYRINTH_X (APPROACH_W / 2 - 8)
#define APPROACH_LABYRINTH_Y (APPROACH_H / 2 - 2)
#define APPROACH_OPENING 8
#define APPROACH_SUNSCAR_ROAD 21
#define APPROACH_PEAKS_ROAD 22
#define APPROACH_PEAKS_NORTH_ROAD 23
#define APPROACH_QUARRY_ROAD 24
#define APPROACH_BLACKWATER_ROAD 25
#define APPROACH_ELDERWOOD_ROAD 26
#define APPROACH_COAST_ROAD 27
#define BRIDGE_QUARRY_X 48
#define QUARRY_LABYRINTH_X 24
#define QUARRY_LABYRINTH_Y 32
#define APPROACH_PEAKS_BRANCH_X (APPROACH_W / 2 - 12)
#define APPROACH_BRIDGE_X (APPROACH_W / 2)
#define APPROACH_BRIDGE_EXIT_Y 4

typedef struct {
    Location town;
    Location area;
    int gate_x;
    int gate_y;
    int unplaced; // No town gate leads here: kept for debug travel, or reached from another road.
} TownApproach;

extern const TownApproach town_approaches[];
extern const int town_approach_count;

const TownApproach *approach_route(const GameState *g);
const char *approach_area_name(Location area);
const char *approach_town_name(Location town);
int approach_sunscar_road(const GameState *g);
int approach_peaks_road(const GameState *g);
int approach_peaks_exit(const GameState *g, int *x, int *y);
int approach_bridge_exit(const GameState *g, int *x, int *y);
int approach_bridge_road(const GameState *g);
const char *approach_origin_name(const TownApproach *route);
TileType approach_floor(Location area);
TileType approach_scenery(Location area, int progress);
TileType approach_ground(Location area);
int approach_progress(const Map *map, int x, int y);
void approach_grow_scenery(Map *map, Location area);
void approach_migrate_scenery(GameState *g);
TileType approach_coast_ground(int x, int y);
void approach_refresh_dungeon(GameState *g);
void approach_refresh_forest(GameState *g);
void approach_retire_labyrinth(GameState *g);
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
int approach_enter_region(GameState *g, int index);
int approach_travel(GameState *g, int x, int y);
void game_leave_approach(GameState *g);
void game_leave_approach_to_town(GameState *g, Location town);

#endif
