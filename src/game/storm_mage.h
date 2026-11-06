#ifndef STORM_MAGE_H
#define STORM_MAGE_H

#include "game.h"

#define STORM_MAGE_NAME "Soren"
#define STORM_MAGE_SPEAKER "Mage Soren"
#define STORM_MAGE_X 46
#define STORM_MAGE_Y 12
#define STORM_MAGE_INN_X 28
#define STORM_MAGE_INN_Y 14
#define STORM_GUIDE_OAK_X 17
#define STORM_GUIDE_OAK_Y 13
#define STORM_GUIDE_DAWN_X 12
#define STORM_GUIDE_DAWN_Y 14

enum { STORM_MAGE_CAPTIVE, STORM_MAGE_FREED, STORM_MAGE_FOLLOWING, STORM_MAGE_WAITING, STORM_MAGE_DEAD };

void storm_mage_init(GameState *g);
void storm_mage_sync(GameState *g);
int storm_mage_max_hp(const GameState *g);
int storm_mage_max_mp(const GameState *g);
int storm_mage_at(const GameState *g, int x, int y);
int storm_mage_beside(const GameState *g, int x, int y);
int storm_mage_hostile(const GameState *g, const Enemy *e);
void storm_mage_follow(GameState *g, int old_x, int old_y);
void storm_mage_turn(GameState *g);
void storm_mage_take_hit(GameState *g, const Enemy *e);
int storm_mage_revive(GameState *g, int hp);
int storm_mage_talk(GameState *g, int x, int y);
int storm_mage_prompt_active(const GameState *g);
int storm_mage_handle_key(GameState *g, int key, int repeat);
void storm_mage_place_guide(GameState *g);
void storm_mage_migrate(GameState *g, int version);
int storm_mage_valid(const GameState *g);

#endif
