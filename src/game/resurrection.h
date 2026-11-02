#ifndef RESURRECTION_H
#define RESURRECTION_H

#include "game.h"

#define SOULSTONE_X 25
#define SOULSTONE_Y 12
#define SOULSTONE_RITUAL_X 25
#define SOULSTONE_RITUAL_Y 14
#define SOULSTONE_WARLOCK_X 28
#define SOULSTONE_WARLOCK_Y 15
#define SOULSTONE_ART_X 22
#define SOULSTONE_ART_Y 11
#define RESURRECTION_GOLD 200
#define RESURRECTION_SCROLL_GOLD 400
#define RESURRECTION_LEVEL 10
#define RESURRECTION_MP 80

enum {
    RESURRECTION_NONE,
    RESURRECTION_SERVICE,
    RESURRECTION_PAID_CONFIRM,
    RESURRECTION_SCROLL_CONFIRM,
    RESURRECTION_SPELL_CONFIRM
};

int resurrection_available(const GameState *g);
void resurrection_place(GameState *g);
int resurrection_talk(GameState *g);
int resurrection_offer_spell(GameState *g);
int resurrection_prompt_active(const GameState *g);
// 1 handles input without a turn; 2 completes a Mage's ritual and consumes a turn.
int resurrection_handle_key(GameState *g, int key, int repeat);

#endif
