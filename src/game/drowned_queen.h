#ifndef DROWNED_QUEEN_H
#define DROWNED_QUEEN_H

#include "game.h"
#include "actions.h"

typedef enum {
    QUEEN_ROW = 1,
    QUEEN_COLUMN,
    QUEEN_POCKETS
} QueenAttack;

void drowned_queen_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int drowned_queen_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int drowned_queen_controls(const GameState *g);
int drowned_queen_tick(GameState *g, Enemy *e);
int drowned_queen_has_interaction(const GameState *g);
int drowned_queen_interact(GameState *g);
void drowned_queen_restore_reward(GameState *g);
void drowned_queen_migrate(GameState *g);

#endif
