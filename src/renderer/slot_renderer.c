#include "slot_renderer.h"
#include "sprites.h"
#include "../systems/save_load.h"
#include <stdio.h>

void slot_draw(Renderer *r, const SlotSelect *s, int is_save) {
    int full_tiles_x = r->screen_w / TILE_SIZE;

    for (int y = 0; y < r->tiles_y; y++)
        for (int x = 0; x < full_tiles_x; x++)
            draw_floor(r, x, y);

    for (int x = 0; x < full_tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, full_tiles_x - 1, y);
    }

    SDL_Color gold   = {220, 180,  60, 255};
    SDL_Color green  = { 80, 160,  80, 255};
    SDL_Color dimmed = { 80,  80,  80, 255};
    SDL_Color hint   = { 50,  70,  50, 255};

    int cx = r->screen_w / 2;
    int cy = r->screen_h / 2;

    renderer_draw_text(r, is_save ? "SAVE GAME" : "LOAD GAME",
        cx - 70, cy - 100, gold, r->font_large);

    for (int i = 0; i < 3; i++) {
        int item_y = cy - 20 + i * 40;
        char label[48];
        char level_label[16];
        char name[21];
        int level;
        int occupied = get_save_preview(i + 1, name, &level);

        if (occupied) {
            snprintf(label, sizeof(label), "SLOT %d - %s", i + 1, name);
            snprintf(level_label, sizeof(level_label), "LVL %d", level);
        } else {
            snprintf(label, sizeof(label), "SLOT %d - EMPTY", i + 1);
        }

        SDL_Color color = occupied ? green : dimmed;

        if (s->selected == i) {
            color = gold;
            renderer_draw_text(r, ">", cx - 240, item_y, gold, r->font_small);
        }
        renderer_draw_text(r, label, cx - 220, item_y, color, r->font_small);
        if (occupied) {
            renderer_draw_text(r, level_label, cx + 130, item_y, color,
                r->font_small);
        }
    }

    renderer_draw_text(r, "UP DOWN NAVIGATE   ENTER SELECT   ESC CANCEL",
        cx - 220, cy + 120, hint, r->font_small);
}
