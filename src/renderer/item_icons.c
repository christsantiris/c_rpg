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

void draw_icon_empty_slot(Renderer *r, int px, int py) {
    SDL_Color border = {40, 40, 60, 255};
    SDL_Color bg     = {18, 18, 35, 255};
    fill_rect_px(r, px,   py,   ICON_SIZE,   ICON_SIZE,   bg);
    fill_rect_px(r, px,   py,   ICON_SIZE,   1,           border);
    fill_rect_px(r, px,   py,   1,           ICON_SIZE,   border);
    fill_rect_px(r, px,   py+ICON_SIZE-1, ICON_SIZE, 1,   border);
    fill_rect_px(r, px+ICON_SIZE-1, py, 1, ICON_SIZE,     border);
}