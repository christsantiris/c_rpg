#ifndef SAVE_LOAD_HEADER_H
#define SAVE_LOAD_HEADER_H

#include "../game/game.h"

#define SAVE_SLOTS 3

int save_game(const GameState *g, int slot);
int load_game(GameState *g, int slot);
int save_exists(int slot);
int get_save_preview(int slot, char *name_out, int *level_out);

#endif