#ifndef STONEWAKE_H
#define STONEWAKE_H

#include "game.h"

int stonewake_map(const Map *map);
int stonewake_crane_tile(TileType tile);
int stonewake_crane_mode(TileType tile);
int stonewake_has_salvage(const GameState *g, int x, int y);
void stonewake_migrate_cranes(GameState *g);
void stonewake_generate(Map *map, int level);
int stonewake_has_interaction(const GameState *g);
int stonewake_interact(GameState *g);
int stonewake_colossus_tick(GameState *g, Enemy *enemy);
int stonewake_attack_marks(const GameState *g, const Enemy *enemy, int x, int y);
void stonewake_migrate(GameState *g);
void stonewake_begin_expedition(GameState *g);
const char *stonewake_stage_name(int level);
int stonewake_claim_warhammer(GameState *g);

#endif
