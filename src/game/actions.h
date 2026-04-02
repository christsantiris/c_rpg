#ifndef ACTION_HEADER_H
#define ACTION_HEADER_H

typedef enum {
    ACTION_NONE,
    ACTION_MOVE,
    ACTION_MELEE_ATTACK,
    ACTION_DESCEND,
    ACTION_ASCEND,
    ACTION_PICK_UP,
    ACTION_USE_ITEM,
    ACTION_EQUIP_ITEM,
    ACTION_DROP_ITEM
} ActionType;

typedef struct {
    ActionType type;
    int        target_x, target_y;
} Action;

#endif