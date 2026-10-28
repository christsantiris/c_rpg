#ifndef BOSS_GATES_H
#define BOSS_GATES_H

#include "game.h"

typedef enum {
    BOSS_GATE_NONE,
    BOSS_GATE_ROAD,
    BOSS_GATE_DESTINATION,
    BOSS_GATE_INTERNAL
} BossGatePolicy;

typedef struct {
    Location area;
    BossGatePolicy policy;
    const char *boss;
} BossGateRule;

typedef struct {
    Location area;
    int x;
    int y;
    int vertical;
} RoadCheckpoint;

const BossGateRule *boss_gate_rule(Location area);
int boss_gate_open(const GameState *g, Location area);
int boss_gate_crossing(GameState *g, Location area);
int boss_gate_checkpoints(const GameState *g, RoadCheckpoint *points);
void boss_gate_refresh(GameState *g);
void boss_gate_migrate(GameState *g);
int boss_gate_hint(const GameState *g);
int boss_gate_interact(GameState *g);
int boss_gate_bump(GameState *g, int x, int y);

#endif
