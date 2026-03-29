#include "renderer.h"
#include <stdio.h>

#define FONT_PATH "assets/PressStart2P-Regular.ttf"

void renderer_init(Renderer *r, SDL_Renderer *sdl, int screen_w, int screen_h) {
    r->sdl      = sdl;
    r->screen_w = screen_w;
    r->screen_h = screen_h;
    r->tiles_x  = screen_w / TILE_SIZE;
    r->tiles_y  = screen_h / TILE_SIZE;

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        r->font_large = NULL;
        r->font_small = NULL;
        return;
    }

    r->font_large = TTF_OpenFont(FONT_PATH, 16);
    r->font_small = TTF_OpenFont(FONT_PATH, 10);

    if (!r->font_large || !r->font_small)
        fprintf(stderr, "TTF_OpenFont error: %s\n", TTF_GetError());
}

void renderer_free(Renderer *r) {
    if (r->font_large) TTF_CloseFont(r->font_large);
    if (r->font_small) TTF_CloseFont(r->font_small);
    TTF_Quit();
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
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TILE_SIZE, TILE_SIZE
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

void renderer_draw_text(Renderer *r, const char *text, int x, int y,
                        SDL_Color color, TTF_Font *font) {
    if (!font) return;
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(r->sdl, surface);
    if (texture) {
        SDL_Rect dst = { x, y, surface->w, surface->h };
        SDL_RenderCopy(r->sdl, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}