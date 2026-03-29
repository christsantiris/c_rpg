#include "name_entry_renderer.h"
#include "sprites.h"
#include <string.h>

void name_entry_draw(Renderer *r, const NameEntry *n) {
    int cx = r->screen_w / 2;
    int cy = r->screen_h / 2;

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

    SDL_Color gold   = {220, 180,  60, 255};
    SDL_Color green  = { 80, 160,  80, 255};
    SDL_Color hint   = { 50,  70,  50, 255};
    SDL_Color cursor = {220, 180,  60, 255};

    // Prompt
    renderer_draw_text(r, "ENTER YOUR NAME", cx - 120, cy - 60,
                       gold, r->font_large);

    // Name typed so far
    if (n->length > 0)
        renderer_draw_text(r, n->name, cx - 120, cy, green, r->font_large);

    // Blinking cursor block
    SDL_Rect cur = {
        cx - 120 + n->length * 16, cy,
        10, 18
    };
    SDL_SetRenderDrawColor(r->sdl, cursor.r, cursor.g, cursor.b, 255);
    SDL_RenderFillRect(r->sdl, &cur);

    // Hints
    renderer_draw_text(r, "ENTER TO CONFIRM   ESC TO CANCEL",
                       cx - 160, cy + 60, hint, r->font_small);
}