#ifndef GOBLIN_KING_H
#define GOBLIN_KING_H

#include "game.h"
#include "actions.h"

typedef enum {
    GOBLIN_KING_AXE = 1,
    GOBLIN_KING_BOMBS
} GoblinKingAttack;

void goblin_king_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int goblin_king_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int goblin_king_keg_fuse(const GameState *g, int x, int y);
int goblin_king_keg_marks(const GameState *g, int x, int y);
int goblin_king_remaining_kegs(const GameState *g);
int goblin_king_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int goblin_king_tick_kegs(GameState *g);
int goblin_king_has_interaction(const GameState *g);
int goblin_king_interact(GameState *g);
void goblin_king_restore_reward(GameState *g);
void goblin_king_migrate(GameState *g);

#endif
