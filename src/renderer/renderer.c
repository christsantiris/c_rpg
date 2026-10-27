#include "renderer.h"
#include "info_panel.h"
#include "../game/town_road.h"
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

    r->blacksmith_texture = load_sprite_texture(sdl, "assets/images/blacksmith.bmp");
    r->alchemist_texture = load_sprite_texture(sdl, "assets/images/alchemist.bmp");
    r->tavern_texture = load_sprite_texture(sdl, "assets/images/tavern.bmp");
    r->inn_texture = load_sprite_texture(sdl, "assets/images/inn.bmp");
    r->harbor_texture = load_sprite_texture(sdl, "assets/images/harbor.bmp");
    r->healer_texture = load_sprite_texture(sdl, "assets/images/healer.bmp");
    r->witch_texture = load_sprite_texture(sdl, "assets/images/witch-hut.bmp");
    r->labyrinth_texture = load_sprite_texture(sdl,
        "assets/images/labyrinth-entrance.bmp");
    r->dungeon_entrance_texture = load_sprite_texture(sdl, "assets/images/dungeon-entrance.bmp");
    r->swamp_entrance_texture = load_sprite_texture(sdl, "assets/images/swamp-entrance.bmp");
    r->elderwood_entrance_texture = load_sprite_texture(sdl, "assets/images/elderwood-entrance.bmp");
    r->elderwood_enemy_texture = load_sprite_texture(sdl, "assets/images/elderwood-enemies.bmp");
    r->stonewake_enemy_texture = load_sprite_texture(sdl, "assets/images/stonewake-enemies.bmp");
    r->stonewake_workshop_texture = load_sprite_texture(sdl, "assets/images/stonewake-workshop.bmp");
    r->stonewake_crane_texture = load_sprite_texture(sdl, "assets/images/stonewake-crane.bmp");
    r->stonewake_prop_texture = load_sprite_texture(sdl, "assets/images/stonewake-props.bmp");
    r->war_machine_texture = load_sprite_texture(sdl, "assets/images/war-machine-props.bmp");
    r->ironfang_boss_texture = load_sprite_texture(sdl, "assets/images/ironfang-boss.bmp");
    r->stormwatch_enemy_texture = load_sprite_texture(sdl, "assets/images/stormwatch-enemies.bmp");
    r->mangrove_enemy_texture = load_sprite_texture(sdl, "assets/images/mangrove-enemies.bmp");
    r->mangrove_entrance_texture = load_sprite_texture(sdl, "assets/images/mangrove-entrance.bmp");
    r->mangrove_root_texture = load_sprite_texture(sdl, "assets/images/mangrove-root.bmp");
    r->skiff_texture = load_sprite_texture(sdl, "assets/images/skiff.bmp");
    r->stormbell_texture = load_sprite_texture(sdl, "assets/images/stormbell-props.bmp");
    r->spice_skiff_texture = load_sprite_texture(sdl, "assets/images/spice-skiff-props.bmp");
    r->forest_entrance_texture = load_sprite_texture(sdl, "assets/images/forest-entrance.bmp");
    r->mountain_entrance_texture = load_sprite_texture(sdl, "assets/images/mountain-entrance.bmp");
    const char *entrance_paths[TOWN_EXIT_GIANTS + 1] = {NULL};
    entrance_paths[TOWN_EXIT_DESERT] = "assets/images/desert-entrance.bmp";
    entrance_paths[TOWN_EXIT_COAST] = "assets/images/coast-entrance.bmp";
    entrance_paths[TOWN_EXIT_DRAGONSPINE] = "assets/images/dragonspine-entrance.bmp";
    entrance_paths[TOWN_EXIT_FROST] = "assets/images/frostfell-entrance.bmp";

    entrance_paths[TOWN_EXIT_ASHEN] = "assets/images/ashen-entrance.bmp";
    entrance_paths[TOWN_EXIT_GLASSDEEP] = "assets/images/stonewake-entrance.bmp";
    entrance_paths[TOWN_EXIT_GIANTS] = "assets/images/giants-crown-entrance.bmp";
    entrance_paths[TOWN_EXIT_IRONFANG] = "assets/images/ironfang-entrance.bmp";
    entrance_paths[TOWN_EXIT_STORMWATCH] = "assets/images/stormwatch-entrance.bmp";
    entrance_paths[TOWN_EXIT_CATACOMBS] = "assets/images/catacombs-entrance.bmp";
    for (int i = 0; i <= TOWN_EXIT_GIANTS; i++) {
        r->area_entrance_textures[i] = entrance_paths[i] ? load_sprite_texture(sdl, entrance_paths[i]) : NULL;
    }
    const char *towns[TOWN_ROAD_COUNT] = {"oakhaven", "stillbury", "rosemoor", "ridgeshire", "dawnmere"};
    for (int i = 0; i < TOWN_ROAD_COUNT; i++) {
        char path[100];
        snprintf(path, sizeof(path), "assets/images/%s-town.bmp", towns[i]);
        r->town_textures[i] = load_sprite_texture(sdl, path);
    }
    for (int i = 0; i <= TOWN_EXIT_GLASSDEEP; i++) {
        r->town_gate_textures[i] = NULL;
    }
    r->castle_texture = load_sprite_texture(sdl, "assets/images/castle-of-no-return.bmp");
    r->apothecary_texture = load_sprite_texture(sdl, "assets/images/apothecary.bmp");
    r->guild_texture = load_sprite_texture(sdl, "assets/images/adventurers-guild.bmp");
    r->workshop_texture = load_sprite_texture(sdl, "assets/images/workshop.bmp");
    r->town_hall_texture = load_sprite_texture(sdl, "assets/images/town-hall.bmp");
    const char *local_paths[4] = {
        "assets/images/bakery.bmp",
        "assets/images/butcher.bmp",
        "assets/images/monastery.bmp",
        "assets/images/spice-merchant.bmp"
    };
    for (int i = 0; i < 4; i++) {
        r->local_building_textures[i] = load_sprite_texture(sdl, local_paths[i]);
    }
    r->island_texture = load_sprite_texture(sdl, "assets/images/island-sprites.bmp");
    r->island_temple_texture = load_sprite_texture(sdl, "assets/images/temple-entrance.bmp");
    r->island_ship_texture = load_sprite_texture(sdl,
        "assets/images/island-ship.bmp");
    r->temple_enemy_texture = load_sprite_texture(sdl,
        "assets/images/temple-enemies.bmp");

    r->castle_enemy_texture = load_sprite_texture(sdl, "assets/images/castle-enemies.bmp");
    r->castle_boss_texture = load_sprite_texture(sdl, "assets/images/castle-bosses.bmp");
    r->frost_kraken_texture = load_sprite_texture(sdl, "assets/images/frost-kraken.bmp");

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
    SDL_DestroyTexture(r->stonewake_enemy_texture);
    SDL_DestroyTexture(r->stonewake_workshop_texture);
    r->stonewake_workshop_texture = NULL;
    SDL_DestroyTexture(r->stonewake_crane_texture);
    SDL_DestroyTexture(r->stonewake_prop_texture);
    r->stonewake_enemy_texture = NULL;
    r->stonewake_crane_texture = NULL;
    r->stonewake_prop_texture = NULL;
    SDL_DestroyTexture(r->ironfang_boss_texture);
    SDL_DestroyTexture(r->war_machine_texture);
    r->war_machine_texture = NULL;
    r->ironfang_boss_texture = NULL;
    SDL_DestroyTexture(r->stormwatch_enemy_texture);
    r->stormwatch_enemy_texture = NULL;
    SDL_DestroyTexture(r->mangrove_enemy_texture);
    r->mangrove_enemy_texture = NULL;
    SDL_DestroyTexture(r->mangrove_entrance_texture);
    SDL_DestroyTexture(r->mangrove_root_texture);
    SDL_DestroyTexture(r->skiff_texture);
    SDL_DestroyTexture(r->spice_skiff_texture);
    SDL_DestroyTexture(r->stormbell_texture);
    r->stormbell_texture = NULL;
    r->mangrove_entrance_texture = NULL;
    r->mangrove_root_texture = NULL;
    r->skiff_texture = NULL;
    r->spice_skiff_texture = NULL;
    for (int i = 0; i < TOWN_ROAD_COUNT; i++) {
        if (r->town_textures[i]) {
            SDL_DestroyTexture(r->town_textures[i]);
            r->town_textures[i] = NULL;
        }
    }
    for (int i = 0; i <= TOWN_EXIT_GLASSDEEP; i++) {
        if (r->town_gate_textures[i]) {
            SDL_DestroyTexture(r->town_gate_textures[i]);
            r->town_gate_textures[i] = NULL;
        }
    }
    for (int i = 0; i < 4; i++) {
        if (r->local_building_textures[i]) {
            SDL_DestroyTexture(r->local_building_textures[i]);
            r->local_building_textures[i] = NULL;
        }
    }
    if (r->castle_enemy_texture) {
        SDL_DestroyTexture(r->castle_enemy_texture);
        r->castle_enemy_texture = NULL;
    }
    if (r->frost_kraken_texture) {
        SDL_DestroyTexture(r->frost_kraken_texture);
        r->frost_kraken_texture = NULL;
    }
    if (r->castle_boss_texture) {
        SDL_DestroyTexture(r->castle_boss_texture);
        r->castle_boss_texture = NULL;
    }
    if (r->castle_texture) {
        SDL_DestroyTexture(r->castle_texture);
        r->castle_texture = NULL;
    }
    if (r->apothecary_texture) {
        SDL_DestroyTexture(r->apothecary_texture);
        r->apothecary_texture = NULL;
    }
    if (r->guild_texture) {
        SDL_DestroyTexture(r->guild_texture);
        r->guild_texture = NULL;
    }
    if (r->island_ship_texture) {
        SDL_DestroyTexture(r->island_ship_texture);
        r->island_ship_texture = NULL;
    }
    if (r->temple_enemy_texture) {
        SDL_DestroyTexture(r->temple_enemy_texture);
        r->temple_enemy_texture = NULL;
    }
    if (r->island_texture) {
        SDL_DestroyTexture(r->island_texture);
        r->island_texture = NULL;
    }
    if (r->island_temple_texture) {
        SDL_DestroyTexture(r->island_temple_texture);
        r->island_temple_texture = NULL;
    }
    if (r->workshop_texture) {
        SDL_DestroyTexture(r->workshop_texture);
        r->workshop_texture = NULL;
    }
    if (r->town_hall_texture) {
        SDL_DestroyTexture(r->town_hall_texture);
        r->town_hall_texture = NULL;
    }
    if (r->healer_texture) {
        SDL_DestroyTexture(r->healer_texture);
        r->healer_texture = NULL;
    }
    if (r->witch_texture) {
        SDL_DestroyTexture(r->witch_texture);
        r->witch_texture = NULL;
    }
    if (r->labyrinth_texture) {
        SDL_DestroyTexture(r->labyrinth_texture);
        r->labyrinth_texture = NULL;
    }
    if (r->swamp_entrance_texture) {
        SDL_DestroyTexture(r->swamp_entrance_texture);
        r->swamp_entrance_texture = NULL;
    }
    for (int i = 0; i <= TOWN_EXIT_GIANTS; i++) {
        if (r->area_entrance_textures[i]) {
            SDL_DestroyTexture(r->area_entrance_textures[i]);
            r->area_entrance_textures[i] = NULL;
        }
    }
    if (r->elderwood_entrance_texture) {
        SDL_DestroyTexture(r->elderwood_entrance_texture);
        r->elderwood_entrance_texture = NULL;
    }
    if (r->elderwood_enemy_texture) {
        SDL_DestroyTexture(r->elderwood_enemy_texture);
        r->elderwood_enemy_texture = NULL;
    }
    if (r->forest_entrance_texture) {
        SDL_DestroyTexture(r->forest_entrance_texture);
        r->forest_entrance_texture = NULL;
    }
    if (r->mountain_entrance_texture) {
        SDL_DestroyTexture(r->mountain_entrance_texture);
        r->mountain_entrance_texture = NULL;
    }
    if (r->dungeon_entrance_texture) {
        SDL_DestroyTexture(r->dungeon_entrance_texture);
        r->dungeon_entrance_texture = NULL;
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
    if (r->inn_texture) {
        SDL_DestroyTexture(r->inn_texture);
        r->inn_texture = NULL;
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
