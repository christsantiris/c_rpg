#ifndef ELOWEN_H
#define ELOWEN_H

#include "game.h"

#define ELOWEN_REWARD_GOLD 80

enum {
    ELOWEN_MISSING,
    ELOWEN_ASSIGNED,
    ELOWEN_RESCUED,
    ELOWEN_HOME
};

int elowen_at(const GameState *g, int x, int y);
void elowen_prepare(GameState *g);
void elowen_rescue(GameState *g);
void elowen_follow(GameState *g, int old_x, int old_y);
void elowen_home(GameState *g);

#endif
