#ifndef CROWNROAD_NORTH_H
#define CROWNROAD_NORTH_H

#include "game.h"

#define NORTH_SQUADS 6
#define NORTH_SQUAD_SIZE 15
#define NORTH_ENEMIES (NORTH_SQUADS * NORTH_SQUAD_SIZE)

enum { NORTH_ARROW = 1 };

void northroad_generate(Map *map);
void northroad_enter(GameState *g, int from_castle, int portal);
void northroad_capture(const GameState *g, NorthRoadCache *state);
void northroad_store(GameState *g);
void northroad_leave(GameState *g, int castle);
int northroad_travel(GameState *g, int x, int y);
int northroad_interact(GameState *g);
int northroad_has_interaction(const GameState *g);
int northroad_tick(GameState *g, Enemy *enemy, int *damage);
int northroad_attack_marks(const GameState *g, const Enemy *enemy, int x, int y);
int northroad_shielded(const GameState *g, const Enemy *enemy);
int northroad_weapon_damage(const GameState *g, const Enemy *enemy, int damage);
int northroad_valid(const GameState *g);
void game_spawn_enemy(GameState *g, Enemy *enemy, EnemyType type, int x, int y);

#endif
