#include "renderer.h"
#include "info_panel.h"
#include <stdio.h>
#include "message_bar.h"

#define FONT_PATH "assets/PressStart2P-Regular.ttf"

static SDL_Texture *load_sprite_texture(SDL_Renderer *sdl, const char *path) {
    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface) {
        fprintf(stderr, "Sprite texture error for %s: %s\n", path,
            SDL_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdl, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        fprintf(stderr, "Sprite texture error for %s: %s\n", path,
            SDL_GetError());
        return NULL;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
    return texture;
}

void renderer_init(Renderer *r, SDL_Renderer *sdl, int screen_w, int screen_h) {
    r->sdl      = sdl;
    r->screen_w = screen_w;
    r->screen_h = screen_h;
    r->tiles_x  = screen_w / TILE_SIZE;
    r->tiles_y  = (screen_h - MESSAGE_BAR_H) / TILE_SIZE;

    r->blacksmith_texture = load_sprite_texture(sdl, "assets/blacksmith.bmp");
    r->alchemist_texture = load_sprite_texture(sdl, "assets/alchemist.bmp");
    r->tavern_texture = load_sprite_texture(sdl, "assets/tavern.bmp");
    r->harbor_texture = load_sprite_texture(sdl, "assets/harbor.bmp");
    r->healer_texture = load_sprite_texture(sdl, "assets/healer.bmp");
    r->island_texture = load_sprite_texture(sdl, "assets/island-sprites.bmp");

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        r->font_large = NULL;
        r->font_small = NULL;
        r->font_tiny  = NULL;   
        return;
    }

    r->font_large = TTF_OpenFont(FONT_PATH, 16);
    r->font_small = TTF_OpenFont(FONT_PATH, 10);
    r->font_tiny  = TTF_OpenFont(FONT_PATH, 8);

    if (!r->font_large || !r->font_small || !r->font_tiny)
        fprintf(stderr, "TTF_OpenFont error: %s\n", TTF_GetError());
}

void renderer_free(Renderer *r) {
    if (r->island_texture) {
        SDL_DestroyTexture(r->island_texture);
        r->island_texture = NULL;
    }
    if (r->healer_texture) {
        SDL_DestroyTexture(r->healer_texture);
        r->healer_texture = NULL;
    }
    if (r->blacksmith_texture) {
        SDL_DestroyTexture(r->blacksmith_texture);
        r->blacksmith_texture = NULL;
    }
    if (r->alchemist_texture) {
        SDL_DestroyTexture(r->alchemist_texture);
        r->alchemist_texture = NULL;
    }
    if (r->tavern_texture) {
        SDL_DestroyTexture(r->tavern_texture);
        r->tavern_texture = NULL;
    }
    if (r->harbor_texture) {
        SDL_DestroyTexture(r->harbor_texture);
        r->harbor_texture = NULL;
    }
    if (r->font_large) TTF_CloseFont(r->font_large);
    if (r->font_small) TTF_CloseFont(r->font_small);
    if (r->font_tiny) TTF_CloseFont(r->font_tiny);
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
    r->tiles_y  = (new_h - MESSAGE_BAR_H) / TILE_SIZE;
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
