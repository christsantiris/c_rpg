#ifndef ACTION_HEADER_H
#define ACTION_HEADER_H

typedef enum {
    ACTION_NONE,
    ACTION_MOVE,
    ACTION_MELEE_ATTACK,
    ACTION_DESCEND,
    ACTION_ASCEND
} ActionType;

typedef struct {
    ActionType type;
    int        target_x, target_y;
} Action;

#endif