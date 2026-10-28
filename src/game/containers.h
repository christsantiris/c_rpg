#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "game.h"

enum { CONTAINER_ARMORY = 1000, CONTAINER_CHAPEL };

int containers_at(const GameState *g, int x, int y, int *kind, int *opened);
const LootContainer *containers_record(const GameState *g, int x, int y);
TileType containers_ground(const GameState *g, int x, int y);
int containers_interact(GameState *g);
int containers_pickup(GameState *g);
const char *containers_hint(const GameState *g);
Item containers_reward(const LootContainer *c, int bit);
int containers_valid(const GameState *g);
int containers_kind_location(int kind);
void containers_restore(GameState *g);

#endif
