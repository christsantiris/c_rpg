#ifndef GAMBLER_HEADER_H
#define GAMBLER_HEADER_H

#include "../game/game.h"

#define MAX_GAMBLER_OPTIONS 6

typedef enum {
    GAMBLER_OPTION_BET,
    GAMBLER_OPTION_LOAN,
    GAMBLER_OPTION_REPAY,
    GAMBLER_OPTION_LEAVE
} GamblerOptionType;

typedef struct {
    GamblerOptionType type;
    int wager;
} GamblerOption;

typedef enum {
    GAMBLER_NONE = 0,
    GAMBLER_CLOSED,
    GAMBLER_BET,
    GAMBLER_LOAN,
    GAMBLER_REPAY
} GamblerResult;

typedef struct {
    int selected;
} GamblerScreen;

void gambler_init(GamblerScreen *s);
int gambler_build_options(const GameState *g, GamblerOption options[MAX_GAMBLER_OPTIONS]);
GamblerResult gambler_activate(GamblerScreen *s, const GameState *g, int *wager);
GamblerResult gambler_handle_key(GamblerScreen *s, int scancode, const GameState *g, int *wager);

#endif
