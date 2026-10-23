#ifndef GAME_MANGROVE_H
#define GAME_MANGROVE_H

#include "game.h"

#define MANGROVE_PATH_W 64
#define MANGROVE_PATH_H 48
#define MANGROVE_GATE_X 32
#define MANGROVE_GATE_Y 24

extern const int mangrove_landings[4][2];
extern const int mangrove_caches[3][2];

void mangrove_generate(Map *map, unsigned int searched);
void mangrove_path(GameState *g, int returning);
void mangrove_enter(GameState *g, int portal);
void mangrove_store(GameState *g);
void mangrove_capture(const GameState *g, MangroveState *state);
void mangrove_leave(GameState *g);
int mangrove_travel(GameState *g, int x, int y);
int mangrove_interact(GameState *g);
const char *mangrove_hint(const GameState *g);
int mangrove_state_valid(const GameState *g);
void mangrove_migrate_town(GameState *g);

#endif
