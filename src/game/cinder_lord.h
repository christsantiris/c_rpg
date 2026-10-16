#ifndef CINDER_LORD_H
#define CINDER_LORD_H

#include "game.h"
#include "actions.h"

typedef enum { CINDER_LANCE = 1, CINDER_ERUPTION, CINDER_RING } CinderAttack;
enum { CINDER_WEAPON_PENDING = 1, CINDER_POTION_PENDING = 2 };

void cinder_lord_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int cinder_lord_attack_turns(const GameState *g, const Enemy *e, int x, int y);
int cinder_lord_hot_turns(TileType tile);
int cinder_lord_cool_turns(TileType tile);
int cinder_lord_sluice_turns(TileType tile);
int cinder_lord_tick_floor(GameState *g);
int cinder_lord_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int cinder_lord_has_interaction(const GameState *g);
int cinder_lord_interact(GameState *g);
int cinder_lord_pickup(GameState *g);
void cinder_lord_restore_reward(GameState *g);
void cinder_lord_cancel_landing(GameState *g, int x, int y);
void cinder_lord_migrate(GameState *g);

#endif
