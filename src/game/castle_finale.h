#ifndef CASTLE_FINALE_H
#define CASTLE_FINALE_H
#include "game.h"

void finale_generate(Map *m);
int finale_interact(GameState *g, int x, int y);
int finale_migrate(GameState *g);
void finale_sync(GameState *g);
int finale_pending(const GameState *g);
int finale_handle_key(GameState *g, int key, int repeat);
#endif
