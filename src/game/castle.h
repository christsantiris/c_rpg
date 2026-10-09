#ifndef CASTLE_HEADER_H
#define CASTLE_HEADER_H

#include "game.h"

const char *castle_floor_name(int level);
void castle_generate(Map *m, int level);
void castle_migrate_layout(GameState *g);
int castle_migrate_encounters(GameState *g);
void castle_spawn(GameState *g);
void castle_store(GameState *g);
void castle_enter(GameState *g, int level);
void castle_leave(GameState *g, int town);
int castle_travel(GameState *g, int forward);
void castle_request(GameState *g, int escape);
int castle_prompt_key(GameState *g, int key, int repeat);
int castle_has_interaction(const GameState *g);
int castle_interact(GameState *g);
int castle_step(GameState *g);
int castle_tick(GameState *g);
int castle_attack_marks(const Map *m, const Enemy *e, int x, int y);
int castle_barrier_warning(const GameState *g, int x, int y);
int castle_fire_warning(const GameState *g, int x, int y);
int castle_enemy_damage(const GameState *g, const Enemy *e, int damage);
void castle_record_death(GameState *g, Enemy *e);

#endif
