#ifndef GAME_PIRATE_H
#define GAME_PIRATE_H

#include "game.h"

enum { PIRATE_SHOT = 1, PIRATE_BLADE_WAVE = 3, PIRATE_SABER };

#define PIRATE_QUEST_LEVEL 3
#define PIRATE_QUEST_X 10
#define PIRATE_QUEST_Y 6
#define PIRATE_QUEST_GOLD 100

const char *pirate_stage_name(int level);
void pirate_generate(Map *map, int level, unsigned int seed, unsigned int claimed);
void pirate_edge(int level, int forward, int *x, int *y);
void pirate_chest_position(int level, int index, int *x, int *y);
void pirate_reachable(const Map *map, unsigned char cells[PIRATE_H][PIRATE_W]);
void pirate_migrate(GameState *g, int version);
void pirate_refresh_treasure(GameState *g);
void pirate_talk_merchant(GameState *g);
void pirate_enter(GameState *g);
void pirate_store(GameState *g);
void pirate_capture(const GameState *g, PirateCache *cache);
void pirate_leave(GameState *g);
void pirate_change_level(GameState *g, int descending);
int pirate_travel(GameState *g, int x, int y);
int pirate_interact(GameState *g);
int pirate_near_chest(const GameState *g);
int pirate_tick(GameState *g, Enemy *enemy, int *damage);
int pirate_marks(const GameState *g, const Enemy *enemy, int x, int y);
void pirate_defeat(GameState *g);
void pirate_portal(GameState *g);

#endif
