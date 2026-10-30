#include "wilds_renderer.h"
#include "sprites.h"
#include "controls_renderer.h"
#include <stdlib.h>
#include <stdio.h>

void draw_boulder(Renderer *r, int sx, int sy) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_SetRenderDrawColor(r->sdl, 24, 29, 35, 255);
    SDL_Rect shadow = {px + 3, py + 17, 19, 5};
    SDL_RenderFillRect(r->sdl, &shadow);
    static const int widths[8] = {8, 12, 16, 18, 18, 18, 14, 10};
    for (int row = 0; row < 8; row++) {
        SDL_SetRenderDrawColor(r->sdl, 130 - row * 8, 142 - row * 8, 150 - row * 8, 255);
        SDL_Rect rock = {px + (TILE_SIZE - widths[row]) / 2, py + 4 + row * 2, widths[row], 2};
        SDL_RenderFillRect(r->sdl, &rock);
    }
    SDL_SetRenderDrawColor(r->sdl, 171, 185, 190, 255);
    SDL_RenderDrawLine(r->sdl, px + 8, py + 7, px + 13, py + 5);
    SDL_SetRenderDrawColor(r->sdl, 65, 77, 86, 255);
    SDL_RenderDrawLine(r->sdl, px + 15, py + 9, px + 11, py + 14);
}

void draw_wilds_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    unsigned int noise = (unsigned int)x * 73856093u ^ (unsigned int)y * 19349663u;
    if (tile == TILE_SWAMP_WATER) {
        // Keep deep bog impassable while restoring its dense, swaying vegetation.
        draw_swamp_wall(r, sx, sy, x, y);
        return;
    }
    if (tile == TILE_SWAMP_REEDS) {
        draw_swamp_floor(r, sx, sy, x, y);
        int lean = (SDL_GetTicks() / 500 + noise % 3) % 3 - 1;
        for (int i = 0; i < 5; i++) {
            int rx = px + 3 + i * 4;
            int height = 10 + (noise >> (i * 3)) % 7;
            SDL_SetRenderDrawColor(r->sdl, 67, 106, 49, 255);
            SDL_RenderDrawLine(r->sdl, rx, py + 22, rx + lean, py + 22 - height);
            SDL_SetRenderDrawColor(r->sdl, 118, 126, 63, 255);
            SDL_Rect head = {rx - 1 + lean, py + 20 - height, 3, 5};
            SDL_RenderFillRect(r->sdl, &head);
        }
        return;
    }
    if (tile == TILE_DRAGON_RAVINE) {
        draw_dragonspine_wall(r, &g->map, sx, sy, x, y);
        return;
    }
    draw_dragonspine_floor(r, sx, sy, x, y, tile == TILE_DRAGON_ASH_BOULDER ? 1 : 0);
    draw_boulder(r, sx, sy);
    if (abs(g->player.x - x) + abs(g->player.y - y) == 1) {
        char hint[48];
        snprintf(hint, sizeof(hint), "%s: PUSH BOULDER", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]));
        renderer_draw_text(r, hint, px - 35, py - 15, (SDL_Color){204, 221, 230, 255}, r->font_tiny);
    }
}
