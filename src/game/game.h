#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include "map.h"
#include "enemy.h"

typedef struct {
    int  x, y;
    char name[21];
    int  hp,     max_hp;
    int  mp,     max_mp;
    int  attack, defense;
    int  level,  experience, experience_next;
} Player;

typedef struct {
    Player player;
    Map    map;
    int    level;
    Enemy  enemies[MAX_ENEMIES];
    int    enemy_count;
} GameState;

void game_init(GameState *g);
void game_move_player(GameState *g, int dx, int dy);
void game_descend(GameState *g);
void game_ascend(GameState *g);

#endif