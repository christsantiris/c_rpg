#ifndef RENDERER_HEADER_H
#define RENDERER_HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define TILE_SIZE 24

typedef struct {
    SDL_Renderer *sdl;
    SDL_Texture *blacksmith_texture;
    SDL_Texture *alchemist_texture;
    SDL_Texture *tavern_texture;
    SDL_Texture *harbor_texture;
    SDL_Texture *healer_texture;
    SDL_Texture *island_texture;
    TTF_Font     *font_large;
    TTF_Font     *font_small;
    TTF_Font     *font_tiny;
    int           screen_w;
    int           screen_h;
    int           tiles_x;
    int           tiles_y;
} Renderer;

void renderer_init(Renderer *r, SDL_Renderer *sdl, int screen_w, int screen_h);
void renderer_free(Renderer *r);
void renderer_begin_frame(Renderer *r);
void renderer_end_frame(Renderer *r);
void renderer_draw_tile_bg(Renderer *r, int tile_x, int tile_y, SDL_Color color);
void renderer_on_resize(Renderer *r, int new_w, int new_h);
void renderer_draw_text(Renderer *r, const char *text, int x, int y, SDL_Color color, TTF_Font *font);

#endif
