#include "landing_renderer.h"
#include "../renderer/sprites.h"

void landing_draw(Renderer *r, const LandingScreen *s) {
    int cx = r->tiles_x / 2;
    int cy = r->tiles_y / 2;

    // Background
    for (int y = 0; y < r->tiles_y; y++)
        for (int x = 0; x < r->tiles_x; x++)
            draw_floor(r, x, y);

    // Border
    for (int x = 0; x < r->tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, r->tiles_x - 1, y);
    }

    // Title bar above menu
    for (int x = cx - 10; x < cx + 10; x++)
        draw_wall(r, x, cy - 4);

    // Menu items as colored tile blocks
    SDL_Color gold   = {220, 180,  60, 255};
    SDL_Color green  = { 80, 160,  80, 255};
    SDL_Color dimmed = { 40,  60,  40, 255};

    const SDL_Color colors[3] = { green, green, dimmed };

    for (int i = 0; i < 3; i++) {
        int item_y = cy + i * 2;
        SDL_Color c = (s->selected == i) ? gold : colors[i];

        // Cursor block
        if (s->selected == i) {
            SDL_Rect cursor = {
                (cx - 6) * TILE_SIZE, item_y * TILE_SIZE,
                TILE_SIZE, TILE_SIZE
            };
            SDL_SetRenderDrawColor(r->sdl, gold.r, gold.g, gold.b, 255);
            SDL_RenderFillRect(r->sdl, &cursor);
        }

        // Item highlight block
        SDL_Rect block = {
            (cx - 4) * TILE_SIZE, item_y * TILE_SIZE,
            8 * TILE_SIZE, TILE_SIZE
        };
        SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, 60);
        SDL_RenderFillRect(r->sdl, &block);
    }
}