#ifndef DESERT_PHARAOH_H
#define DESERT_PHARAOH_H

#include "game.h"
#include "actions.h"

typedef enum { PHARAOH_BEAM = 1, PHARAOH_BURIAL, PHARAOH_STORM } PharaohAttack;

void desert_pharaoh_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int desert_pharaoh_shadow(const GameState *g, const Enemy *e, int x, int y);
int desert_pharaoh_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int desert_pharaoh_sand_turns(TileType tile);
int desert_pharaoh_tick_floor(GameState *g);
int desert_pharaoh_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
void desert_pharaoh_cancel_landing(GameState *g, int x, int y);
void desert_pharaoh_restore_reward(GameState *g);
int desert_pharaoh_has_interaction(const GameState *g);
int desert_pharaoh_interact(GameState *g);
int desert_pharaoh_pickup(GameState *g);
void desert_pharaoh_migrate(GameState *g);

#endif
