#include "inventory_renderer.h"
#include "equipment_compare_renderer.h"
#include "sprites.h"

static int inventory_visible_rows(const Renderer *r) {
    int list_top = 130;
    int comparison_top = (r->tiles_y - 9) * TILE_SIZE;
    int rows = (comparison_top - list_top - 12) / 36;
    if (rows < 1) {
        return 1;
    }
    return rows;
}

static int inventory_list_start(int selected, int count, int visible_rows) {
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
        int visible_rows = inventory_visible_rows(r);
        int list_start = inventory_list_start(s->selected,
            g->inventory_count, visible_rows);
        int list_end = list_start + visible_rows;
        if (list_end > g->inventory_count) {
            list_end = g->inventory_count;
        }
        if (g->inventory_count > visible_rows) {
            char range[32];
            SDL_snprintf(range, sizeof(range), "%d-%d OF %d",
                list_start + 1, list_end, g->inventory_count);
            renderer_draw_text(r, range, cx + 90, 90, dimmed,
                r->font_tiny);
        }

        for (int i = list_start; i < list_end; i++) {
            const Item *item = &g->inventory[i];
            int item_y = 130 + (i - list_start) * 36;

            char detail[32] = "";
            if (item->type == ITEM_POTION_HEALTH) {
                SDL_snprintf(detail, sizeof(detail), "+%d HP", item->heal_hp);
            } else if (item->type == ITEM_POTION_MANA) {
                SDL_snprintf(detail, sizeof(detail), "+%d MP", item->heal_mp);
            } else if (item->type == ITEM_WEAPON) {
                SDL_snprintf(detail, sizeof(detail), "+%d ATK",
                    item->attack_bonus);
            } else if (item->type == ITEM_ARMOR ||
                item->type == ITEM_SHIELD) {
                SDL_snprintf(detail, sizeof(detail), "+%d DEF",
                    item->defense_bonus);
            } else if (item->type == ITEM_SPELL_TOME) {
                SDL_snprintf(detail, sizeof(detail), "MAGE UPGRADE");
            }

            // Equipped indicator
            int is_equipped =
                (item->type == ITEM_WEAPON &&
                    (g->equipped_main_hand == i ||
                    g->equipped_off_hand == i)) ||
                (item->type == ITEM_ARMOR && g->equipped_armor == i) ||
                (item->type == ITEM_SHIELD && g->equipped_off_hand == i);
            SDL_Color color = is_equipped ? gold : white;
            const char *slot = "";
            if (g->equipped_main_hand == i) {
                slot = "[M]";
            } else if (g->equipped_off_hand == i) {
                slot = "[O]";
            } else if (g->equipped_armor == i) {
                slot = "[A]";
            }

            if (s->selected == i) {
                renderer_draw_text(r, ">", cx - 280, item_y, gold,
                    r->font_small);
            }
            renderer_draw_text(r, item->name, cx - 260, item_y, color,
                r->font_small);
            if (detail[0] != '\0') {
                renderer_draw_text(r, detail, cx + 80, item_y, color,
                    r->font_small);
            }
            if (is_equipped) {
                renderer_draw_text(r, slot, cx + 190, item_y, gold,
                    r->font_small);
            }
        }
    }

    if (s->selected >= 0 && s->selected < g->inventory_count) {
        const Item *selected = &g->inventory[s->selected];
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

    renderer_draw_text(r, "U USE  E MAIN  O OFF-HAND  D DROP  ESC CLOSE",
        cx - 245, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}
