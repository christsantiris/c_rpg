#include "controls_renderer.h"
#include "sprites.h"
#include <stdio.h>

const char *controls_key_name(int scancode) {
    const char *name = SDL_GetScancodeName((SDL_Scancode)scancode);
    return name[0] ? name : "?";
}

static void draw_centered(Renderer *r, const char *text, int y, SDL_Color color, TTF_Font *font) {
    int w = 0;
    TTF_SizeText(font, text, &w, NULL);
    renderer_draw_text(r, text, (r->screen_w - w) / 2, y, color, font);
}

void controls_draw(Renderer *r, const GameState *g, const ControlsScreen *s) {
    int full_tiles_x = r->screen_w / TILE_SIZE;
    for (int y = 0; y < r->tiles_y; y++) {
        for (int x = 0; x < full_tiles_x; x++) {
            draw_floor(r, x, y);
        }
    }
    for (int x = 0; x < full_tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, full_tiles_x - 1, y);
    }

    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color green = {80, 160, 80, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dim = {120, 120, 120, 255};
    SDL_Color hint = {50, 70, 50, 255};

    draw_centered(r, "CONTROLS", 40, gold, r->font_large);

    int label_x = r->screen_w / 2 - CONTROLS_ROW_HALF_W + 24;
    int key_x = r->screen_w / 2 + 60;
    for (int i = 0; i < CONTROL_COUNT; i++) {
        // Text sits in the middle of its clickable row.
        int y = CONTROLS_LIST_TOP + i * CONTROLS_ROW_H + 8;
        int selected = i == s->selected;
        const char *key = selected && s->waiting_for_key
            ? "PRESS A KEY" : controls_key_name(g->key_bindings[i]);
        if (selected) {
            renderer_draw_text(r, ">", label_x - 24, y, gold, r->font_small);
        }
        renderer_draw_text(r, controls_label((ControlAction)i), label_x, y,
            selected ? gold : green, r->font_small);
        renderer_draw_text(r, key, key_x, y, selected ? gold : white, r->font_small);
    }

    int note_y = CONTROLS_LIST_TOP + CONTROL_COUNT * CONTROLS_ROW_H + 24;
    if (s->key_refused) {
        draw_centered(r, "ESC AND THE ARROW KEYS CANNOT BE CHANGED", note_y, white,
            r->font_small);
    } else if (s->swapped_action >= 0) {
        char note[96];
        snprintf(note, sizeof(note), "%s now uses %s",
            controls_label((ControlAction)s->swapped_action),
            controls_key_name(g->key_bindings[s->swapped_action]));
        draw_centered(r, note, note_y, white, r->font_small);
    } else {
        draw_centered(r, "ARROW KEYS ALWAYS MOVE   ESC ALWAYS OPENS THE MENU", note_y, dim,
            r->font_small);
    }

    draw_centered(r, s->waiting_for_key ? "PRESS A NEW KEY   ESC CANCEL"
        : "UP DOWN SELECT   ENTER CHANGE   ESC BACK", r->screen_h - 60, hint, r->font_small);
}
