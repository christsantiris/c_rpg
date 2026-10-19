#ifndef REGIONAL_TERRAIN_H
#define REGIONAL_TERRAIN_H

#include "game.h"

int regional_terrain_tile(TileType tile);
int regional_terrain_heat(TileType tile);
int regional_terrain_has_interaction(const GameState *g);
int regional_terrain_interact(GameState *g);
int regional_terrain_tick(GameState *g);
void regional_terrain_cancel_landing(GameState *g, int x, int y);
void regional_terrain_migrate(GameState *g);
int regional_terrain_light(const GameState *g, int x, int y);

#endif
