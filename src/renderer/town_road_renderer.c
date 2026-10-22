#include "town_road_renderer.h"
#include "../game/town_road.h"
#include "sprites.h"
#include <stdio.h>

void draw_town_road(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_TOWN_ROAD) {
        return;
    }
    int index = town_road_index(g->road_town);
    if (index < 0) {
        return;
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
    }
}
