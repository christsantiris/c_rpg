#ifndef GAME_IRONFANG_H
#define GAME_IRONFANG_H

#include "game.h"

enum { IRON_ROCKFALL = 1, IRON_CART_ATTACK, IRON_PICK_SWEEP };

const char *ironfang_stage_name(int level);
void ironfang_generate(Map *map, int level);
void ironfang_pack(IronfangStage *state, const Map *map);
void ironfang_unpack(const IronfangStage *state, Map *map);
void ironfang_capture(const GameState *g, IronfangStage *state);
void ironfang_store(GameState *g);
void ironfang_enter(GameState *g, int portal);
void ironfang_leave(GameState *g);
void ironfang_change_level(GameState *g, int forward);
int ironfang_travel(GameState *g, int x, int y);
int ironfang_move(GameState *g, int x, int y);
int ironfang_interact(GameState *g);
const char *ironfang_hint(const GameState *g);
int ironfang_lit(const GameState *g, int x, int y);
int ironfang_tick(GameState *g, Enemy *enemy, int *damage);
int ironfang_attack_marks(const GameState *g, const Enemy *enemy, int x, int y);
int ironfang_tick_ore(GameState *g);
int ironfang_ore_marks(const GameState *g, int x, int y);
void ironfang_defeat(GameState *g);
int ironfang_valid(const GameState *g);
void ironfang_migrate_road(GameState *g);

#endif
