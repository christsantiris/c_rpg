#ifndef RESTORATION_H
#define RESTORATION_H

#include "game.h"

#define RESTORATION_X 20
#define RESTORATION_Y 17
#define RESTORATION_PER_GOLD 10

void restoration_enter(GameState *g, Location location);
void restoration_leave(GameState *g);
int restoration_talk(GameState *g);
int restoration_offer(const GameState *g, int choice);
int restoration_purchase(GameState *g, int choice);
int restoration_prompt_active(const GameState *g);
int restoration_handle_key(GameState *g, int scancode, int repeat);

#endif
