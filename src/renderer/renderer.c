#include "renderer.h"

void renderer_init(Renderer *r, SDL_Renderer *sdl, int screen_w, int screen_h) {
    r->sdl      = sdl;
    r->screen_w = screen_w;
    r->screen_h = screen_h;
    r->tiles_x  = screen_w / TILE_SIZE;
    r->tiles_y  = screen_h / TILE_SIZE;
}

void renderer_begin_frame(Renderer *r) {
    SDL_SetRenderDrawColor(r->sdl, 10, 10, 20, 255);
    SDL_RenderClear(r->sdl);
}

void renderer_end_frame(Renderer *r) {
    SDL_RenderPresent(r->sdl);
}

void renderer_draw_tile_bg(Renderer *r, int tile_x, int tile_y, SDL_Color color) {
    SDL_Rect rect = {
        tile_x * TILE_SIZE,
        tile_y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void renderer_on_resize(Renderer *r, int new_w, int new_h) {
    r->screen_w = new_w;
    r->screen_h = new_h;
    r->tiles_x  = new_w / TILE_SIZE;
    r->tiles_y  = new_h / TILE_SIZE;
}