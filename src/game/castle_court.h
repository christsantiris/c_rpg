#ifndef CASTLE_COURT_H
#define CASTLE_COURT_H

#include "game.h"

#define COURT_GATE_X 19
#define COURT_GATE_Y 43
#define COURT_LEVER_X 19
#define COURT_LEVER_Y 49

void court_generate(Map *m);
int court_interact(GameState *g, int x, int y);
int court_talk(GameState *g, int x, int y);
int court_dialogue(const GameState *g);
int court_handle_key(GameState *g, int key, int repeat);
int court_migrate(GameState *g);

#endif
