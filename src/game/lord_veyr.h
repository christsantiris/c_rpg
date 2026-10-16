#ifndef LORD_VEYR_H
#define LORD_VEYR_H

#include "game.h"

enum { VEYR_SWING = 1, VEYR_ADVANCE, VEYR_BLAST, VEYR_WEST_LANE, VEYR_EAST_LANE, VEYR_CROWN, VEYR_FIRE };
enum { VEYR_DEFERRED_FIRE = 4, VEYR_ESCORT_PENDING = 8 };

int lord_veyr_marks(const Enemy *e, int x, int y);
int lord_veyr_escort_marks(const Enemy *e, int x, int y);
int lord_veyr_barrier_change(const Enemy *e, int x, int y);
int lord_veyr_tick(GameState *g, Enemy *e);
void lord_veyr_cancel(GameState *g, Enemy *e);
void lord_veyr_disable_ward(GameState *g, int x, int y);
void lord_veyr_migrate(GameState *g);

#endif
