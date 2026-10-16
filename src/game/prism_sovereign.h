#ifndef PRISM_SOVEREIGN_H
#define PRISM_SOVEREIGN_H

#include "game.h"
#include "actions.h"

typedef struct { int x1, y1, x2, y2; } PrismSegment;
enum { PRISM_BEAM = 1, PRISM_SPLIT = 2, PRISM_ROBES_PENDING = 1, PRISM_POTION_PENDING = 2 };

void prism_sovereign_prepare_map(Map *m, const Enemy *enemies, int count, int px, int py);
int prism_sovereign_segments(const GameState *g, const Enemy *e, PrismSegment out[4]);
int prism_sovereign_attack_marks(const GameState *g, const Enemy *e, int x, int y);
int prism_sovereign_tick(GameState *g, Enemy *e, EnemyProjectiles *shots);
int prism_sovereign_has_interaction(const GameState *g);
int prism_sovereign_interact(GameState *g);
int prism_sovereign_pickup(GameState *g);
void prism_sovereign_restore_reward(GameState *g);
void prism_sovereign_cancel_landing(GameState *g, int x, int y);
void prism_sovereign_migrate(GameState *g);

#endif
