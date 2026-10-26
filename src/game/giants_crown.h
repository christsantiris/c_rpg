#ifndef GIANTS_CROWN_H
#define GIANTS_CROWN_H

#include "game.h"

#define CASTLE_ASHEN_GATE_X 8
#define GIANTS_APPROACH_ROUTE 20
#define ASHEN_APPROACH_ROUTE 13

const char *giants_stage_name(int level);
void giants_generate(Map *map, int level);
void giants_pack(GiantsStage *state, const Map *map);
void giants_unpack(const GiantsStage *state, Map *map);
void giants_capture(const GameState *g, GiantsStage *state);
void giants_store(GameState *g);
void giants_enter(GameState *g, int portal);
void giants_leave(GameState *g);
void giants_change_level(GameState *g, int forward);
int giants_travel(GameState *g, int x, int y);
int giants_interact(GameState *g);
const char *giants_hint(const GameState *g);
int giants_tick(GameState *g, Enemy *enemy, int *damage);
int giants_attack_marks(const Enemy *enemy, int x, int y);
void giants_defeat(GameState *g);
int giants_valid(const GameState *g);
void giants_place_castle_road(Map *map);
void giants_migrate_geography(GameState *g);

#endif
