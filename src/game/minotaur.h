#ifndef MINOTAUR_H
#define MINOTAUR_H

#include "game.h"
#include "actions.h"

typedef enum { MINOTAUR_CHARGE = 1, MINOTAUR_AXE, MINOTAUR_WIDE_AXE } MinotaurAttack;
typedef enum { MINOTAUR_REWARD_NONE, MINOTAUR_REWARD_AEGIS, MINOTAUR_REWARD_CLAIMED, MINOTAUR_REWARD_LEGACY } MinotaurReward;

void minotaur_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int minotaur_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int minotaur_tick(GameState *g, Enemy *e);
void minotaur_restore_reward(GameState *g);
int minotaur_has_interaction(const GameState *g);
int minotaur_interact(GameState *g);
int minotaur_pickup(GameState *g);
void minotaur_record_pickup(GameState *g, const Item *item);
void minotaur_migrate(GameState *g);

#endif
