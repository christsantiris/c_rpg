#ifndef RENDERER_HEADER_H
#define RENDERER_HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define TILE_SIZE 24

typedef enum {
    TOWN_EXIT_DUNGEON,
    TOWN_EXIT_FOREST,
    TOWN_EXIT_MOUNTAINS,
    TOWN_EXIT_COAST,
    TOWN_EXIT_ROAD,
    TOWN_EXIT_SWAMP,
    TOWN_EXIT_DRAGONSPINE,
    TOWN_EXIT_FROST,
    TOWN_EXIT_DESERT,
    TOWN_EXIT_ASHEN,
    TOWN_EXIT_GLASSDEEP,
    TOWN_EXIT_CATACOMBS,
    TOWN_EXIT_STORMWATCH,
    TOWN_EXIT_IRONFANG,
    TOWN_EXIT_GIANTS
} TownExitStyle;

typedef struct {
    SDL_Renderer *sdl;
    SDL_Texture *blacksmith_texture;
    SDL_Texture *alchemist_texture;
    SDL_Texture *tavern_texture;
    SDL_Texture *inn_texture;
    SDL_Texture *harbor_texture;
    SDL_Texture *healer_texture;
    SDL_Texture *witch_texture;
    SDL_Texture *labyrinth_texture;
    SDL_Texture *dungeon_entrance_texture;
    SDL_Texture *mountain_entrance_texture;
    SDL_Texture *forest_entrance_texture;
    SDL_Texture *elderwood_entrance_texture;
    SDL_Texture *elderwood_enemy_texture;
    SDL_Texture *mangrove_enemy_texture;
    SDL_Texture *stormwatch_enemy_texture;
    SDL_Texture *ironfang_boss_texture;
    SDL_Texture *stonewake_enemy_texture;
    SDL_Texture *stonewake_crane_texture;
    SDL_Texture *stonewake_prop_texture;
    SDL_Texture *stonewake_workshop_texture;
    SDL_Texture *mangrove_entrance_texture;
    SDL_Texture *mangrove_root_texture;
    SDL_Texture *skiff_texture;
    SDL_Texture *spice_skiff_texture;
    SDL_Texture *swamp_entrance_texture;
    SDL_Texture *area_entrance_textures[TOWN_EXIT_GIANTS + 1];
    SDL_Texture *town_gate_textures[TOWN_EXIT_GLASSDEEP + 1];
    SDL_Texture *town_textures[5]; // Indexed by town_road_index().
    SDL_Texture *castle_texture;
    SDL_Texture *apothecary_texture;
    SDL_Texture *guild_texture;
    SDL_Texture *workshop_texture;
    SDL_Texture *town_hall_texture;
    SDL_Texture *local_building_textures[4];
    SDL_Texture *island_texture;
    SDL_Texture *island_temple_texture;
    SDL_Texture *island_ship_texture;
    SDL_Texture *temple_enemy_texture;
    SDL_Texture *castle_enemy_texture;
    SDL_Texture *castle_boss_texture;
    SDL_Texture *frost_kraken_texture;
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
