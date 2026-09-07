#include "shop_renderer.h"
#include "sprites.h"

void shop_draw(Renderer *r, const GameState *g, const ShopScreen *s) {
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
    SDL_Color dimmed = { 80,  80,  80, 255};
    SDL_Color hint   = { 50,  70,  50, 255};
    SDL_Color white  = {200, 200, 200, 255};
    SDL_Color red    = {200,  60,  60, 255};
    SDL_Color green  = { 80, 160,  80, 255};

    int cx = r->screen_w / 2;

    const char *title = s->type == SHOP_TYPE_ALCHEMIST
        ? "ALCHEMIST" : "BLACKSMITH";
    renderer_draw_text(r, title, cx - 60, 40, gold, r->font_large);

    char gold_str[32];
    SDL_snprintf(gold_str, sizeof(gold_str), "YOUR GOLD: %d", g->gold);
    renderer_draw_text(r, gold_str, cx - 60, 80, gold, r->font_small);

    // Tab headers
    SDL_Color buy_color  = s->mode == 0 ? gold : dimmed;
    SDL_Color sell_color = s->mode == 1 ? gold : dimmed;
    renderer_draw_text(r, "BUY",  cx - 60, 108, buy_color,  r->font_small);
    renderer_draw_text(r, "SELL", cx,       108, sell_color, r->font_small);

    if (s->mode == 0) {
        // Buy mode
        for (int i = 0; i < s->item_count; i++) {
            const Item *item = &s->items[i];
            int item_y = 140 + i * 24;
            char label[64];
            SDL_snprintf(label, sizeof(label), "%-20s  %d gold",
                item->name, item->value);
            int can_afford = g->gold >= item->value;
            if (s->selected == i) {
                renderer_draw_text(r, ">", cx - 200, item_y, gold, r->font_small);
                renderer_draw_text(r, label, cx - 180, item_y,
                    can_afford ? gold : red, r->font_small);
            } else {
                renderer_draw_text(r, label, cx - 180, item_y,
                    can_afford ? white : dimmed, r->font_small);
            }
        }
    } else {
        // Sell mode
        if (g->inventory_count == 0) {
            renderer_draw_text(r, "NOTHING TO SELL",
                cx - 90, 200, dimmed, r->font_small);
        } else {
            for (int i = 0; i < g->inventory_count; i++) {
                const Item *item = &g->inventory[i];
                int item_y = 140 + i * 36;
                int sell_price = item->value / 2;
                char label[64];
                SDL_snprintf(label, sizeof(label), "%-20s  %d gold",
                    item->name, sell_price);
                if (s->selected == i) {
                    renderer_draw_text(r, ">", cx - 200, item_y, gold, r->font_small);
                    renderer_draw_text(r, label, cx - 180, item_y, green, r->font_small);
                } else {
                    renderer_draw_text(r, label, cx - 180, item_y, white, r->font_small);
                }
            }
        }
    }

    int detail_count = s->mode == 0 ? s->item_count : g->inventory_count;
    if (s->selected >= 0 && s->selected < detail_count) {
        const Item *selected = s->mode == 0
            ? &s->items[s->selected] : &g->inventory[s->selected];
        if (selected->type == ITEM_WEAPON) {
            int allowed = item_class_allowed(selected,
                g->player.player_class);
            char details[96];
            SDL_snprintf(details, sizeof(details), "%s | %s | %s%s",
                item_rarity_label(selected), item_class_label(selected),
                item_hands_label(selected), allowed ? "" : " | CLASS LOCKED");
            renderer_draw_text(r, details, cx - 180,
                (r->tiles_y - 4) * TILE_SIZE,
                allowed ? green : red, r->font_tiny);
            if (selected->critical_chance_bonus > 0) {
                char trait[64];
                SDL_snprintf(trait, sizeof(trait),
                    "+%d%% MELEE CRITICAL CHANCE",
                    selected->critical_chance_bonus);
                renderer_draw_text(r, trait, cx - 180,
                    (r->tiles_y - 3) * TILE_SIZE, green, r->font_tiny);
            } else if (selected->cleave_percent > 0) {
                char trait[64];
                SDL_snprintf(trait, sizeof(trait),
                    "%d%% DAMAGE TO NEARBY ENEMIES",
                    selected->cleave_percent);
                renderer_draw_text(r, trait, cx - 180,
                    (r->tiles_y - 3) * TILE_SIZE, green, r->font_tiny);
            } else if (selected->pierces_targets) {
                renderer_draw_text(r, "ARROWS PIERCE ALL TARGETS", cx - 180,
                    (r->tiles_y - 3) * TILE_SIZE, green, r->font_tiny);
            } else if (selected->spell_power_bonus > 0) {
                char trait[64];
                SDL_snprintf(trait, sizeof(trait), "+%d SPELL POWER",
                    selected->spell_power_bonus);
                renderer_draw_text(r, trait, cx - 180,
                    (r->tiles_y - 3) * TILE_SIZE, green, r->font_tiny);
            } else if (selected->armor_penetration_percent > 0) {
                char trait[64];
                SDL_snprintf(trait, sizeof(trait),
                    "IGNORES %d%% ENEMY DEFENSE",
                    selected->armor_penetration_percent);
                renderer_draw_text(r, trait, cx - 180,
                    (r->tiles_y - 3) * TILE_SIZE, green, r->font_tiny);
            }
        }
    }

    renderer_draw_text(r, "TAB BUY/SELL   ENTER SELECT   ESC CLOSE",
        cx - 200, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}
