#include "item_icons.h"

static void fill_rect_px(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void draw_icon_weapon(Renderer *r, int px, int py) {
    SDL_Color blade  = {200, 200, 220, 255};
    SDL_Color guard  = {220, 180,  40, 255};
    SDL_Color handle = {120,  80,  40, 255};
    // Blade (diagonal top-right to center)
    fill_rect_px(r, px+14, py+2,  4, 4, blade);
    fill_rect_px(r, px+10, py+6,  4, 4, blade);
    fill_rect_px(r, px+6,  py+10, 4, 4, blade);
    // Guard
    fill_rect_px(r, px+4,  py+8,  6, 3, guard);
    fill_rect_px(r, px+8,  py+4,  3, 6, guard);
    // Handle
    fill_rect_px(r, px+2,  py+14, 4, 4, handle);
    fill_rect_px(r, px+4,  py+16, 3, 5, handle);
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
