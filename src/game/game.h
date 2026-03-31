#include "map.h"
#include "enemy.h"

#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#define MAX_MESSAGES 3
#define MAX_MESSAGE_LEN 40

typedef struct {
    int  x, y;
    char name[21];
    int  hp,     max_hp;
    int  mp,     max_mp;
    int  attack, defense;
    int  level,  experience, experience_next;
} Player;

typedef struct {
    Map   map;
    Enemy enemies[MAX_ENEMIES];
    int   enemy_count;
    int   valid;
} LevelCache;

typedef struct {
    Player player;
    Map    map;
    int    level;
    Enemy  enemies[MAX_ENEMIES];
    int    enemy_count;
    LevelCache level_cache[MAX_DEPTH];
    char messages[MAX_MESSAGES][MAX_MESSAGE_LEN];
    int  message_count;
} GameState;

void game_init(GameState *g);
void game_move_player(GameState *g, int dx, int dy);
void game_descend(GameState *g);
void game_ascend(GameState *g);

void action_resolve_player(GameState *g, Action a);
void action_resolve_enemies(GameState *g);

#endif