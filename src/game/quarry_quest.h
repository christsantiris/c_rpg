#ifndef QUARRY_QUEST_H
#define QUARRY_QUEST_H

#include "game.h"

#define QUARRY_ORIN_X 20
#define QUARRY_ORIN_Y 27
#define QUARRY_WORKER_X 16
#define QUARRY_WORKER_Y 27
#define QUARRY_CAMP_X 12
#define QUARRY_CAMP_Y 21

enum { QUARRY_NONE, QUARRY_PENDING, QUARRY_HAMMER, QUARRY_BOW, QUARRY_ROBES, QUARRY_LEGACY };
enum { QUARRY_POTION_NONE, QUARRY_POTION_PENDING, QUARRY_STRENGTH, QUARRY_INTELLIGENCE, QUARRY_POTION_LEGACY };
enum { QUARRY_MENU, QUARRY_EQUIPMENT, QUARRY_POTION };

int quarry_road(const GameState *g);
void quarry_prepare(GameState *g);
void quarry_update(GameState *g);
int quarry_talk(GameState *g);
int quarry_dialogue(const GameState *g);
int quarry_handle_key(GameState *g, int key, int repeat);
int quarry_reward_interact(GameState *g, int pickup);
void quarry_record_defeat(GameState *g);
int quarry_migrate(GameState *g, int version);
int quarry_valid(const GameState *g);
void game_refresh_quarry_stage(GameState *g);

#endif
