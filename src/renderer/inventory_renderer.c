#include "inventory_renderer.h"
#include "equipment_compare_renderer.h"
#include "item_icons.h"
#include "sprites.h"

void inventory_draw(Renderer *r, const GameState *g, const InventoryScreen *s) {
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

    SDL_Color gold   = {220, 180,  60, 255};
    SDL_Color dimmed = { 80,  80,  80, 255};
    SDL_Color hint = {105, 150, 105, 255};
    SDL_Color white  = {200, 200, 200, 255};

    int cx = r->screen_w / 2;
    renderer_draw_text(r, "INVENTORY", cx - 60, 35, gold, r->font_large);
    char summary[64];
    SDL_snprintf(summary, sizeof(summary), "GOLD: %d   BAG: %d/%d", g->gold, g->inventory_count, MAX_INVENTORY);
    renderer_draw_text(r, summary, cx - 150, 70, gold, r->font_small);
    renderer_draw_text(r, "BAG", cx - 155, 100, s->equipment_tab ? dimmed : gold, r->font_small);
    renderer_draw_text(r, "EQUIPMENT", cx - 30, 100, s->equipment_tab ? gold : dimmed, r->font_small);
    renderer_draw_text(r, "TAB SWITCH | 1/2 ACCESSORY", cx + 90, 102, hint, r->font_tiny);

    int count = s->equipment_tab ? EQUIPMENT_SLOT_COUNT : g->inventory_count;
    if (!count) {
        renderer_draw_text(r, "BAG EMPTY", cx - 50, 165, dimmed, r->font_small);
    } else {
        int visible_rows = inventory_visible_rows(r->tiles_y);
        int list_start = inventory_list_start(s->selected, count, visible_rows);
        int list_end = list_start + visible_rows;
        if (list_end > count) {
            list_end = count;
        }
        for (int i = list_start; i < list_end; i++) {
            int index = i + (s->equipment_tab ? MAX_INVENTORY : 0);
            const Item *item = &g->inventory[index];
            int item_y = 130 + (i - list_start) * 36;
            draw_icon_empty_slot(r, cx - 314, item_y - 5);
            if (item->active) {
                draw_icon_item(r, cx - 314, item_y - 5, item);
            }
            if (s->equipment_tab) {
                const Item *strong = game_equipped_item(g, EQUIPMENT_STRONG_HAND);
                int disabled = index == EQUIPMENT_WEAK_HAND && strong && strong->weapon_hands == WEAPON_HANDS_TWO;
                renderer_draw_text(r, s->selected == i ? ">" : "", cx - 280, item_y, gold, r->font_small);
                renderer_draw_text(r, game_equipment_slot_name(index), cx - 260, item_y, hint, r->font_tiny);
                renderer_draw_text(r, disabled ? "DISABLED (2H)" : item->active ? item->name : "EMPTY", cx - 90, item_y, item->active ? white : dimmed, r->font_small);
                continue;
            }

            char detail[32] = "";
            if (item->type == ITEM_POTION_HEALTH) {
                SDL_snprintf(detail, sizeof(detail), "FULL HP");
            } else if (item->type == ITEM_POTION_MANA) {
                SDL_snprintf(detail, sizeof(detail), "FULL MP");
            } else if (item->type == ITEM_POTION_STRENGTH) {
                SDL_snprintf(detail, sizeof(detail), "+1 BASE ATK");
            } else if (item->type == ITEM_POTION_INTELLIGENCE) {
                SDL_snprintf(detail, sizeof(detail), "+1 MAX MP");
            } else if (item->type == ITEM_WEAPON) {
                SDL_snprintf(detail, sizeof(detail), item->sharpened ? "+%d ATK (SHARPENED)" : "+%d ATK",
                    item->attack_bonus);
            } else if (item->type == ITEM_ARMOR ||
                item->type == ITEM_SHIELD || item->type == ITEM_HELMET || item->type == ITEM_ACCESSORY) {
                SDL_snprintf(detail, sizeof(detail), "+%d ATK +%d DEF", item->attack_bonus, item->defense_bonus);
            } else if (item->type == ITEM_SPELL_TOME) {
                SDL_snprintf(detail, sizeof(detail), "MAGE UPGRADE");
            } else if (item->type == ITEM_TREASURE_MAP) {
                SDL_snprintf(detail, sizeof(detail), "KEY ITEM");
            }

            SDL_Color color = white;
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
        }
    }

    int selected_index = inventory_selected_index(s);
    if (s->selected >= 0 && s->selected < count && g->inventory[selected_index].active) {
        const Item *selected = &g->inventory[selected_index];
        if (selected->type == ITEM_WEAPON) {
            const Item *equipped = NULL;
            if (g->equipped_main_hand >= 0 &&
                g->equipped_main_hand < MAX_CARRIED_ITEMS) {
                equipped = &g->inventory[g->equipped_main_hand];
            }
            draw_weapon_comparison(r, g, selected, equipped,
                (r->tiles_y - 9) * TILE_SIZE);
        } else if (selected->type == ITEM_ARMOR || selected->type == ITEM_HELMET || selected->type == ITEM_ACCESSORY) {
            EquipmentSlot slot = selected->type == ITEM_ARMOR ? EQUIPMENT_ARMOR : selected->type == ITEM_HELMET ? EQUIPMENT_HELMET : EQUIPMENT_ACCESSORY_1;
            const Item *equipped = game_equipped_item(g, s->equipment_tab ? selected_index : slot);
            draw_armor_comparison(r, g, selected, equipped,
                (r->tiles_y - 9) * TILE_SIZE);
        } else if (selected->type == ITEM_SHIELD) {
            const Item *equipped = NULL;
            if (g->equipped_off_hand >= 0 &&
                g->equipped_off_hand < MAX_CARRIED_ITEMS &&
                g->inventory[g->equipped_off_hand].type == ITEM_SHIELD) {
                equipped = &g->inventory[g->equipped_off_hand];
            }
            draw_shield_comparison(r, g, selected, equipped,
                (r->tiles_y - 9) * TILE_SIZE);
        }
    }

    if (g->message_count > 0) {
        renderer_draw_text(r, g->messages[g->message_count - 1], cx - 300, (r->tiles_y - 3) * TILE_SIZE + 10, white, r->font_tiny);
    }
    const char *buttons[] = {"U USE", s->equipment_tab ? "E/ENTER UNEQUIP" : "E/ENTER EQUIP", "O OFF-HAND", "X UNEQUIP (GEAR)", "D DROP", "ESC CLOSE"};
    for (int i = 0; i < 6; i++) {
        renderer_draw_text(r, buttons[i], cx - 270 + (i % 3) * 180,
            (r->tiles_y - 2) * TILE_SIZE + (i / 3) * 16, hint, r->font_tiny);
    }
}
