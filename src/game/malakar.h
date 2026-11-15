#ifndef MALAKAR_H
#define MALAKAR_H

#include "game.h"

enum { MALAKAR_VIAL = 1, MALAKAR_PRESSURE, MALAKAR_RESTORE };

void malakar_generate(Map *m);
void malakar_init(Enemy *e, int x, int y);
void malakar_prepare(GameState *g);
int malakar_marks(const Enemy *e, int x, int y);
int malakar_tick(GameState *g, Enemy *e);
int malakar_damage(GameState *g, Enemy *e, int damage);
int malakar_interact(GameState *g, int x, int y);
int malakar_migrate(GameState *g);

#endif
