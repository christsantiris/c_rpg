#ifndef CASTLE_GATEHOUSE_H
#define CASTLE_GATEHOUSE_H

#include "game.h"

#define GATEHOUSE_SHORTCUT_X 26
#define GATEHOUSE_SHORTCUT_Y 21
#define GATEHOUSE_LEVER_X 26
#define GATEHOUSE_LEVER_Y 25

void gatehouse_generate(Map *m);
void gatehouse_supply(GameState *g);
int gatehouse_migrate(GameState *g);
int gatehouse_open_shortcut(GameState *g, int x, int y);

#endif
