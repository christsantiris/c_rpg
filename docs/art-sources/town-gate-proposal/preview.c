// Approval scene using the game's existing SDL tile, building and player art.
#include "renderer/sprites.h"
#include <assert.h>

static void scene(Renderer *r, SDL_Texture *gate) {
    for (int y = 0; y < 14; y++) {
        for (int x = 0; x < 24; x++) {
            draw_town_floor(r, x, y);
            if (x == 12 || (y == 12 && x >= 5 && x <= 12) || (x == 5 && y >= 11 && y <= 12)) {
                draw_town_path(r, x, y);
            }
            if (y == 6 && x != 12) {
                draw_wall(r, x, y);
            }
        }
    }
    draw_shop_blacksmith(r, 3, 8);
    SDL_Rect dest = {240, 72, 120, 96};
    SDL_RenderCopy(r->sdl, gate, NULL, &dest);
    Item sword = item_make_long_sword();
    Item shield = item_make_tower_shield();
    draw_player_at(r, 288, 216, CLASS_WARRIOR, &sword, &shield, NULL, 0, -1);
    renderer_draw_text(r, "ELDERWOOD FOREST", 342, 206, (SDL_Color){151, 192, 116, 255}, r->font_tiny);
    renderer_draw_text(r, "CASTLE ROAD", 342, 220, (SDL_Color){210, 190, 140, 255}, r->font_tiny);
    SDL_RenderPresent(r->sdl);
}

int main(void) {
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 576, 336, 32, SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    SDL_Renderer *sdl = SDL_CreateSoftwareRenderer(surface);
    assert(sdl);
    Renderer r = {0};
    renderer_init(&r, sdl, 576, 336);
    const char *names[] = {"open", "closed"};
    for (int n = 0; n < 2; n++) {
        char path[160];
        SDL_snprintf(path, sizeof(path), "docs/art-sources/town-gate-proposal/gate-%s.bmp", names[n]);
        SDL_Surface *sprite = SDL_LoadBMP(path);
        assert(sprite);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(sdl, sprite);
        assert(texture);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
        scene(&r, texture);
        SDL_snprintf(path, sizeof(path), "/tmp/town-gate-%s-scene.bmp", names[n]);
        assert(SDL_SaveBMP(surface, path) == 0);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(sprite);
    }
    renderer_free(&r);
    SDL_DestroyRenderer(sdl);
    SDL_FreeSurface(surface);
    SDL_Quit();
    return 0;
}
