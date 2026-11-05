#ifndef WESTERN_ARC_H
#define WESTERN_ARC_H

#include "game.h"

#define ZARA_OAKHAVEN_X 10
#define ZARA_OAKHAVEN_Y 18
#define CAMP_ZARA_X 42
#define CAMP_ZARA_Y 27
#define CAMP_GARRAN_X 46
#define CAMP_GARRAN_Y 27
#define CAMP_LIORA_X 50
#define CAMP_LIORA_Y 27
#define TRIBUTE_X 54
#define TRIBUTE_Y 15
#define TRIBUTE_GUARDS 7
#define TRIBUTE_SQUAD 47
#define TRIBUTE_GOLD 200
#define TRIBUTE_XP 500
#define EDDA_STILLBURY_X 61
#define EDDA_STILLBURY_Y 6

void western_init(GameState *g);
void western_prepare(GameState *g);
void western_update(GameState *g);
void western_migrate(GameState *g);
int western_talk(GameState *g);
int western_interact(GameState *g);
const char *western_hint(const GameState *g);
int western_convoy_enemy(const GameState *g, const Enemy *e);
int western_convoy_prompt(const GameState *g);
int western_handle_key(GameState *g, int key, int repeat);
int western_valid(const GameState *g);

#endif
