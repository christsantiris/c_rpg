#ifndef SWAMP_DEMON_H
#define SWAMP_DEMON_H

#include "game.h"
#include "actions.h"

typedef enum { DEMON_CLEAVE = 1, DEMON_POOL, DEMON_FAN } DemonAttack;

void swamp_demon_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int swamp_demon_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int swamp_demon_rot_turns(TileType tile);
int swamp_demon_vent_turns(TileType tile);
int swamp_demon_tick_floor(GameState *g);
int swamp_demon_tick(GameState *g, Enemy *e);
int swamp_demon_has_interaction(const GameState *g);
int swamp_demon_interact(GameState *g);
void swamp_demon_restore_reward(GameState *g);
void swamp_demon_migrate(GameState *g);

#endif
