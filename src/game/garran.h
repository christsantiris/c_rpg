#ifndef GARRAN_H
#define GARRAN_H

#include "game.h"

#define GARRAN_SPEAKER "Caravan Scout Garran"
#define GARRAN_MAX_HP 90
#define GARRAN_DEFENSE 4
#define GARRAN_BOW_ATTACK 14
#define GARRAN_DAGGER_ATTACK 10
#define GARRAN_RANGE 5
#define GARRAN_INN_X 31
#define GARRAN_INN_Y 14

enum { GARRAN_WAITING, GARRAN_FOLLOWING, GARRAN_DEAD };
enum { GARRAN_CAMP, GARRAN_STILLBURY };
enum { GARRAN_MENU, GARRAN_JOIN, GARRAN_WAIT, GARRAN_QUEST };

void garran_init(GameState *g);
void garran_sync(GameState *g);
int garran_position(const GameState *g, int *x, int *y);
int garran_at(const GameState *g, int x, int y);
int garran_beside(const GameState *g, int x, int y);
void garran_follow(GameState *g, int old_x, int old_y);
void garran_turn(GameState *g);
void garran_take_hit(GameState *g, const Enemy *e);
int garran_revive(GameState *g, int hp);
int garran_talk(GameState *g, int x, int y);
int garran_dialogue(const GameState *g);
int garran_handle_key(GameState *g, int key, int repeat);
const char *garran_hint(const GameState *g);
int garran_valid(const GameState *g);

#endif
