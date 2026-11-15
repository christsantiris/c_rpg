#ifndef MALAKAR_FINAL_H
#define MALAKAR_FINAL_H

#include "game.h"

enum { FINAL_SWING = 1, FINAL_ADVANCE, FINAL_BLAST, FINAL_WEST_LANE, FINAL_EAST_LANE, FINAL_CROWN, FINAL_FIRE };
enum { FINAL_DEFERRED_FIRE = 4, FINAL_ESCORT_PENDING = 8 };

int malakar_final_marks(const Enemy *e, int x, int y);
int malakar_final_escort_marks(const Enemy *e, int x, int y);
int malakar_final_barrier_change(const Enemy *e, int x, int y);
int malakar_final_tick(GameState *g, Enemy *e);
void malakar_final_cancel(GameState *g, Enemy *e);
void malakar_final_disable_ward(GameState *g, int x, int y);
void malakar_final_migrate(GameState *g);

#endif
