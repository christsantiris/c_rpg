#ifndef RED_DRAGON_H
#define RED_DRAGON_H

#include "game.h"
#include "actions.h"

enum { DRAGON_BREATH = 1, DRAGON_SWEEP, DRAGON_WINGBEAT, DRAGON_INFERNO_FIRST, DRAGON_INFERNO_SECOND };

void red_dragon_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int red_dragon_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int red_dragon_hot_turns(TileType tile);
TileType red_dragon_base_tile(TileType tile);
int red_dragon_tick_floor(GameState *g);
int red_dragon_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
void red_dragon_award_reward(GameState *g, int x, int y);
void red_dragon_restore_reward(GameState *g);
int red_dragon_has_interaction(const GameState *g);
int red_dragon_interact(GameState *g, int pickup);
void red_dragon_cancel_landing(GameState *g, int x, int y);
void red_dragon_migrate(GameState *g);

#endif
