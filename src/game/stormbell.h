#ifndef STORMBELL_H
#define STORMBELL_H

#include "game.h"

#define SEREN_X 34
#define SEREN_Y 10
#define STORMBELL_CLAPPER_X 55
#define STORMBELL_CLAPPER_Y 7
#define STORMBELL_X 36
#define STORMBELL_Y 23
#define STORMBELL_REWARD_GOLD 180
#define STORMBELL_REWARD_SCORE 1200

enum { STORMBELL_CLAPPER = 1, STORMBELL_FITTED = 2, STORMBELL_SECURE = 4 };

int stormbell_talk(GameState *g);
void stormbell_accept(GameState *g);
const char *stormbell_hint(const GameState *g);
int stormbell_interact(GameState *g);
int stormbell_marker(const GameState *g, int *x, int *y);
int stormbell_prompt_active(const GameState *g);
int stormbell_handle_key(GameState *g, int key, int repeat);
void stormbell_capture(const GameState *g, Enemy guards[3]);
void stormbell_restore(GameState *g);
void stormbell_turn(GameState *g);
void stormbell_migrate(GameState *g);
int stormbell_valid(const GameState *g);

#endif
