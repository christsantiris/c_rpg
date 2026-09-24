#include "shop_renderer.h"
#include "equipment_compare_renderer.h"
#include "sprites.h"

static void draw_shop_room(Renderer *r) {
    int tiles_x = (r->screen_w + TILE_SIZE - 1) / TILE_SIZE;
    int tiles_y = (r->screen_h + TILE_SIZE - 1) / TILE_SIZE;
    for (int y = 0; y < tiles_y; y++) {
        for (int x = 0; x < tiles_x; x++) {
            draw_floor(r, x, y);
        }
    }
    for (int x = 0; x < tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, tiles_y - 1);
    }
    for (int y = 0; y < tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, tiles_x - 1, y);
    }
}

static int healer_option_y(const Renderer *r, int option) {
    int first = r->screen_h < 440 ? 110 : 300;
    int spacing = r->screen_h < 440 ? 32 : 40;
    return first + option * spacing;
}

SDL_Rect shop_healer_button_rect(const Renderer *r, int option) {
    int width = r->screen_w < 520 ? r->screen_w - 64 : 440;
    return (SDL_Rect){(r->screen_w - width) / 2,
        healer_option_y(r, option) - 7, width, 30};
}

static void draw_healer_option(Renderer *r, int option, int selected, int available, const char *label) {
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dimmed = {80, 80, 80, 255};
    SDL_Color red = {200, 60, 60, 255};
    SDL_Rect row = shop_healer_button_rect(r, option);
    if (selected) {
        SDL_SetRenderDrawColor(r->sdl, 36, 58, 48, 255);
        SDL_RenderFillRect(r->sdl, &row);
        renderer_draw_text(r, ">", row.x + 12, healer_option_y(r, option),
            gold, r->font_small);
    }
    SDL_Color color = selected ? (available ? gold : red)
        : (available ? white : dimmed);
    renderer_draw_text(r, label, row.x + 36, healer_option_y(r, option),
        color, r->font_small);
}

