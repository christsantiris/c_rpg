#include "class_select_renderer.h"
#include "sprites.h"
#include <stdio.h>

static const char *class_names[3] = { "Warrior", "Mage", "Rogue" };
static const char *class_descs[3] = {
    "HP:150  MP: 20  ATK:14  DEF:6",
    "HP: 70  MP:100  ATK: 6  DEF:3",
    "HP:100  MP: 40  ATK:10  DEF:4"
};
static const char *class_items[3] = {
    "Starts with: Rusty Sword",
    "Starts with: Magic Arrow Scroll",
    "Starts with: Bow"
};

void class_select_draw(Renderer *r, const ClassSelectScreen *s) {
    int cx = r->screen_w / 2;
    int cy = r->screen_h / 2;

    for (int y = 0; y < r->tiles_y; y++) {
        for (int x = 0; x < r->tiles_x; x++) {
            draw_floor(r, x, y);
        }
    }
    for (int x = 0; x < r->tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, r->tiles_x - 1, y);
    }

    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dim = {120, 120, 120, 255};
    SDL_Color green = {80, 160, 80, 255};
    SDL_Color hint = {50, 70, 50, 255};

    renderer_draw_text(r, "CHOOSE YOUR CLASS", cx - 136, cy - 110, gold, r->font_large);

    for (int i = 0; i < 3; i++) {
        int item_y = cy - 50 + i * 36;
        SDL_Color col = (i == s->selected) ? gold : dim;
        char label[32];
        if (i == s->selected) {
            snprintf(label, sizeof(label), "> %s", class_names[i]);
        } else {
            snprintf(label, sizeof(label), "  %s", class_names[i]);
        }
        renderer_draw_text(r, label, cx - 120, item_y, col, r->font_large);
    }

    renderer_draw_text(r, class_descs[s->selected], cx - 120, cy + 70, white, r->font_small);
    renderer_draw_text(r, class_items[s->selected], cx - 120, cy + 92, green, r->font_small);

    renderer_draw_text(r, "UP/DOWN TO SELECT   ENTER TO CONFIRM   ESC TO GO BACK",
                       cx - 210, cy + 130, hint, r->font_small);
}
