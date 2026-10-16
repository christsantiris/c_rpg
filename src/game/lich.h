#ifndef LICH_H
#define LICH_H

#include "game.h"
#include "actions.h"

typedef enum {
    LICH_LANCE = 1,
    LICH_RING,
    LICH_SUMMON
} LichAttack;

void lich_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int lich_active_seals(const GameState *g);
int lich_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int lich_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int lich_enemy_damage(const GameState *g, const Enemy *e, int damage);
int lich_has_interaction(const GameState *g);
int lich_interact(GameState *g);
void lich_restore_reward(GameState *g);
void lich_migrate(GameState *g);

#endif