static void draw_healer_visit(Renderer *r, const GameState *g, const ShopScreen *s) {
    draw_shop_room(r);
    SDL_Color gold = {220, 180, 60, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color red = {200, 60, 60, 255};
    SDL_Color green = {80, 160, 80, 255};
    SDL_Color hint = {110, 130, 115, 255};
    int cx = r->screen_w / 2;
    int compact = r->screen_h < 440;
    int title_y = compact ? 18 : 40;
    int stats_y = compact ? 48 : 80;
    renderer_draw_text(r, "HEALER", cx - 48, title_y, gold, r->font_large);
    int hp_price = game_healer_price(g);
    int mp_price = game_healer_mana_price(g);
    int missing_hp = g->player.max_hp - g->player.hp;
    int missing_mp = g->player.max_mp - g->player.mp;
    char text[96];
    SDL_snprintf(text, sizeof(text), "YOUR GOLD: %d", g->gold);
    renderer_draw_text(r, text, cx - 205, stats_y, gold, r->font_small);
    SDL_snprintf(text, sizeof(text), "HP: %d / %d", g->player.hp, g->player.max_hp);
    renderer_draw_text(r, text, cx + 70, stats_y, white, r->font_small);
    SDL_snprintf(text, sizeof(text), "MP: %d / %d", g->player.mp, g->player.max_mp);
    renderer_draw_text(r, text, cx + 70, stats_y + 22, white, r->font_small);

    if (compact) {
        renderer_draw_text(r, "RESTORE HP OR MP: 1 GOLD PER 3 POINTS.", cx - 205, 86,
            white, r->font_tiny);
    } else {
        draw_healer_portrait(r, cx - 245, 124, 3);
        renderer_draw_text(r, "LYSA", cx - 105, 132, gold, r->font_small);
        renderer_draw_text(r, "WELCOME, TRAVELER.", cx - 105, 160,
            white, r->font_small);
        renderer_draw_text(r, "I CAN RESTORE ALL MISSING HP OR MP.", cx - 105, 184,
            white, r->font_small);
        renderer_draw_text(r, "RATE: 1 GOLD PER 3 POINTS, ROUNDED UP.", cx - 105, 218,
            hint, r->font_tiny);
        const char *hp_status = hp_price == 0 ? "HP: ALREADY FULL"
            : (g->gold < hp_price ? "HP: CANNOT AFFORD" : "HP: AVAILABLE");
        const char *mp_status = mp_price == 0 ? "MP: ALREADY FULL"
            : (g->gold < mp_price ? "MP: CANNOT AFFORD" : "MP: AVAILABLE");
        renderer_draw_text(r, hp_status, cx - 105, 244,
            hp_price == 0 || g->gold >= hp_price ? green : red, r->font_tiny);
        renderer_draw_text(r, mp_status, cx + 45, 244,
            mp_price == 0 || g->gold >= mp_price ? green : red, r->font_tiny);
    }

    char heal_label[96];
    if (hp_price == 0) {
        SDL_snprintf(heal_label, sizeof(heal_label),
            "FULL HEALTH - NO TREATMENT NEEDED");
    } else {
        SDL_snprintf(heal_label, sizeof(heal_label),
            "RESTORE %d HP    %d GOLD", missing_hp, hp_price);
    }
    draw_healer_option(r, 0, s->selected == 0,
        hp_price == 0 || g->gold >= hp_price, heal_label);
    char mana_label[96];
    if (mp_price == 0) {
        SDL_snprintf(mana_label, sizeof(mana_label),
            "FULL MANA - NO RESTORATION NEEDED");
    } else {
        SDL_snprintf(mana_label, sizeof(mana_label),
            "RESTORE %d MP    %d GOLD", missing_mp, mp_price);
    }
    draw_healer_option(r, 1, s->selected == 1,
        mp_price == 0 || g->gold >= mp_price, mana_label);
    draw_healer_option(r, 2, s->selected == 2, 1, "RETURN TO TOWN");
    renderer_draw_text(r, "UP/DOWN OR W/S SELECT   ENTER CONFIRM   ESC CLOSE",
        cx - 220, r->screen_h - (compact ? 20 : 48), hint, r->font_tiny);
}

static int shop_visible_rows(const Renderer *r) {
    int list_top = 140;
    int detail_top = (r->tiles_y - 9) * TILE_SIZE;
    int rows = (detail_top - list_top - 16) / 32;
    if (rows < 1) {
        return 1;
    }
    return rows;
}

static int shop_list_start(int selected, int count, int visible_rows) {
    int start = selected - visible_rows + 1;
    if (start < 0) {
        start = 0;
    }
    int max_start = count - visible_rows;
    if (max_start < 0) {
        max_start = 0;
    }
    if (start > max_start) {
        start = max_start;
    }
    return start;
}

static void draw_shop_scrollbar(Renderer *r, int x, int y, int height, int start, int count, int visible_rows) {
    if (count <= visible_rows) {
        return;
    }

    SDL_Color track_color = {38, 38, 55, 255};
    SDL_Color thumb_color = {155, 126, 48, 255};
    SDL_Rect track = {x, y, 6, height};
    SDL_SetRenderDrawColor(r->sdl, track_color.r, track_color.g,
        track_color.b, track_color.a);
    SDL_RenderFillRect(r->sdl, &track);

    int thumb_height = height * visible_rows / count;
    if (thumb_height < 16) {
        thumb_height = 16;
    }
    int travel = height - thumb_height;
    int max_start = count - visible_rows;
    int thumb_y = y + travel * start / max_start;
    SDL_Rect thumb = {x, thumb_y, 6, thumb_height};
    SDL_SetRenderDrawColor(r->sdl, thumb_color.r, thumb_color.g,
        thumb_color.b, thumb_color.a);
    SDL_RenderFillRect(r->sdl, &thumb);
}

void shop_draw(Renderer *r, const GameState *g, const ShopScreen *s) {
    if (s->type == SHOP_TYPE_HEALER) {
        draw_healer_visit(r, g, s);
        return;
    }
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
    SDL_Color dimmed = {80, 80, 80, 255};
    SDL_Color hint = {50, 70, 50, 255};
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color red = {200, 60, 60, 255};
    SDL_Color green = {80, 160, 80, 255};

    int cx = r->screen_w / 2;

    const char *title = s->type == SHOP_TYPE_ALCHEMIST
        ? "ALCHEMIST" : "BLACKSMITH";
    renderer_draw_text(r, title, cx - 60, 40, gold, r->font_large);

    char gold_str[32];
    SDL_snprintf(gold_str, sizeof(gold_str), "YOUR GOLD: %d", g->gold);
    renderer_draw_text(r, gold_str, cx - 60, 80, gold, r->font_small);

    char tier[32];
    SDL_snprintf(tier, sizeof(tier), "STOCK TIER %d/4", s->stock_tier);
    renderer_draw_text(r, tier, cx + 100, 80, gold, r->font_tiny);
    const char *unlock = s->stock_tier < 4
        ? "NEXT TIER: DEFEAT AN AREA BOSS"
        : "ALL STOCK TIERS UNLOCKED";
    renderer_draw_text(r, unlock, cx + 100, 96, dimmed, r->font_tiny);

    // Tab headers
    SDL_Color buy_color  = s->mode == 0 ? gold : dimmed;
    SDL_Color sell_color = s->mode == 1 ? gold : dimmed;
    renderer_draw_text(r, "BUY",  cx - 60, 108, buy_color,  r->font_small);
    renderer_draw_text(r, "SELL", cx,       108, sell_color, r->font_small);

    int list_count = s->mode == 0 ? s->item_count : g->inventory_count;
    int visible_rows = shop_visible_rows(r);
    int list_start = shop_list_start(s->selected, list_count, visible_rows);
    int list_end = list_start + visible_rows;
    if (list_end > list_count) {
        list_end = list_count;
    }
    if (list_count > 0) {
        char range[32];
        SDL_snprintf(range, sizeof(range), "%d-%d OF %d",
            list_start + 1, list_end, list_count);
        renderer_draw_text(r, range, cx + 90, 108, dimmed, r->font_tiny);
    }

    if (s->mode == 0) {
        // Buy mode
        for (int i = list_start; i < list_end; i++) {
            const Item *item = &s->items[i];
            int item_y = 140 + (i - list_start) * 32;
            char label[64];
            SDL_snprintf(label, sizeof(label), "%-20s  %d gold",
                item->name, shop_buy_price(item));
            int can_afford = g->gold >= shop_buy_price(item);
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
            for (int i = list_start; i < list_end; i++) {
                const Item *item = &g->inventory[i];
                int item_y = 140 + (i - list_start) * 32;
                int sell_price = shop_sell_price(item);
                int accepted = shop_accepts_item(s->type, item);
                char label[64];
                SDL_snprintf(label, sizeof(label), "%-20s  %d gold",
                    item->name, sell_price);
                if (item->type == ITEM_TREASURE_MAP) {
                    SDL_snprintf(label, sizeof(label), "%s  NOT FOR SALE", item->name);
                } else if (!accepted) {
                    SDL_snprintf(label, sizeof(label), "%s  %s ONLY", item->name,
                        s->type == SHOP_TYPE_ALCHEMIST ? "BLACKSMITH" : "ALCHEMIST");
                }
                if (s->selected == i) {
                    renderer_draw_text(r, ">", cx - 200, item_y, gold, r->font_small);
                    renderer_draw_text(r, label, cx - 180, item_y,
                        accepted ? green : red, r->font_small);
                } else {
                    renderer_draw_text(r, label, cx - 180, item_y,
                        accepted ? white : dimmed, r->font_small);
                }
            }
        }
    }

    draw_shop_scrollbar(r, cx + 225, 140, visible_rows * 32 - 8,
        list_start, list_count, visible_rows);

    int detail_count = s->mode == 0 ? s->item_count : g->inventory_count;
    if (s->selected >= 0 && s->selected < detail_count) {
        const Item *selected = s->mode == 0
            ? &s->items[s->selected] : &g->inventory[s->selected];
        Item priced_selected = *selected;
        priced_selected.value = s->mode == 0
            ? shop_buy_price(selected)
            : (shop_accepts_item(s->type, selected) ? shop_sell_price(selected) : 0);
        selected = &priced_selected;
        if (selected->type == ITEM_WEAPON) {
            const Item *equipped = NULL;
            if (g->equipped_main_hand >= 0 &&
                g->equipped_main_hand < g->inventory_count) {
                equipped = &g->inventory[g->equipped_main_hand];
            }
            draw_weapon_comparison(r, g, selected, equipped,
                (r->tiles_y - 9) * TILE_SIZE);
        } else if (selected->type == ITEM_ARMOR) {
            const Item *equipped = NULL;
            if (g->equipped_armor >= 0 &&
                g->equipped_armor < g->inventory_count) {
                equipped = &g->inventory[g->equipped_armor];
            }
            draw_armor_comparison(r, g, selected, equipped,
                (r->tiles_y - 9) * TILE_SIZE);
        } else if (selected->type == ITEM_SHIELD) {
            const Item *equipped = NULL;
            if (g->equipped_off_hand >= 0 &&
                g->equipped_off_hand < g->inventory_count &&
                g->inventory[g->equipped_off_hand].type == ITEM_SHIELD) {
                equipped = &g->inventory[g->equipped_off_hand];
            }
            draw_shield_comparison(r, g, selected, equipped,
                (r->tiles_y - 9) * TILE_SIZE);
        }
    }

    renderer_draw_text(r, "TAB BUY/SELL   ENTER SELECT   ESC CLOSE",
        cx - 200, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}
