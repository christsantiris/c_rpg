#include "gambler.h"
#include <SDL2/SDL.h>

void gambler_init(GamblerScreen *s) {
    s->selected = 0;
}

int gambler_build_options(const GameState *g, GamblerOption options[MAX_GAMBLER_OPTIONS]) {
    int count = 0;
    int loan = game_gambler_loan_amount(g);
    if (loan > 0) {
        options[count++] = (GamblerOption){GAMBLER_OPTION_LOAN, loan};
    }
    if (g->gambler_debt < GAMBLER_DEBT_LIMIT) {
        static const int wagers[] = {5, 10, 25};
        if (g->gold > 0 && g->gold < wagers[0]) {
            options[count++] = (GamblerOption){GAMBLER_OPTION_BET, g->gold};
        }
        for (int i = 0; i < 3; i++) {
            if (g->gold >= wagers[i]) {
                options[count++] = (GamblerOption){GAMBLER_OPTION_BET, wagers[i]};
            }
        }
    }
    if (g->gambler_debt > 0 && g->gold > 0) {
        options[count++] = (GamblerOption){GAMBLER_OPTION_REPAY, 0};
    }
    options[count++] = (GamblerOption){GAMBLER_OPTION_LEAVE, 0};
    return count;
}

GamblerResult gambler_activate(GamblerScreen *s, const GameState *g, int *wager) {
    GamblerOption options[MAX_GAMBLER_OPTIONS];
    int count = gambler_build_options(g, options);
    if (s->selected >= count) {
        s->selected = count - 1;
    }
    *wager = options[s->selected].wager;
    if (options[s->selected].type == GAMBLER_OPTION_BET) {
        return GAMBLER_BET;
    }
    if (options[s->selected].type == GAMBLER_OPTION_LOAN) {
        return GAMBLER_LOAN;
    }
    if (options[s->selected].type == GAMBLER_OPTION_REPAY) {
        return GAMBLER_REPAY;
    }
    return GAMBLER_CLOSED;
}

GamblerResult gambler_handle_key(GamblerScreen *s, int scancode, const GameState *g, int *wager) {
    GamblerOption options[MAX_GAMBLER_OPTIONS];
    int count = gambler_build_options(g, options);
    if (s->selected >= count) {
        s->selected = count - 1;
    }
    if (scancode == SDL_SCANCODE_UP || scancode == SDL_SCANCODE_W) {
        s->selected--;
        if (s->selected < 0) {
            s->selected = 0;
        }
    } else if (scancode == SDL_SCANCODE_DOWN || scancode == SDL_SCANCODE_S) {
        s->selected++;
        if (s->selected >= count) {
            s->selected = count - 1;
        }
    } else if (scancode == SDL_SCANCODE_RETURN ||
        scancode == SDL_SCANCODE_KP_ENTER) {
        return gambler_activate(s, g, wager);
    } else if (scancode == SDL_SCANCODE_ESCAPE) {
        return GAMBLER_CLOSED;
    }
    return GAMBLER_NONE;
}
