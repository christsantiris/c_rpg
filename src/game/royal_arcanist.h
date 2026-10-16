#ifndef ROYAL_ARCANIST_H
#define ROYAL_ARCANIST_H

#include "game.h"

enum { ARCANIST_BLAST = 1, ARCANIST_CROSS, ARCANIST_WEST_LANE, ARCANIST_EAST_LANE };

int royal_arcanist_marks(const Enemy *e, int x, int y);
int royal_arcanist_barrier_change(const Enemy *e, int x, int y);
int royal_arcanist_tick(GameState *g, Enemy *e);
void royal_arcanist_disable_ward(GameState *g, int x, int y);
void royal_arcanist_cancel(Enemy *e);
void royal_arcanist_migrate(GameState *g);

#endif
