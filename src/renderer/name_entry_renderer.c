#include "name_entry_renderer.h"
#include "sprites.h"

void name_entry_draw(Renderer *r, const NameEntry *n) {
    int cx = r->tiles_x / 2;
    int cy = r->tiles_y / 2;

    // Background and border
    for (int y = 0; y < r->tiles_y; y++)
        for (int x = 0; x < r->tiles_x; x++)
            draw_floor(r, x, y);

    for (int x = 0; x < r->tiles_x; x++) {
        draw_wall(r, x, 0);
        draw_wall(r, x, r->tiles_y - 1);
    }
    for (int y = 0; y < r->tiles_y; y++) {
        draw_wall(r, 0, y);
        draw_wall(r, r->tiles_x - 1, y);
    }

    // Prompt bar
    for (int x = cx - 10; x < cx + 10; x++)
        draw_wall(r, x, cy - 3);

    // Name input box — one tile per character slot
    for (int i = 0; i < MAX_NAME_LEN; i++) {
        int tx = cx - MAX_NAME_LEN / 2 + i;
        SDL_Rect slot = {
            tx * TILE_SIZE + 1, cy * TILE_SIZE + 1,
            TILE_SIZE - 2,      TILE_SIZE - 2
        };
        // Filled slot if character entered, outline if empty
        if (i < n->length) {
            SDL_SetRenderDrawColor(r->sdl, 80, 160, 80, 255);
        } else if (i == n->length) {
            // Cursor position
            SDL_SetRenderDrawColor(r->sdl, 220, 180, 60, 255);
        } else {
            SDL_SetRenderDrawColor(r->sdl, 30, 40, 30, 255);
        }
        SDL_RenderFillRect(r->sdl, &slot);
    }

    // Confirm hint bar
    for (int x = cx - 10; x < cx + 10; x++)
        draw_wall(r, x, cy + 3);
}