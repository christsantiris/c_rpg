#include "town_road_renderer.h"
#include "../game/town_road.h"
#include "info_panel.h"
#include "sprites.h"
#include <stdio.h>

static void boundary_label(Renderer *r, const Viewport *v, TownSide side, const char *area) {
    int x;
    int y;
    town_road_exit(side, &x, &y);
    if (!viewport_is_visible(v, x, y)) {
        return;
    }
    char text[64];
    snprintf(text, sizeof(text), "%c: %s", "NESW"[side], area);
    int width;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE - 18;
    int right = r->screen_w - INFO_PANEL_W - width - 8;
    if (px < 8) {
        px = 8;
    }
    if (px > right) {
        px = right;
    }
    if (py < 88) {
        py = 88;
    }
    int bottom = r->tiles_y * TILE_SIZE - TTF_FontHeight(r->font_tiny) - 8;
    if (py > bottom) {
        py = bottom;
    }
    SDL_Rect backing = {px - 4, py - 4, width + 8, TTF_FontHeight(r->font_tiny) + 8};
    SDL_SetRenderDrawColor(r->sdl, 12, 26, 20, 255);
    SDL_RenderFillRect(r->sdl, &backing);
    SDL_SetRenderDrawColor(r->sdl, 221, 195, 116, 255);
    SDL_RenderDrawRect(r->sdl, &backing);
    renderer_draw_text(r, text, px, py, (SDL_Color){241, 215, 147, 255}, r->font_tiny);
}

void draw_town_road(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_TOWN_ROAD) {
        return;
    }
    int index = town_road_index(g->road_town);
    if (index < 0) {
        return;
    }
    if (g->road_town == LOCATION_TOWN) {
        // White water runs east through the northern rapids.
        for (int y = v->cam_y; y <= 10 && y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < TOWN_ROAD_W && x < v->cam_x + v->tiles_x; x++) {
                unsigned int seed = (unsigned int)x * 73u + (unsigned int)y * 151u;
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_SetRenderDrawColor(r->sdl, 139, 202, 215, 255);
                SDL_RenderDrawLine(r->sdl, px + 2, py + 5 + seed % 11u, px + 13, py + 5 + seed % 11u);
                SDL_SetRenderDrawColor(r->sdl, 204, 231, 227, 255);
                SDL_RenderDrawLine(r->sdl, px + 9, py + 8 + seed % 11u, px + 19, py + 8 + seed % 11u);
            }
        }
    }
    SDL_Rect destination = {viewport_to_screen_x(v, TOWN_SPRITE_X) * TILE_SIZE,
        viewport_to_screen_y(v, TOWN_SPRITE_Y) * TILE_SIZE, TOWN_SPRITE_W * TILE_SIZE, TOWN_SPRITE_H * TILE_SIZE};
    if (r->town_textures[index]) {
        SDL_RenderCopy(r->sdl, r->town_textures[index], NULL, &destination);
    }
    SDL_Color label = {224, 201, 123, 255};
    const char *name = approach_town_name(g->road_town);
    int width;
    TTF_SizeText(r->font_small, name, &width, NULL);
    renderer_draw_text(r, name, destination.x + (destination.w - width) / 2, destination.y - 28, label, r->font_small);
    if (g->road_town == LOCATION_TOWN) {
        draw_town_road_sign(r, viewport_to_screen_x(v, TOWN_ROAD_W / 2 - 5), viewport_to_screen_y(v, 12), "NORTHERN RAPIDS\nNO CROSSING");
    }
    for (int side = 0; side < 4; side++) {
        TownApproach connection;
        if (!town_road_connection(g->road_town, side, &connection)) {
            continue;
        }
        const char *area = approach_area_name(connection.area);
        int x;
        int y;
        x = side == TOWN_SIDE_WEST ? TOWN_SPRITE_X - 7 : side == TOWN_SIDE_EAST ? TOWN_SPRITE_X + TOWN_SPRITE_W + 6 : TOWN_ROAD_W / 2;
        y = side == TOWN_SIDE_NORTH ? TOWN_SPRITE_Y - 5 : side == TOWN_SIDE_SOUTH ? TOWN_SPRITE_Y + TOWN_SPRITE_H + 4 : TOWN_ROAD_H / 2 + 2;
        char sign[48];
        snprintf(sign, sizeof(sign), "%c: %s", "NESW"[side], area);
        TTF_SizeText(r->font_tiny, sign, &width, NULL);
        int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
        int py = viewport_to_screen_y(v, y) * TILE_SIZE;
        renderer_draw_text(r, sign, px, py, (SDL_Color){140, 203, 143, 255}, r->font_tiny);
        boundary_label(r, v, side, area);
    }
}
