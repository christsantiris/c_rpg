#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include "map.h"

typedef struct {
    int x, y;
    char name[21];
} Player;

typedef struct {
    Player player;
    Map    map;
    int    level;
} GameState;

void game_init(GameState *g);
void game_move_player(GameState *g, int dx, int dy);
void game_descend(GameState *g);
void game_ascend(GameState *g);

#endif