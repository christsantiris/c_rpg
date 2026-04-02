#include "inventory_renderer.h"
#include "sprites.h"

void inventory_draw(Renderer *r, const GameState *g, const InventoryScreen *s) {
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

    int cx = r->screen_w / 2;
    int cy = r->screen_h / 2;

    renderer_draw_text(r, "INVENTORY", cx - 60, 40, gold, r->font_large);

    // Gold display
    char gold_str[32];
    SDL_snprintf(gold_str, sizeof(gold_str), "GOLD: %d", g->gold);
    renderer_draw_text(r, gold_str, cx - 60, 90, gold, r->font_small);

    if (g->inventory_count == 0) {
        renderer_draw_text(r, "EMPTY", cx - 30, cy, dimmed, r->font_small);
    } else {
        for (int i = 0; i < g->inventory_count; i++) {
            const Item *item = &g->inventory[i];
            int item_y = 130 + i * 36;

            char label[64];
            if (item->type == ITEM_POTION_HEALTH)
                SDL_snprintf(label, sizeof(label), "%s  +%d HP", item->name, item->heal_hp);
            else if (item->type == ITEM_POTION_MANA)
                SDL_snprintf(label, sizeof(label), "%s  +%d MP", item->name, item->heal_mp);
            else if (item->type == ITEM_WEAPON)
                SDL_snprintf(label, sizeof(label), "%s  +%d ATK", item->name, item->attack_bonus);
            else if (item->type == ITEM_ARMOR)
                SDL_snprintf(label, sizeof(label), "%s  +%d DEF", item->name, item->defense_bonus);
            else
                SDL_snprintf(label, sizeof(label), "%s", item->name);

            // Equipped indicator
            int is_equipped = (g->equipped_weapon == i || g->equipped_armor == i);
            SDL_Color color = is_equipped ? gold : white;

            if (s->selected == i) {
                renderer_draw_text(r, ">", cx - 200, item_y, gold, r->font_small);
                renderer_draw_text(r, label, cx - 180, item_y, color, r->font_small);
                if (is_equipped)
                    renderer_draw_text(r, "[E]", cx + 120, item_y, gold, r->font_small);
            } else {
                renderer_draw_text(r, label, cx - 180, item_y, color, r->font_small);
                if (is_equipped)
                    renderer_draw_text(r, "[E]", cx + 120, item_y, gold, r->font_small);
            }
        }
    }

    renderer_draw_text(r, "U USE  E EQUIP  D DROP  ESC CLOSE", cx - 180, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}