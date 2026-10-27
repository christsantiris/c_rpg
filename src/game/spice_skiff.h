#ifndef SPICE_SKIFF_H
#define SPICE_SKIFF_H

#include "game.h"

#define SPICE_WRECK_X 22
#define SPICE_WRECK_Y 11
#define SPICE_CASKET_X 11
#define SPICE_CASKET_Y 26
#define SPICE_REWARD_GOLD 150
#define SPICE_REWARD_SCORE 1000

enum { SPICE_MANIFEST = 1, SPICE_CARGO = 2 };

extern const char spice_skiff_offer[];

void spice_skiff_talk(GameState *g);
void spice_skiff_accept(GameState *g);
const char *spice_skiff_hint(const GameState *g);
int spice_skiff_interact(GameState *g);
int spice_skiff_marker(const GameState *g, int *x, int *y);

#endif
