#include "landing_renderer.h"
#include "sprites.h"
#include "../screens/landing.h"
#include "../audio/music.h"
#include "../audio/sfx.h"
#include <stdio.h>

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

    // Build dynamic labels
    char music_label[16];
    char sfx_label[16];
    snprintf(music_label, sizeof(music_label), "MUSIC %s", music_enabled() ? "ON" : "OFF");
    snprintf(sfx_label,   sizeof(sfx_label),   "SFX %s",   sfx_enabled()   ? "ON" : "OFF");

    const char *labels_base[5] = {
        "NEW GAME", "LOAD GAME", music_label, sfx_label, "QUIT"
    };
    const char *labels_active[7] = {
        "NEW GAME", "CONTINUE", "SAVE GAME", "LOAD GAME", music_label, sfx_label, "QUIT"
    };

    // Menu items
    int count = landing_item_count(s);
    const char **labels = s->has_active_game ? labels_active : labels_base;

    for (int i = 0; i < count; i++) {
        int item_y = (r->screen_h / 2) - 20 + i * 36;
        int item_x = (r->screen_w / 2) - 80;
        SDL_Color color = (i == count - 1) ? dimmed : green;
        if (s->selected == i) {
            renderer_draw_text(r, ">", item_x - 24, item_y, cursor, r->font_small);
            renderer_draw_text(r, labels[i], item_x, item_y, gold, r->font_small);
        } else {
            renderer_draw_text(r, labels[i], item_x, item_y, color, r->font_small);
        }
    }

    // Hint
    SDL_Color hint = {50, 70, 50, 255};
    int hint_x = (r->screen_w / 2) - 140;
    int hint_y = r->screen_h - 60;
    renderer_draw_text(r, "UP DOWN NAVIGATE   ENTER SELECT", hint_x, hint_y, hint, r->font_small);

    // Confirm new game prompt
    if (s->confirming_new_game) {
        SDL_Color white = {200, 200, 200, 255};
        renderer_draw_text(r, "START NEW GAME? (Y/N)",
            (r->screen_w / 2) - 160,
            (r->screen_h / 2) + 160,
            white, r->font_small);
    }
}