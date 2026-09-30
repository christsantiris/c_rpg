#include "combat_feedback.h"

static CombatFeedbackEvent feedback_events[MAX_COMBAT_FEEDBACK];
static int feedback_first = 0;
static int feedback_count = 0;

void combat_feedback_clear(void) {
    feedback_first = 0;
    feedback_count = 0;
}

void combat_feedback_add(const GameState *g, CombatFeedbackKind kind, CombatFeedbackArrival arrival, int x, int y, int amount) {
    int slot;
    if (feedback_count < MAX_COMBAT_FEEDBACK) {
        slot = (feedback_first + feedback_count) % MAX_COMBAT_FEEDBACK;
        feedback_count++;
    } else {
        // A full list replaces its oldest result so the newest one shows.
        slot = feedback_first;
        feedback_first = (feedback_first + 1) % MAX_COMBAT_FEEDBACK;
    }
    CombatFeedbackEvent *event = &feedback_events[slot];
    event->kind = kind;
    event->arrival = arrival;
    event->location = g->location;
    event->level = g->level;
    event->x = x;
    event->y = y;
    event->amount = amount;
    #ifndef TEST_BUILD
    event->created_at = SDL_GetTicks();
    #else
    event->created_at = 0;
    #endif
}

int combat_feedback_count(void) {
    return feedback_count;
}

// Index 0 is the oldest result still in the list.
const CombatFeedbackEvent *combat_feedback_get(int index) {
    return &feedback_events[(feedback_first + index) % MAX_COMBAT_FEEDBACK];
}
