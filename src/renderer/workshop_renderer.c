#include "workshop_renderer.h"
#include "sprites.h"

static void centered(Renderer *r, const char *text, int y, SDL_Color color) {
    int width = 0;
    TTF_SizeText(r->font_small, text, &width, NULL);
    renderer_draw_text(r, text, (r->screen_w - width) / 2, y, color, r->font_small);
}

void workshop_draw(Renderer *r, const GameState *g, const WorkshopScreen *s) {
    for (int y = 0; y < r->tiles_y; y++) {
        for (int x = 0; x < r->screen_w / TILE_SIZE; x++) {
            if (x == 0 || y == 0 || x == r->screen_w / TILE_SIZE - 1 || y == r->tiles_y - 1) {
                draw_wall(r, x, y);
            } else {
                draw_floor(r, x, y);
            }
        }
    }
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {205, 205, 220, 255};
    SDL_Color dimmed = {130, 130, 145, 255};
    centered(r, "GARRICK'S WORKSHOP", 40, gold);
    centered(r, "SWORDS, AXES AND DAGGERS: +1 ATTACK, ONCE PER WEAPON", 76, white);
    char text[160];
    SDL_snprintf(text, sizeof(text), "YOUR GOLD: %d", g->gold);
    centered(r, text, 110, gold);
    int rows = workshop_visible_rows(r->screen_h);
    int start = workshop_list_start(s->selected, rows);
    if (!g->inventory_count) {
        centered(r, "BRING A BLADE TO SHARPEN", 170, dimmed);
    }
    for (int i = start; i < g->inventory_count && i < start + rows; i++) {
        const Item *item = &g->inventory[i];
        int y = 150 + (i - start) * 32;
        int eligible = item_can_sharpen(item);
        const char *status = item->sharpened ? "ALREADY SHARPENED" : eligible ? "50 GOLD" : "CANNOT SHARPEN";
        const char *hand = g->equipped_main_hand == i ? " [MAIN]" : g->equipped_off_hand == i ? " [OFF]" : "";
        SDL_snprintf(text, sizeof(text), "%s%s  ATK +%d", item->name, hand, item->attack_bonus);
        renderer_draw_text(r, i == s->selected ? ">" : " ", r->screen_w / 2 - 300, y, gold, r->font_small);
        renderer_draw_text(r, text, r->screen_w / 2 - 278, y, eligible ? white : dimmed, r->font_tiny);
        renderer_draw_text(r, status, r->screen_w / 2 + 130, y, i == s->selected ? gold : dimmed, r->font_tiny);
    }
    if (g->message_count > 0) {
        renderer_draw_text(r, g->messages[g->message_count - 1], 40, r->screen_h - 126, white, r->font_tiny);
    }
    SDL_Rect button = workshop_button_rect(r->screen_w, r->screen_h);
    SDL_SetRenderDrawColor(r->sdl, gold.r, gold.g, gold.b, gold.a);
    SDL_RenderDrawRect(r->sdl, &button);
    centered(r, "SHARPEN (+1 ATTACK) - 50 GOLD", button.y + 7, gold);
    centered(r, "UP/DOWN: SELECT   ENTER: SHARPEN   ESC: BACK", r->screen_h - 48, dimmed);
}
