#include "item_icons.h"

static void fill_rect_px(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

static void draw_icon_rusty_sword(Renderer *r, int px, int py) {
    SDL_Color rust = {139, 72, 43, 255};
    SDL_Color rust_hi = {188, 103, 57, 255};
    SDL_Color guard = {92, 67, 47, 255};
    SDL_Color grip = {73, 44, 29, 255};
    fill_rect_px(r, px + 15, py + 3, 3, 3, rust_hi);
    fill_rect_px(r, px + 12, py + 6, 4, 4, rust);
    fill_rect_px(r, px + 9, py + 9, 4, 4, rust_hi);
    fill_rect_px(r, px + 6, py + 12, 4, 3, rust);
    fill_rect_px(r, px + 4, py + 11, 7, 3, guard);
    fill_rect_px(r, px + 5, py + 14, 3, 6, grip);
}

static void draw_icon_short_sword(Renderer *r, int px, int py) {
    SDL_Color blade = {191, 200, 210, 255};
    SDL_Color edge = {235, 240, 242, 255};
    SDL_Color guard = {196, 153, 48, 255};
    SDL_Color grip = {91, 54, 34, 255};
    fill_rect_px(r, px + 14, py + 5, 4, 4, edge);
    fill_rect_px(r, px + 11, py + 8, 4, 4, blade);
    fill_rect_px(r, px + 8, py + 11, 4, 4, edge);
    fill_rect_px(r, px + 5, py + 12, 8, 3, guard);
    fill_rect_px(r, px + 5, py + 15, 3, 5, grip);
}

static void draw_icon_long_sword(Renderer *r, int px, int py) {
    SDL_Color blade = {155, 180, 207, 255};
    SDL_Color edge = {225, 235, 244, 255};
    SDL_Color guard = {208, 170, 62, 255};
    SDL_Color grip = {73, 47, 42, 255};
    fill_rect_px(r, px + 18, py + 2, 3, 3, edge);
    fill_rect_px(r, px + 15, py + 5, 4, 4, blade);
    fill_rect_px(r, px + 12, py + 8, 4, 4, edge);
    fill_rect_px(r, px + 9, py + 11, 4, 4, blade);
    fill_rect_px(r, px + 6, py + 14, 4, 3, edge);
    fill_rect_px(r, px + 4, py + 12, 8, 3, guard);
    fill_rect_px(r, px + 4, py + 16, 3, 6, grip);
    fill_rect_px(r, px + 2, py + 20, 5, 2, guard);
}

static void draw_icon_battle_axe(Renderer *r, int px, int py) {
    SDL_Color handle = {104, 61, 33, 255};
    SDL_Color handle_hi = {157, 94, 45, 255};
    SDL_Color iron = {145, 154, 163, 255};
    SDL_Color edge = {218, 225, 228, 255};
    fill_rect_px(r, px + 5, py + 16, 3, 6, handle);
    fill_rect_px(r, px + 7, py + 12, 3, 6, handle_hi);
    fill_rect_px(r, px + 9, py + 8, 3, 6, handle);
    fill_rect_px(r, px + 11, py + 4, 3, 7, handle_hi);
    fill_rect_px(r, px + 13, py + 3, 7, 4, iron);
    fill_rect_px(r, px + 16, py + 6, 6, 5, iron);
    fill_rect_px(r, px + 19, py + 8, 3, 5, edge);
    fill_rect_px(r, px + 11, py + 2, 4, 8, iron);
}

static void draw_icon_staff(Renderer *r, int px, int py) {
    SDL_Color wood = {105, 66, 38, 255};
    SDL_Color wood_hi = {164, 105, 51, 255};
    SDL_Color gem = {104, 178, 214, 255};
    SDL_Color glow = {190, 230, 242, 255};
    fill_rect_px(r, px + 5, py + 17, 3, 5, wood);
    fill_rect_px(r, px + 7, py + 13, 3, 6, wood_hi);
    fill_rect_px(r, px + 9, py + 9, 3, 6, wood);
    fill_rect_px(r, px + 11, py + 5, 3, 6, wood_hi);
    fill_rect_px(r, px + 13, py + 2, 7, 3, wood);
    fill_rect_px(r, px + 17, py + 4, 4, 5, gem);
    fill_rect_px(r, px + 18, py + 4, 2, 2, glow);
}

static void draw_icon_bow(Renderer *r, int px, int py) {
    SDL_Color wood = {148, 91, 43, 255};
    SDL_Color wood_hi = {205, 142, 65, 255};
    SDL_Color string = {205, 206, 190, 255};
    SDL_Color arrow = {173, 181, 184, 255};
    fill_rect_px(r, px + 6, py + 3, 4, 4, wood_hi);
    fill_rect_px(r, px + 4, py + 6, 4, 5, wood);
    fill_rect_px(r, px + 3, py + 10, 4, 5, wood_hi);
    fill_rect_px(r, px + 5, py + 15, 4, 5, wood);
    fill_rect_px(r, px + 8, py + 19, 4, 3, wood_hi);
    fill_rect_px(r, px + 9, py + 4, 1, 16, string);
    fill_rect_px(r, px + 10, py + 11, 10, 2, arrow);
    fill_rect_px(r, px + 18, py + 9, 3, 6, arrow);
}

static void draw_icon_generic_weapon(Renderer *r, int px, int py) {
    SDL_Color blade = {200, 200, 220, 255};
    SDL_Color guard = {220, 180, 40, 255};
    SDL_Color handle = {120, 80, 40, 255};
    fill_rect_px(r, px + 14, py + 2, 4, 4, blade);
    fill_rect_px(r, px + 10, py + 6, 4, 4, blade);
    fill_rect_px(r, px + 6, py + 10, 4, 4, blade);
    fill_rect_px(r, px + 4, py + 8, 6, 3, guard);
    fill_rect_px(r, px + 8, py + 4, 3, 6, guard);
    fill_rect_px(r, px + 2, py + 14, 4, 4, handle);
    fill_rect_px(r, px + 4, py + 16, 3, 5, handle);
}

void draw_icon_weapon(Renderer *r, int px, int py, const Item *item) {
    switch (item->visual_id) {
        case ITEM_VISUAL_RUSTY_SWORD:
            draw_icon_rusty_sword(r, px, py);
            break;
        case ITEM_VISUAL_SHORT_SWORD:
            draw_icon_short_sword(r, px, py);
            break;
        case ITEM_VISUAL_LONG_SWORD:
            draw_icon_long_sword(r, px, py);
            break;
        case ITEM_VISUAL_BATTLE_AXE:
            draw_icon_battle_axe(r, px, py);
            break;
        case ITEM_VISUAL_STAFF:
            draw_icon_staff(r, px, py);
            break;
        case ITEM_VISUAL_BOW:
            draw_icon_bow(r, px, py);
            break;
        default:
            draw_icon_generic_weapon(r, px, py);
            break;
    }
}

void draw_icon_armor(Renderer *r, int px, int py) {
    SDL_Color metal  = {100, 120, 140, 255};
    SDL_Color dark   = { 60,  80, 100, 255};
    // Chest plate
    fill_rect_px(r, px+4,  py+4,  16, 14, metal);
    fill_rect_px(r, px+6,  py+6,  12, 10, dark);
    // Shoulders
    fill_rect_px(r, px+2,  py+4,  4,  8, metal);
    fill_rect_px(r, px+18, py+4,  4,  8, metal);
    // Center line
    fill_rect_px(r, px+11, py+4,  2, 14, dark);
}

void draw_icon_spell(Renderer *r, int px, int py, SpellID spell_id) {
    if (spell_id == SPELL_MAGIC_ARROW) {
        SDL_Color glow = {80, 220, 255, 255};
        SDL_Color core = {220, 250, 255, 255};
        fill_rect_px(r, px + 4, py + 15, 4, 4, glow);
        fill_rect_px(r, px + 7, py + 12, 4, 4, glow);
        fill_rect_px(r, px + 10, py + 9, 4, 4, core);
        fill_rect_px(r, px + 13, py + 6, 4, 4, glow);
        fill_rect_px(r, px + 16, py + 3, 4, 4, core);
        fill_rect_px(r, px + 15, py + 2, 6, 2, glow);
        fill_rect_px(r, px + 19, py + 2, 2, 6, glow);
    } else if (spell_id == SPELL_FIREBALL) {
        SDL_Color outer = {220, 55, 20, 255};
        SDL_Color flame = {255, 135, 25, 255};
        SDL_Color core = {255, 230, 90, 255};
        fill_rect_px(r, px + 5, py + 7, 14, 13, outer);
        fill_rect_px(r, px + 8, py + 5, 9, 16, flame);
        fill_rect_px(r, px + 10, py + 9, 6, 8, core);
        fill_rect_px(r, px + 12, py + 2, 4, 5, flame);
    } else if (spell_id == SPELL_HEAL) {
        SDL_Color glow = {60, 220, 110, 255};
        SDL_Color core = {190, 255, 205, 255};
        fill_rect_px(r, px + 9, py + 3, 7, 18, glow);
        fill_rect_px(r, px + 4, py + 8, 17, 8, glow);
        fill_rect_px(r, px + 11, py + 5, 3, 14, core);
        fill_rect_px(r, px + 6, py + 10, 13, 3, core);
    } else {
        SDL_Color outer = {140, 75, 230, 255};
        SDL_Color inner = {80, 220, 235, 255};
        fill_rect_px(r, px + 6, py + 3, 12, 3, outer);
        fill_rect_px(r, px + 3, py + 6, 3, 12, outer);
        fill_rect_px(r, px + 18, py + 6, 3, 12, outer);
        fill_rect_px(r, px + 6, py + 18, 12, 3, outer);
        fill_rect_px(r, px + 8, py + 6, 8, 3, inner);
        fill_rect_px(r, px + 6, py + 9, 3, 7, inner);
        fill_rect_px(r, px + 15, py + 9, 3, 7, inner);
        fill_rect_px(r, px + 8, py + 16, 8, 3, inner);
    }
}

void draw_icon_empty_slot(Renderer *r, int px, int py) {
    SDL_Color border = {40, 40, 60, 255};
    SDL_Color bg     = {18, 18, 35, 255};
    fill_rect_px(r, px,   py,   ICON_SIZE,   ICON_SIZE,   bg);
    fill_rect_px(r, px,   py,   ICON_SIZE,   1,           border);
    fill_rect_px(r, px,   py,   1,           ICON_SIZE,   border);
    fill_rect_px(r, px,   py+ICON_SIZE-1, ICON_SIZE, 1,   border);
    fill_rect_px(r, px+ICON_SIZE-1, py, 1, ICON_SIZE,     border);
}
