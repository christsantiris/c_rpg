#ifndef ENEMY_TACTICS_H
#define ENEMY_TACTICS_H

#include "game.h"

// -1: use ordinary movement, 0: hold an assigned position, 1: moved one tile.
int enemy_tactics_move(GameState *g, int index);
int enemy_goblin_frontline(const Enemy *enemy);

#endif
