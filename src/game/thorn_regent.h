#ifndef THORN_REGENT_H
#define THORN_REGENT_H

#include "game.h"
#include "actions.h"

typedef enum { REGENT_LANCE = 1, REGENT_ROOTS, REGENT_REGROW } RegentAttack;
enum { REGENT_CLOAK_PENDING = 1, REGENT_POTION_PENDING = 2 };

void thorn_regent_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int thorn_regent_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int thorn_regent_root_turns(TileType tile);
int thorn_regent_wall_turns(TileType tile);
int thorn_regent_node_turns(TileType tile);
int thorn_regent_tick_floor(GameState *g);
int thorn_regent_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int thorn_regent_has_interaction(const GameState *g);
int thorn_regent_interact(GameState *g);
int thorn_regent_pickup(GameState *g);
void thorn_regent_restore_reward(GameState *g);
void thorn_regent_cancel_landing(GameState *g, int x, int y);
void thorn_regent_migrate(GameState *g);

#endif
