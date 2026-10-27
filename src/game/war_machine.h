#ifndef WAR_MACHINE_H
#define WAR_MACHINE_H

#include "game.h"

#define WAR_ORDERS_X 18
#define WAR_ORDERS_Y 12
#define WAR_WINCH_X 61
#define WAR_WINCH_Y 24
#define WAR_CARGO_X 62
#define WAR_CARGO_Y 23
#define WAR_SEAL_X 53
#define WAR_SEAL_Y 13
#define WAR_REWARD_GOLD 160
#define WAR_REWARD_SCORE 1000
#define WAR_SPEAKER "Marshal Veyra: Ironfang"

enum { WAR_ORDERS = 1, WAR_SABOTAGE = 2, WAR_SEAL = 4 };

int war_machine_talk(GameState *g);
void war_machine_accept(GameState *g);
int war_machine_dialogue(const GameState *g);
int war_machine_handle_key(GameState *g, int key, int repeat);
const char *war_machine_hint(const GameState *g);
int war_machine_interact(GameState *g);
int war_machine_marker(const GameState *g, int *x, int *y);
int war_machine_valid(const GameState *g);

#endif
