#include "gambler_renderer.h"
#include "sprites.h"

static void draw_gambler_room(Renderer *r) {
    int tiles_x = (r->screen_w + TILE_SIZE - 1) / TILE_SIZE;
    int tiles_y = (r->screen_h + TILE_SIZE - 1) / TILE_SIZE;
    for (int y = 0; y < tiles_y; y++) {
        for (int x = 0; x < tiles_x; x++) {
            draw_tavern_floor(r, x, y);
        }
    }
    for (int x = 0; x < tiles_x; x++) {
        draw_tavern_wall(r, x, 0);
        draw_tavern_wall(r, x, tiles_y - 1);
    }
    for (int y = 0; y < tiles_y; y++) {
        draw_tavern_wall(r, 0, y);
        draw_tavern_wall(r, tiles_x - 1, y);
    }
}

static int gambler_option_y(const Renderer *r, int option) {
    int first = r->screen_h < 440 ? 150 : 330;
    int spacing = r->screen_h < 440 ? 34 : 42;
    return first + option * spacing;
}

SDL_Rect gambler_button_rect(const Renderer *r, int option) {
    int width = r->screen_w < 620 ? r->screen_w - 64 : 520;
    return (SDL_Rect){(r->screen_w - width) / 2,
        gambler_option_y(r, option) - 8, width, 32};
}

static void gambler_option_label(const GameState *g, GamblerOption option, char *label, size_t size) {
    if (option.type == GAMBLER_OPTION_BET) {
        SDL_snprintf(label, size, "WAGER %d GOLD", option.wager);
    } else if (option.type == GAMBLER_OPTION_LOAN) {
        SDL_snprintf(label, size, "FULL RECOVERY    ADD %d GOLD DEBT",
            option.wager);
    } else if (option.type == GAMBLER_OPTION_REPAY) {
        int payment = g->gold < g->gambler_debt ? g->gold : g->gambler_debt;
        SDL_snprintf(label, size, "REPAY DEBT    %d GOLD", payment);
    } else {
        SDL_strlcpy(label, "LEAVE THE TABLE", size);
    }
}

void gambler_draw(Renderer *r, const GameState *g, GamblerScreen *s) {
    draw_gambler_room(r);
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {205, 200, 190, 255};
    SDL_Color red = {200, 60, 60, 255};
    SDL_Color green = {82, 166, 94, 255};
    SDL_Color hint = {120, 130, 120, 255};
    int cx = r->screen_w / 2;
    int compact = r->screen_h < 440;
    int title_y = compact ? 18 : 40;
    int stats_y = compact ? 50 : 82;
    char text[128];

    renderer_draw_text(r, "ROOK'S TABLE", cx - 88, title_y, gold,
        r->font_large);
    SDL_snprintf(text, sizeof(text), "YOUR GOLD: %d", g->gold);
    renderer_draw_text(r, text, cx - 245, stats_y, gold, r->font_small);
    SDL_snprintf(text, sizeof(text), "DEBT: %d / %d", g->gambler_debt,
        GAMBLER_DEBT_LIMIT);
    renderer_draw_text(r, text, cx + 70, stats_y,
        g->gambler_debt >= GAMBLER_DEBT_LIMIT ? red : white,
        r->font_small);

    if (compact) {
        renderer_draw_text(r, "EVEN DRAW. A WIN PAYS TWICE THE STAKE.",
            cx - 245, 82, white, r->font_tiny);
        renderer_draw_text(r, "ROOK CAN FINANCE FULL HP AND MP RECOVERY.",
            cx - 245, 100, hint, r->font_tiny);
        if (g->message_count > 0) {
            renderer_draw_text(r, g->messages[g->message_count - 1],
                cx - 245, 120, gold, r->font_tiny);
        }
    } else {
        draw_gambler_portrait(r, cx - 255, 122, 3);
        renderer_draw_text(r, "ROOK", cx - 112, 132, gold, r->font_small);
        renderer_draw_text(r, "HIGH CARD. EVEN ODDS.", cx - 112, 162,
            white, r->font_small);
        renderer_draw_text(r, "A WIN PAYS TWICE YOUR STAKE.", cx - 112, 188,
            white, r->font_small);
        renderer_draw_text(r,
            "GAMBLE, REPAY DEBT, OR FINANCE FULL RECOVERY.", cx - 112, 218,
            hint, r->font_tiny);
        const char *status;
        SDL_Color status_color;
        int recovery_cost = game_gambler_recovery_cost(g);
        int loan = game_gambler_loan_amount(g);
        if (loan > 0) {
            int payment = g->gold < recovery_cost ? g->gold : recovery_cost;
            SDL_snprintf(text, sizeof(text),
                "FULL RECOVERY: PAY %d NOW, ADD %d TO DEBT.",
                payment, loan);
            status = text;
            status_color = green;
        } else if (recovery_cost > 0 && g->gold < recovery_cost) {
            status = "CREDIT IS EXHAUSTED. EARN GOLD OR REPAY YOUR DEBT.";
            status_color = red;
        } else if (g->gambler_debt >= GAMBLER_DEBT_LIMIT) {
            status = "ROOK REFUSES TO GAMBLE UNTIL YOU REPAY HIM.";
            status_color = red;
        } else if (recovery_cost > 0) {
            status = "YOU CAN AFFORD RECOVERY. GAMBLING IS YOUR CHOICE.";
            status_color = green;
        } else {
            status = "PLACE A WAGER OR SETTLE YOUR MARKER.";
            status_color = green;
        }
        renderer_draw_text(r, status, cx - 112, 246, status_color,
            r->font_tiny);
        if (g->message_count > 0) {
            renderer_draw_text(r, g->messages[g->message_count - 1],
                cx - 245, 286, gold, r->font_tiny);
        }
    }

    GamblerOption options[MAX_GAMBLER_OPTIONS];
    int count = gambler_build_options(g, options);
    if (s->selected >= count) {
        s->selected = count - 1;
    }
    for (int i = 0; i < count; i++) {
        SDL_Rect row = gambler_button_rect(r, i);
        if (s->selected == i) {
            SDL_SetRenderDrawColor(r->sdl, 55, 48, 35, 255);
            SDL_RenderFillRect(r->sdl, &row);
            renderer_draw_text(r, ">", row.x + 12,
                gambler_option_y(r, i), gold, r->font_small);
        }
        char label[96];
        gambler_option_label(g, options[i], label, sizeof(label));
        renderer_draw_text(r, label, row.x + 38, gambler_option_y(r, i),
            s->selected == i ? gold : white, r->font_small);
    }

    renderer_draw_text(r, "UP/DOWN SELECT   ENTER CHOOSE   ESC LEAVE",
        cx - 215, r->screen_h - 46, hint, r->font_small);
}
