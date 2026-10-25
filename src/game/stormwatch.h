#ifndef GAME_STORMWATCH_H
#define GAME_STORMWATCH_H

#include "game.h"

enum { STORM_DIVE = 1, STORM_SWEEP, STORM_LIGHTNING, STORM_BURST };

const char *stormwatch_stage_name(int level);
void stormwatch_generate(Map *map, int level);
void stormwatch_capture(const GameState *g, StormwatchStage *state);
void stormwatch_pack(StormwatchStage *state, const Map *map);
void stormwatch_unpack(const StormwatchStage *state, Map *map);
void stormwatch_store(GameState *g);
void stormwatch_enter(GameState *g, int portal);
void stormwatch_change_level(GameState *g, int forward);
void stormwatch_leave(GameState *g);
int stormwatch_travel(GameState *g, int x, int y);
int stormwatch_interact(GameState *g);
const char *stormwatch_hint(const GameState *g);
void stormwatch_wind(GameState *g);
int stormwatch_wind_marks(const GameState *g, int lane, int x, int y);
int stormwatch_tick(GameState *g, Enemy *enemy, int *damage);
int stormwatch_attack_marks(const GameState *g, const Enemy *enemy, int x, int y);
void stormwatch_defeat(GameState *g);
int stormwatch_defense(const Enemy *enemy, int x, int y);
int stormwatch_valid(const GameState *g);

#endif
