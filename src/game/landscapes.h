#ifndef LANDSCAPES_H
#define LANDSCAPES_H

#include "game.h"

void map_generate_frost_ridges(Map *m, int level);
int landscape_tile(TileType tile);
int landscape_has_interaction(const GameState *g);
int landscape_interact(GameState *g);
void landscape_new_expedition(Map *m);
void landscape_migrate(GameState *g);
void sunscar_migrate(GameState *g);

#endif
