#ifndef CROWN_QUEST_H
#define CROWN_QUEST_H

#include "game.h"

#define EDDA_X 18
#define EDDA_Y 18
#define CROWN_INSCRIPTION_X 12
#define CROWN_INSCRIPTION_Y 14
#define CROWN_RELIC_X 5
#define CROWN_RELIC_Y 23
#define CROWN_DOOR_X 8
#define CROWN_DOOR_Y 23
#define CROWN_REWARD_GOLD 220
#define CROWN_REWARD_SCORE 1600

enum { CROWN_INSCRIPTION = 1, CROWN_SEAL = 2, CROWN_JARL = 4, CROWN_TAKEN = 8 };

void crown_quest_sync(GameState *g);
int crown_quest_talk(GameState *g);
void crown_quest_prepare(GameState *g);
void crown_quest_migrate(GameState *g);
const char *crown_quest_hint(const GameState *g);
int crown_quest_interact(GameState *g);
int crown_quest_marker(const GameState *g, int *x, int *y);
int crown_quest_valid(const GameState *g);

#endif
