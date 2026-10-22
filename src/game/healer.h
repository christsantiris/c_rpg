#ifndef HEALER_H
#define HEALER_H

#include "game.h"

#define HEALER_X 20
#define HEALER_Y 17
#define HEALER_HP_PER_GOLD 10

void healer_enter(GameState *g);
void healer_leave(GameState *g);
int healer_talk(GameState *g);
int healer_offer(const GameState *g, int choice);
int healer_purchase(GameState *g, int choice);
int healer_handle_key(GameState *g, int scancode, int repeat);

#endif
