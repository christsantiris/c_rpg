#include "landing_renderer.h"
#include "sprites.h"

static const char *MENU_LABELS[3] = {
    "NEW GAME",
    "LOAD GAME",
    "QUIT"
};

void landing_draw(Renderer *r, const LandingScreen *s) {
    int cx = r->tiles_x / 2;
    int cy = r->tiles_y / 2;

    // Background and border
    for (int y = 0; y < r->tiles_y; y++)
        for (int x = 0; x < r->tiles_x; x++)
            draw_floor(r, x, y);

    for (int x = 0; x < r->tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, r->tiles_x - 1, y);
    }

    // Title
    SDL_Color gold   = {220, 180,  60, 255};
    SDL_Color green  = { 80, 160,  80, 255};
    SDL_Color dimmed = { 40,  60,  40, 255};
    SDL_Color cursor = {220, 180,  60, 255};

    int title_x = (r->screen_w / 2) - 120;
    int title_y = (r->screen_h / 2) - 100;
    renderer_draw_text(r, "CASTLE OF NO RETURN", title_x, title_y, gold, r->font_large);

    // Menu items
    for (int i = 0; i < 3; i++) {
        int item_y = (r->screen_h / 2) - 20 + i * 36;
        int item_x = (r->screen_w / 2) - 80;

        SDL_Color color = (i == 2) ? dimmed : green;

        if (s->selected == i) {
            renderer_draw_text(r, ">", item_x - 24, item_y, cursor, r->font_small);
            renderer_draw_text(r, MENU_LABELS[i], item_x, item_y, gold, r->font_small);
        } else {
            renderer_draw_text(r, MENU_LABELS[i], item_x, item_y, color, r->font_small);
        }
    }

    // Hint
    SDL_Color hint = {50, 70, 50, 255};
    int hint_x = (r->screen_w / 2) - 140;
    int hint_y = r->screen_h - 60;
    renderer_draw_text(r, "UP DOWN NAVIGATE   ENTER SELECT", hint_x, hint_y, hint, r->font_small);
}