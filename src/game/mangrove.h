#ifndef GAME_MANGROVE_H
#define GAME_MANGROVE_H

#include "game.h"

#define MANGROVE_PATH_W 64
#define MANGROVE_PATH_H 48
#define MANGROVE_GATE_X 32
#define MANGROVE_GATE_Y 24

void mangrove_generate(Map *map, unsigned int searched);
void mangrove_generate_level(Map *map, int stage, unsigned int searched);
void mangrove_landing(int stage, int index, int *x, int *y);
void mangrove_cache(int stage, int index, int *x, int *y);
void mangrove_island(int stage, int index, int *x, int *y);
const char *mangrove_stage_name(int stage);
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
void mangrove_change_level(GameState *g, int forward);
void mangrove_spawn(GameState *g, int fresh);
int mangrove_tick(GameState *g, Enemy *enemy, int *damage);
int mangrove_attack_marks(const Enemy *enemy, int x, int y);
void mangrove_defeat(GameState *g);
int mangrove_migrate_pilot(GameState *g);

#endif
