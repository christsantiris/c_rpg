#ifndef COMBAT_FEEDBACK_HEADER_H
#define COMBAT_FEEDBACK_HEADER_H

#include "game.h"

#define MAX_COMBAT_FEEDBACK 32

// Recent combat results for on-screen feedback. The list lives outside
// GameState, so it is never saved and never affects combat.
typedef enum {
    FEEDBACK_ENEMY_DAMAGE = 0,
    FEEDBACK_PLAYER_DAMAGE,
    FEEDBACK_HEAL,
    FEEDBACK_MANA_LOSS,
    FEEDBACK_ENEMY_CRITICAL,
    FEEDBACK_BLOCK,
    FEEDBACK_DODGE,
    FEEDBACK_MISS
} CombatFeedbackKind;

// Results carried by a projectile appear when it lands.
typedef enum {
    FEEDBACK_NOW = 0,
    FEEDBACK_AFTER_PLAYER_SHOT,
    FEEDBACK_AFTER_ENEMY_SHOT
} CombatFeedbackArrival;

typedef struct {
    CombatFeedbackKind kind;
    CombatFeedbackArrival arrival;
    Location location;
    int level;
    int x, y;
    int amount;
    Uint32 created_at;
} CombatFeedbackEvent;

void combat_feedback_clear(void);
void combat_feedback_add(const GameState *g, CombatFeedbackKind kind, CombatFeedbackArrival arrival, int x, int y, int amount);
int combat_feedback_count(void);
const CombatFeedbackEvent *combat_feedback_get(int index);

#endif
