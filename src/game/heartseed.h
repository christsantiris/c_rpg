#ifndef HEARTSEED_H
#define HEARTSEED_H

#include "game.h"

#define TAMSIN_X 17
#define TAMSIN_Y 4
#define HEARTSEED_ORDERS_X 9
#define HEARTSEED_ORDERS_Y 9
#define HEARTSEED_X 53
#define HEARTSEED_Y 33
#define HEARTSEED_REWARD_GOLD 120
#define HEARTSEED_REWARD_SCORE 800

enum { HEARTSEED_ORDERS = 1, HEARTSEED_BINDINGS = 2, HEARTSEED_TAKEN = 4 };
enum { HEARTSEED_UNPLACED, HEARTSEED_BOUND, HEARTSEED_SLAIN, HEARTSEED_FREED };

extern const int heartseed_stakes[3][2];

void heartseed_prepare(GameState *g);
void heartseed_migrate(GameState *g);
int heartseed_talk(GameState *g);
void heartseed_accept(GameState *g);
const char *heartseed_hint(const GameState *g);
int heartseed_interact(GameState *g);
int heartseed_is_guardian(const GameState *g, const Enemy *e);
void heartseed_record_death(GameState *g, const Enemy *e);

#endif
