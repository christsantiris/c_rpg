#ifndef ALDRIC_H
#define ALDRIC_H

#include "game.h"

#define ALDRIC_SPEAKER "Veteran Aldric"
#define ALDRIC_MAX_HP 120
// A Warrior's base attack of 14 plus his Short Sword's 3.
#define ALDRIC_ATTACK 17
#define ALDRIC_DEFENSE 6

enum {
    ALDRIC_WAITING,
    ALDRIC_FOLLOWING,
    ALDRIC_DEAD
};

int aldric_at(const GameState *g, int x, int y);
int aldric_beside(const GameState *g, int x, int y);
void aldric_place(GameState *g);
void aldric_follow(GameState *g, int lead_old_x, int lead_old_y);
void aldric_turn(GameState *g);
void aldric_take_hit(GameState *g, const Enemy *e);
int aldric_revive(GameState *g, int hp);
void aldric_talk(GameState *g, int x, int y);
int aldric_prompt_active(const GameState *g);
int aldric_handle_key(GameState *g, int key, int repeat);

#endif
