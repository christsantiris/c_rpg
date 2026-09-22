#ifndef ACTION_HEADER_H
#define ACTION_HEADER_H

#include "enemy.h"
#include "item.h"

typedef enum {
    ACTION_NONE,
    ACTION_MOVE,
    ACTION_MELEE_ATTACK,
    ACTION_DESCEND,
    ACTION_ASCEND,
    ACTION_PICK_UP,
    ACTION_INTERACT,
    ACTION_USE_ITEM,
    ACTION_EQUIP_ITEM,
    ACTION_EQUIP_OFF_HAND,
    ACTION_DROP_ITEM,
    ACTION_CAST_SPELL,
    ACTION_RANGED_ATTACK
} ActionType;

typedef struct {
    ActionType type;
    int        target_x, target_y;
} Action;

// Transient presentation events for one enemy turn, kept outside saved game state.
typedef struct {
    EnemyType type;
    int start_x, start_y;
    int target_x, target_y;
} EnemyProjectile;

typedef struct {
    EnemyProjectile shots[MAX_ENEMIES];
    int count;
} EnemyProjectiles;

Item random_enemy_item(int level);
Item boss_equipment_reward(EnemyType type);

#endif
