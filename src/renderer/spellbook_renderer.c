#include "spellbook_renderer.h"
#include "sprites.h"

void spellbook_draw(Renderer *r, const GameState *g, const SpellbookScreen *s) {
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
    SDL_Color white  = {200, 200, 200, 255};
    SDL_Color blue   = { 80, 140, 220, 255};

    int cx = r->screen_w / 2;
    int cy = r->screen_h / 2;

    renderer_draw_text(r, "SPELLBOOK", cx - 60, 40, gold, r->font_large);

    if (g->player.known_spell_count == 0) {
        renderer_draw_text(r, "NO SPELLS KNOWN", cx - 90, cy, dimmed, r->font_small);
        renderer_draw_text(r, "USE SCROLLS TO LEARN SPELLS",
            cx - 160, cy + 36, dimmed, r->font_small);
    } else {
        for (int i = 0; i < g->player.known_spell_count; i++) {
            const Spell *sp = &g->player.known_spells[i];
            int item_y = 130 + i * 40;

            char label[64];
            SDL_snprintf(label, sizeof(label), "%-16s  %d MP",
                sp->name, sp->mp_cost);

            int is_equipped = (g->player.equipped_spell == i);
            SDL_Color color = is_equipped ? gold : white;

            if (s->selected == i) {
                renderer_draw_text(r, ">", cx - 200, item_y, gold, r->font_small);
                renderer_draw_text(r, label, cx - 180, item_y, color, r->font_small);
                if (is_equipped)
                    renderer_draw_text(r, "[E]", cx + 120, item_y, blue, r->font_small);
            } else {
                renderer_draw_text(r, label, cx - 180, item_y, color, r->font_small);
                if (is_equipped)
                    renderer_draw_text(r, "[E]", cx + 120, item_y, blue, r->font_small);
            }
        }
    }

    renderer_draw_text(r, "ENTER EQUIP   B/ESC CLOSE", cx - 130, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}