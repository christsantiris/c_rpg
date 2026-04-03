#ifndef ENEMY_HEADER_H
#define ENEMY_HEADER_H

#define MAX_ENEMIES 15

typedef enum {
    ENEMY_SKELETON = 0,
    ENEMY_GOBLIN,
    ENEMY_ZOMBIE,
    ENEMY_ORC,
    ENEMY_TROLL,
    ENEMY_GIANT,
    ENEMY_GOBLIN_KING,
    ENEMY_LICH_KING,
    ENEMY_DEMON_LORD,
    ENEMY_RED_DRAGON,
    ENEMY_TARRASQUE
} EnemyType;

typedef struct {
    int       x, y;
    int       active;
    EnemyType type;
    char      name[16];
    int       hp, max_hp;
    int       attack, defense;
    int       experience;
    int       move_timer;
    int       is_boss;
} Enemy;

#endif