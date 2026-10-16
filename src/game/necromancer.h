#ifndef NECROMANCER_H
#define NECROMANCER_H

#include "game.h"
#include "actions.h"

typedef enum {
    NECRO_BOLT = 1,
    NECRO_ROOTS,
    NECRO_RAISE
} NecromancerAttack;

void necromancer_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int necromancer_active_graves(const GameState *g);
int necromancer_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int necromancer_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int necromancer_has_interaction(const GameState *g);
int necromancer_interact(GameState *g);
void necromancer_restore_reward(GameState *g);
void necromancer_migrate(GameState *g);

#endif
