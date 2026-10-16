#ifndef GRAVE_MARSHAL_H
#define GRAVE_MARSHAL_H

#include "game.h"
#include "actions.h"

typedef enum { MARSHAL_SWEEP = 1, MARSHAL_CROSS, MARSHAL_CHANNEL, MARSHAL_GUARDS } MarshalAttack;

void grave_marshal_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int grave_marshal_attack_marks(const Map *m, const Enemy *e, int x, int y);
int grave_marshal_tick(GameState *g, Enemy *e);
void grave_marshal_interrupt(GameState *g, int x, int y);
void grave_marshal_cancel_landing(GameState *g, Enemy *e, int x, int y);
void grave_marshal_restore_reward(GameState *g);
int grave_marshal_has_interaction(const GameState *g);
int grave_marshal_interact(GameState *g);
int grave_marshal_pickup(GameState *g);
void grave_marshal_migrate(GameState *g);

#endif
