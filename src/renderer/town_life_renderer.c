#include "town_life_renderer.h"
#include "sprites.h"

static void box(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void town_life_draw_goods(Renderer *r, int tx, int ty, int style) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    if (style == 0) {
        box(r, x + 3, y + 8, 18, 8, (SDL_Color){137, 80, 36, 255});
        box(r, x + 5, y + 6, 14, 8, (SDL_Color){228, 177, 86, 255});
        for (int i = 0; i < 3; i++) {
            box(r, x + 7 + i * 4, y + 7, 2, 4, (SDL_Color){255, 220, 139, 255});
        }
    } else if (style == 1) {
        box(r, x + 3, y + 6, 18, 12, (SDL_Color){96, 49, 40, 255});
        box(r, x + 5, y + 7, 14, 8, (SDL_Color){190, 83, 80, 255});
        box(r, x + 10, y + 9, 5, 4, (SDL_Color){242, 205, 179, 255});
    } else if (style == 2) {
        box(r, x + 6, y + 16, 12, 3, (SDL_Color){148, 137, 111, 255});
        box(r, x + 9, y + 6, 6, 11, (SDL_Color){233, 218, 164, 255});
        box(r, x + 11, y + 3, 2, 4, (SDL_Color){255, 185, 67, 255});
    } else {
        SDL_Color spices[3] = {{197, 83, 42, 255}, {225, 170, 48, 255}, {111, 153, 73, 255}};
        for (int i = 0; i < 3; i++) {
            box(r, x + 2 + i * 7, y + 10, 6, 8, (SDL_Color){122, 78, 52, 255});
            box(r, x + 2 + i * 7, y + 8, 6, 4, spices[i]);
        }
    }
}

void town_life_draw_building(Renderer *r, const Viewport *v, const TownBuilding *b) {
    int x = viewport_to_screen_x(v, b->x) * TILE_SIZE;
    int y = viewport_to_screen_y(v, b->y) * TILE_SIZE;
    int w = b->w * TILE_SIZE;
    int h = b->h * TILE_SIZE;
    SDL_Texture *texture = r->local_building_textures[b->style];
    if (texture) {
        SDL_Rect dest = {x, y, w, h};
        SDL_RenderCopy(r->sdl, texture, NULL, &dest);
    } else if (b->style == 2) {
        draw_town_hall(r, viewport_to_screen_x(v, b->x), viewport_to_screen_y(v, b->y));
    } else {
        draw_workshop(r, viewport_to_screen_x(v, b->x), viewport_to_screen_y(v, b->y));
    }
    int width = 0;
    TTF_SizeText(r->font_tiny, b->label, &width, NULL);
    renderer_draw_text(r, b->label, x + (w - width) / 2, y - TILE_SIZE,
        (SDL_Color){220, 180, 60, 255}, r->font_tiny);
}

void town_life_draw_resident(Renderer *r, int tx, int ty, int style) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    SDL_Color clothes[4] = {{168, 122, 65, 255}, {115, 65, 57, 255}, {91, 76, 52, 255}, {125, 67, 145, 255}};
    draw_tavern_floor(r, tx, ty);
    box(r, x + 8, y + 3, 9, 5, (SDL_Color){71, 48, 33, 255});
    box(r, x + 8, y + 7, 9, 6, (SDL_Color){203, 156, 118, 255});
    box(r, x + 6, y + 13, 13, 10, clothes[style]);
    box(r, x + 3, y + 14, 4, 7, clothes[style]);
    box(r, x + 18, y + 14, 4, 7, clothes[style]);
    box(r, x + 10, y + 9, 2, 2, (SDL_Color){38, 30, 27, 255});
    box(r, x + 15, y + 9, 2, 2, (SDL_Color){38, 30, 27, 255});
    if (style == 0 || style == 1) {
        box(r, x + 8, y + 14, 9, 8, (SDL_Color){229, 216, 185, 255});
        box(r, x + 6, y + 2, 13, 5, (SDL_Color){229, 216, 185, 255});
    } else if (style == 2) {
        box(r, x + 8, y + 12, 9, 4, (SDL_Color){188, 182, 164, 255});
        box(r, x + 6, y + 19, 13, 2, (SDL_Color){148, 119, 60, 255});
    } else {
        box(r, x + 6, y + 3, 13, 4, (SDL_Color){201, 150, 78, 255});
    }
}
