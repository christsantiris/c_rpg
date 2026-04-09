#include "help_renderer.h"
#include "sprites.h"

void help_draw(Renderer *r) {
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
    SDL_Color white = {200, 200, 200, 255};
    SDL_Color dim = {120, 120, 120, 255};
    SDL_Color hint = {50, 70, 50, 255};

    int cx = r->screen_w / 2;
    int col1 = cx - 280;
    int col2 = cx + 20;
    int y = 40;
    int lh = 24;

    renderer_draw_text(r, "CONTROLS", cx - 50, y, gold, r->font_large);
    y += lh + 10;

    renderer_draw_text(r, "MOVEMENT",       col1, y, gold,  r->font_small);
    renderer_draw_text(r, "COMBAT",         col2, y, gold,  r->font_small);
    y += lh;
    renderer_draw_text(r, "WASD / ARROWS  Move",     col1, y, white, r->font_tiny);
    renderer_draw_text(r, "Bump enemy     Melee attack", col2, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, ".              Descend stairs", col1, y, white, r->font_tiny);
    renderer_draw_text(r, "C              Cast spell",    col2, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, ",              Ascend stairs", col1, y, white, r->font_tiny);
    renderer_draw_text(r, "F              Fire ranged",   col2, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, "T              Return to town", col1, y, white, r->font_tiny);
    y += lh + 10;

    renderer_draw_text(r, "INVENTORY",      col1, y, gold,  r->font_small);
    renderer_draw_text(r, "WORLD",          col2, y, gold,  r->font_small);
    y += lh;
    renderer_draw_text(r, "I              Open inventory", col1, y, white, r->font_tiny);
    renderer_draw_text(r, "E              Enter shop",     col2, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, "P              Pick up item",   col1, y, white, r->font_tiny);
    renderer_draw_text(r, "ESC            Main menu",      col2, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, "U              Use item",       col1, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, "E              Equip item",     col1, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, "D              Drop item",      col1, y, white, r->font_tiny);
    y += lh + 10;

    renderer_draw_text(r, "MAGIC",          col1, y, gold,  r->font_small);
    y += lh;
    renderer_draw_text(r, "B              Open spellbook", col1, y, white, r->font_tiny);
    y += lh;
    renderer_draw_text(r, "C              Cast equipped spell", col1, y, white, r->font_tiny);
    y += lh + 10;

    renderer_draw_text(r, "Press ? or ESC to close", cx - 120, (r->tiles_y - 2) * TILE_SIZE, hint, r->font_small);
}