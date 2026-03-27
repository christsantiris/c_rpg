#include "sprites.h"

static void fill_rect(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void draw_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = {18, 18, 35, 255};
    SDL_Color dot  = {30, 30, 55, 255};
    fill_rect(r, x,            y,            TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + TILE_SIZE/2, y + TILE_SIZE/2, 2, 2, dot);
}

void draw_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color face  = {50, 45, 80, 255};
    SDL_Color top   = {70, 65, 105, 255};
    SDL_Color seam  = {35, 30, 60, 255};
    fill_rect(r, x,     y,     TILE_SIZE,     TILE_SIZE,     face);
    fill_rect(r, x,     y,     TILE_SIZE,     3,             top);
    fill_rect(r, x,     y,     2,             TILE_SIZE,     top);
    fill_rect(r, x + 2, y + 8, TILE_SIZE - 4, 1,             seam);
}

void draw_player(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body  = {60,  100, 220, 255};
    SDL_Color head  = {200, 160, 120, 255};
    SDL_Color sword = {190, 190, 210, 255};

    // body
    fill_rect(r, x + 7,  y + 11, 10, 10, body);
    // head
    fill_rect(r, x + 8,  y + 4,  8,  7,  head);
    // sword
    fill_rect(r, x + 17, y + 7,  2,  9,  sword);
}

void draw_stairs_up(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = {18,  18,  35,  255};
    SDL_Color step   = {180, 140, 60,  255};
    SDL_Color edge   = {220, 180, 80,  255};
    fill_rect(r, x,         y,         TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3,     y + 14,    18,        3,         step);
    fill_rect(r, x + 6,     y + 10,    12,        3,         step);
    fill_rect(r, x + 9,     y + 6,     6,         3,         step);
    fill_rect(r, x + 3,     y + 14,    18,        1,         edge);
    fill_rect(r, x + 6,     y + 10,    12,        1,         edge);
    fill_rect(r, x + 9,     y + 6,     6,         1,         edge);
}

void draw_stairs_down(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = {18,  18,  35,  255};
    SDL_Color step   = {100, 80,  160, 255};
    SDL_Color edge   = {140, 110, 200, 255};
    fill_rect(r, x,         y,         TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3,     y + 4,     18,        3,         step);
    fill_rect(r, x + 6,     y + 8,     12,        3,         step);
    fill_rect(r, x + 9,     y + 12,    6,         3,         step);
    fill_rect(r, x + 3,     y + 4,     18,        1,         edge);
    fill_rect(r, x + 6,     y + 8,     12,        1,         edge);
    fill_rect(r, x + 9,     y + 12,    6,         1,         edge);
}