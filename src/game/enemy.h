#include "actions.h"

#ifndef ENEMY_HEADER_H
#define ENEMY_HEADER_H

#define MAX_ENEMIES 10

typedef enum {
    ENEMY_GOBLIN = 0,
    ENEMY_SKELETON,
    ENEMY_ORC,
    ENEMY_TROLL
} EnemyType;

typedef struct {
    int       x, y;
    int       active;
    EnemyType type;
    char      name[16];
    int       hp, max_hp;
    int       attack, defense;
    int       experience;
} Enemy;

#endif