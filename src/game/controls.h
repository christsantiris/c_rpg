#ifndef CONTROLS_HEADER_H
#define CONTROLS_HEADER_H

// Gameplay commands the player can rebind. Escape and the arrow keys stay
// fixed so the menu and movement are always reachable.
typedef enum {
    CONTROL_MOVE_UP = 0,
    CONTROL_MOVE_DOWN,
    CONTROL_MOVE_LEFT, // also interacts with the object underfoot
    CONTROL_MOVE_RIGHT,
    CONTROL_DESCEND,
    CONTROL_ASCEND,
    CONTROL_PICK_UP,
    CONTROL_TALK,
    CONTROL_INVENTORY,
    CONTROL_SPELLBOOK,
    CONTROL_QUEST_JOURNAL,
    CONTROL_CAST_SPELL,
    CONTROL_RANGED_ATTACK,
    CONTROL_HELP,
    CONTROL_COUNT
} ControlAction;

void controls_reset(int bindings[CONTROL_COUNT]);
int controls_key_reserved(int scancode);
int controls_action_for_key(const int bindings[CONTROL_COUNT], int scancode);
int controls_assign(int bindings[CONTROL_COUNT], ControlAction action, int scancode);
int controls_valid(const int bindings[CONTROL_COUNT]);
const char *controls_label(ControlAction action);

#endif
