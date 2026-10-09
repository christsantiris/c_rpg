#include "game_renderer.h"
#include "sprites.h"
#include "info_panel.h"
#include "message_bar.h"
#include "minimap_renderer.h"
#include "renderer.h"
#include "item_icons.h"
#include "../game/combat_feedback.h"
#include "../game/catacombs.h"
#include "castle_renderer.h"
#include "jail_renderer.h"
#include "town_life_renderer.h"
#include "../game/jail.h"
#include <string.h>
#include <stdlib.h>

static SDL_Color area_label_color(Location area) {
    switch (area) {
        case LOCATION_FOREST: return (SDL_Color){90, 190, 105, 255};
        case LOCATION_MOUNTAINS: return (SDL_Color){220, 72, 42, 255};
        case LOCATION_COAST: return (SDL_Color){62, 210, 205, 255};
        case LOCATION_SWAMP: return (SDL_Color){113, 204, 79, 255};
        case LOCATION_DRAGONSPINE: return (SDL_Color){187, 218, 232, 255};
        case LOCATION_FROSTFELL: return (SDL_Color){168, 220, 250, 255};
        case LOCATION_MOONVEIL: return (SDL_Color){190, 163, 231, 255};
        case LOCATION_ASHEN: return (SDL_Color){240, 150, 76, 255};
        case LOCATION_CATACOMBS: return (SDL_Color){224, 216, 186, 255};
        case LOCATION_GLASSDEEP: return (SDL_Color){170, 207, 241, 255};
        case LOCATION_TEMPLE: return (SDL_Color){235, 201, 92, 255};
        default: return (SDL_Color){220, 180, 60, 255};
    }
}

static void draw_oswin(Renderer *r, int sx, int sy) {
    draw_tavern_floor(r, sx, sy);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_SetRenderDrawColor(r->sdl, 89, 85, 103, 255);
    SDL_Rect hood = {px + 6, py + 3, 13, 12};
    SDL_Rect robe = {px + 5, py + 13, 15, 11};
    SDL_RenderFillRect(r->sdl, &hood);
    SDL_RenderFillRect(r->sdl, &robe);
    SDL_SetRenderDrawColor(r->sdl, 208, 168, 137, 255);
    SDL_Rect face = {px + 9, py + 6, 7, 7};
    SDL_RenderFillRect(r->sdl, &face);
    SDL_SetRenderDrawColor(r->sdl, 224, 216, 186, 255);
    SDL_RenderDrawLine(r->sdl, px + 10, py + 14, px + 10, py + 22);
    SDL_RenderDrawLine(r->sdl, px + 14, py + 14, px + 14, py + 22);
    SDL_SetRenderDrawColor(r->sdl, 29, 27, 36, 255);
    SDL_RenderDrawPoint(r->sdl, px + 10, py + 8);
    SDL_RenderDrawPoint(r->sdl, px + 14, py + 8);
    SDL_SetRenderDrawColor(r->sdl, 112, 74, 47, 255);
    SDL_Rect book = {px + 14, py + 15, 8, 7};
    SDL_RenderFillRect(r->sdl, &book);
    SDL_SetRenderDrawColor(r->sdl, 226, 216, 183, 255);
    SDL_RenderDrawLine(r->sdl, px + 16, py + 17, px + 20, py + 17);
    SDL_RenderDrawLine(r->sdl, px + 16, py + 19, px + 20, py + 19);
}

static void draw_frostfell_quest_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_NPC_BRENNA || tile == TILE_NPC_ORIN) {
        draw_tavern_floor(r, sx, sy);
    } else {
        draw_frostfell_floor(r, sx, sy, x, y);
    }
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_FROST_JOURNAL) {
        SDL_SetRenderDrawColor(r->sdl, 64, 50, 43, 255);
        SDL_Rect cover = {px + 5, py + 7, 15, 13};
        SDL_RenderFillRect(r->sdl, &cover);
        SDL_SetRenderDrawColor(r->sdl, 229, 214, 178, 255);
        SDL_Rect pages = {px + 8, py + 8, 11, 10};
        SDL_RenderFillRect(r->sdl, &pages);
        SDL_SetRenderDrawColor(r->sdl, 51, 128, 164, 255);
        SDL_RenderDrawLine(r->sdl, px + 7, py + 8, px + 7, py + 18);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 11, px + 17, py + 11);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 14, px + 16, py + 14);
        return;
    }
    SDL_SetRenderDrawColor(r->sdl, 53, 77, 110, 255);
    SDL_Rect hood = {px + 6, py + 3, 13, 11};
    SDL_Rect coat = {px + 5, py + 12, 15, 10};
    SDL_RenderFillRect(r->sdl, &hood);
    SDL_RenderFillRect(r->sdl, &coat);
    SDL_SetRenderDrawColor(r->sdl, 208, 164, 126, 255);
    SDL_Rect face = {px + 9, py + 6, 7, 6};
    SDL_RenderFillRect(r->sdl, &face);
    SDL_SetRenderDrawColor(r->sdl, 214, 225, 230, 255);
    SDL_RenderDrawLine(r->sdl, px + 7, py + 13, px + 17, py + 13);
    SDL_RenderDrawLine(r->sdl, px + 12, py + 14, px + 12, py + 20);
    SDL_SetRenderDrawColor(r->sdl, 29, 37, 49, 255);
    SDL_RenderDrawPoint(r->sdl, px + 10, py + 8);
    SDL_RenderDrawPoint(r->sdl, px + 14, py + 8);
    SDL_Rect boots[2] = {{px + 6, py + 22, 5, 2}, {px + 14, py + 22, 5, 2}};
    SDL_RenderFillRects(r->sdl, boots, 2);
    if (tile == TILE_NPC_ORIN) {
        SDL_SetRenderDrawColor(r->sdl, 236, 220, 172, 255);
        SDL_Rect map = {px + 16, py + 13, 6, 9};
        SDL_RenderFillRect(r->sdl, &map);
        SDL_SetRenderDrawColor(r->sdl, 70, 148, 174, 255);
        SDL_RenderDrawLine(r->sdl, px + 18, py + 15, px + 20, py + 20);
    }
    if (tile == TILE_NPC_BRENNA) {
        SDL_SetRenderDrawColor(r->sdl, 228, 182, 78, 255);
        SDL_Rect badge = {px + 15, py + 15, 3, 3};
        SDL_RenderFillRect(r->sdl, &badge);
    }
}

static void draw_moonveil_quest_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_NPC_LIORA) {
        draw_town_path(r, sx, sy);
        SDL_SetRenderDrawColor(r->sdl, 80, 52, 42, 255);
        SDL_Rect hair = {px + 7, py + 3, 12, 11};
        SDL_RenderFillRect(r->sdl, &hair);
        SDL_SetRenderDrawColor(r->sdl, 220, 175, 139, 255);
        SDL_Rect face = {px + 9, py + 6, 7, 7};
        SDL_RenderFillRect(r->sdl, &face);
        SDL_SetRenderDrawColor(r->sdl, 67, 121, 78, 255);
        SDL_Rect cloak = {px + 5, py + 13, 15, 10};
        SDL_RenderFillRect(r->sdl, &cloak);
        SDL_SetRenderDrawColor(r->sdl, 175, 208, 139, 255);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 14, px + 10, py + 21);
        SDL_SetRenderDrawColor(r->sdl, 33, 34, 30, 255);
        SDL_RenderDrawPoint(r->sdl, px + 10, py + 8);
        SDL_RenderDrawPoint(r->sdl, px + 14, py + 8);
        SDL_SetRenderDrawColor(r->sdl, 145, 102, 62, 255);
        SDL_Rect satchel = {px + 16, py + 16, 6, 6};
        SDL_RenderFillRect(r->sdl, &satchel);
        SDL_SetRenderDrawColor(r->sdl, 191, 161, 229, 255);
        SDL_RenderDrawPoint(r->sdl, px + 18, py + 15);
        SDL_RenderDrawPoint(r->sdl, px + 19, py + 14);
        return;
    }
    if (tile == TILE_MOONVEIL_PLANTING_CIRCLE || tile == TILE_MOONVEIL_MOONFLOWER) {
        draw_moonveil_circle(r, sx, sy, x, y);
    } else {
        draw_moonveil_floor(r, sx, sy, x, y);
    }
    if (tile == TILE_MOONVEIL_SEED_POD) {
        SDL_SetRenderDrawColor(r->sdl, 105, 79, 60, 255);
        SDL_Rect pod = {px + 6, py + 8, 13, 12};
        SDL_RenderFillRect(r->sdl, &pod);
        SDL_SetRenderDrawColor(r->sdl, 61, 137, 82, 255);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 8, px + 7, py + 4);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 8, px + 16, py + 3);
        SDL_SetRenderDrawColor(r->sdl, 218, 209, 249, 255);
        SDL_Rect seed = {px + 10, py + 11, 5, 6};
        SDL_RenderFillRect(r->sdl, &seed);
        return;
    }
    if (tile == TILE_MOONVEIL_SPRING) {
        SDL_SetRenderDrawColor(r->sdl, 133, 141, 166, 255);
        SDL_Rect basin = {px + 4, py + 9, 17, 10};
        SDL_RenderFillRect(r->sdl, &basin);
        SDL_SetRenderDrawColor(r->sdl, 102, 178, 209, 255);
        SDL_Rect water = {px + 6, py + 10, 13, 6};
        SDL_RenderFillRect(r->sdl, &water);
        SDL_SetRenderDrawColor(r->sdl, 214, 228, 251, 255);
        int glint = (SDL_GetTicks() / 220) % 7;
        SDL_RenderDrawLine(r->sdl, px + 8 + glint, py + 12, px + 10 + glint, py + 12);
        return;
    }
    if (tile == TILE_MOONVEIL_PLANTING_CIRCLE) {
        SDL_SetRenderDrawColor(r->sdl, 60, 85, 49, 255);
        SDL_RenderDrawLine(r->sdl, px + 3, py + 8, px + 20, py + 18);
        SDL_RenderDrawLine(r->sdl, px + 6, py + 20, px + 17, py + 5);
        SDL_RenderDrawLine(r->sdl, px + 4, py + 15, px + 19, py + 10);
        return;
    }
    SDL_SetRenderDrawColor(r->sdl, 102, 171, 117, 255);
    SDL_RenderDrawLine(r->sdl, px + 12, py + 11, px + 12, py + 21);
    SDL_RenderDrawLine(r->sdl, px + 12, py + 17, px + 7, py + 15);
    SDL_RenderDrawLine(r->sdl, px + 12, py + 19, px + 17, py + 16);
    int bloom = tile == TILE_MOONVEIL_MOONFLOWER;
    int pulse = (SDL_GetTicks() / 180) % 3;
    SDL_SetRenderDrawColor(r->sdl, 198 + pulse * 12, 184 + pulse * 15, 240, 255);
    SDL_Rect petals[4] = {{px + 8, py + 4, 7, 4}, {px + 8, py + 11, 7, 4},
        {px + 5, py + 7, 4, 5}, {px + 15, py + 7, 4, 5}};
    if (bloom) {
        SDL_RenderFillRects(r->sdl, petals, 4);
    } else {
        SDL_RenderDrawLine(r->sdl, px + 8, py + 8, px + 16, py + 8);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 5, px + 12, py + 11);
    }
    SDL_SetRenderDrawColor(r->sdl, 225, 248, 235, 255);
    SDL_Rect heart = {px + 10, py + 7, bloom ? 5 : 3, bloom ? 5 : 3};
    SDL_RenderFillRect(r->sdl, &heart);
}

static void draw_glassdeep_resonator(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    draw_glassdeep_ruin(r, sx, sy, x, y);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    int lit = tile == TILE_GLASSDEEP_RESONATOR_LIT;
    if (lit) {
        int pulse = (SDL_GetTicks() / 160) % 4;
        SDL_SetRenderDrawColor(r->sdl, 44, 100 + pulse * 12, 124 + pulse * 15, 255);
        SDL_Rect glow = {px + 3 - pulse / 2, py + 3 - pulse / 2, 18 + pulse, 18 + pulse};
        SDL_RenderDrawRect(r->sdl, &glow);
    }
    SDL_SetRenderDrawColor(r->sdl, 94, 100, 120, 255);
    SDL_Rect base = {px + 4, py + 18, 17, 5};
    SDL_RenderFillRect(r->sdl, &base);
    SDL_SetRenderDrawColor(r->sdl, lit ? 112 : 69, lit ? 232 : 91, lit ? 248 : 127, 255);
    for (int row = 0; row < 15; row++) {
        int half = row < 7 ? row / 2 : (14 - row) / 2;
        SDL_RenderDrawLine(r->sdl, px + 12 - half, py + 3 + row, px + 12 + half, py + 3 + row);
    }
    SDL_SetRenderDrawColor(r->sdl, lit ? 233 : 128, lit ? 255 : 146, lit ? 255 : 175, 255);
    SDL_RenderDrawLine(r->sdl, px + 11, py + 5, px + 11, py + 15);
    // The three pedestal marks stay visible even when the crystal is dark.
    for (int mark = 0; mark < 3; mark++) {
        SDL_RenderDrawPoint(r->sdl, px + 8 + mark * 4, py + 20);
    }
}

static void draw_emberforge_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    draw_ashen_ruin(r, sx, sy, x, y);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_EMBERFORGE_MECHANISM) {
        SDL_SetRenderDrawColor(r->sdl, 44, 40, 38, 255);
        SDL_Rect shadow = {px + 4, py + 5, 17, 16};
        SDL_RenderFillRect(r->sdl, &shadow);
        SDL_SetRenderDrawColor(r->sdl, 174, 183, 188, 255);
        SDL_Rect gear = {px + 7, py + 7, 10, 10};
        SDL_RenderDrawRect(r->sdl, &gear);
        SDL_Rect teeth[4] = {{px + 10, py + 4, 4, 4}, {px + 16, py + 10, 4, 4},
            {px + 10, py + 16, 4, 4}, {px + 4, py + 10, 4, 4}};
        for (int i = 0; i < 4; i++) {
            SDL_RenderFillRect(r->sdl, &teeth[i]);
        }
        SDL_SetRenderDrawColor(r->sdl, 237, 183, 78, 255);
        SDL_Rect hub = {px + 10, py + 10, 4, 4};
        SDL_RenderFillRect(r->sdl, &hub);
        return;
    }
    SDL_SetRenderDrawColor(r->sdl, 32, 29, 33, 255);
    SDL_Rect body = {px + 3, py + 8, 18, 14};
    SDL_RenderFillRect(r->sdl, &body);
    SDL_SetRenderDrawColor(r->sdl, 103, 97, 91, 255);
    SDL_Rect chimney = {px + 14, py + 3, 6, 7};
    SDL_RenderFillRect(r->sdl, &chimney);
    SDL_RenderDrawRect(r->sdl, &body);
    SDL_RenderDrawLine(r->sdl, px + 4, py + 12, px + 19, py + 12);
    SDL_RenderDrawLine(r->sdl, px + 4, py + 20, px + 19, py + 20);
    int lit = tile == TILE_EMBERFORGE_LIT;
    SDL_SetRenderDrawColor(r->sdl, lit ? 234 : 55, lit ? 111 : 52, lit ? 28 : 53, 255);
    SDL_Rect hearth = {px + 7, py + 14, 10, 6};
    SDL_RenderFillRect(r->sdl, &hearth);
    if (lit) {
        int flicker = (SDL_GetTicks() / AMBIENT_FRAME_MS) % 2;
        SDL_SetRenderDrawColor(r->sdl, 255, 217, 108, 255);
        SDL_Rect fire = {px + 10, py + 15 - flicker, 4, 5 + flicker};
        SDL_RenderFillRect(r->sdl, &fire);
    }
}

static void draw_catacombs_tile(Renderer *r, const Map *m, int sx, int sy, int x, int y, TileType type) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_Rect tile = {px, py, TILE_SIZE, TILE_SIZE};
    int wall = type == TILE_CATACOMBS_WALL;
    int shade = (x * 17 + y * 31) % 7;
    SDL_SetRenderDrawColor(r->sdl, wall ? 66 : 37 + shade, wall ? 65 : 40 + shade, wall ? 70 : 48 + shade, 255);
    SDL_RenderFillRect(r->sdl, &tile);
    SDL_SetRenderDrawColor(r->sdl, wall ? 33 : 29, wall ? 35 : 31, wall ? 43 : 40, 255);
    SDL_RenderDrawLine(r->sdl, px, py + 11, px + 23, py + 11);
    SDL_RenderDrawLine(r->sdl, px + 11, py, px + 11, py + 10);
    int exposed = wall && (map_is_walkable(m, x - 1, y) || map_is_walkable(m, x + 1, y) ||
        map_is_walkable(m, x, y - 1) || map_is_walkable(m, x, y + 1));
    if (exposed) {
        SDL_Rect niche = {px + 3, py + 3, 17, 7};
        SDL_SetRenderDrawColor(r->sdl, 24, 26, 35, 255);
        SDL_RenderFillRect(r->sdl, &niche);
        SDL_SetRenderDrawColor(r->sdl, 166, 159, 132, 255);
        SDL_RenderDrawLine(r->sdl, px + 5, py + 6, px + 17, py + 6);
        SDL_Rect skull = {px + 7 + shade, py + 13, 5, 5};
        SDL_RenderFillRect(r->sdl, &skull);
        SDL_SetRenderDrawColor(r->sdl, 30, 30, 37, 255);
        SDL_RenderDrawPoint(r->sdl, skull.x + 1, skull.y + 2);
        SDL_RenderDrawPoint(r->sdl, skull.x + 3, skull.y + 2);
    } else if (type == TILE_OSSUARY_BRAZIER || type == TILE_OSSUARY_COLD ||
        type == TILE_MEMORIAL_BRAZIER || type == TILE_MEMORIAL_COLD) {
        SDL_Rect bowl = {px + 5, py + 13, 14, 5};
        SDL_SetRenderDrawColor(r->sdl, 132, 124, 105, 255);
        SDL_RenderFillRect(r->sdl, &bowl);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 18, px + 11, py + 22);
        if (type == TILE_MEMORIAL_BRAZIER || type == TILE_MEMORIAL_COLD) {
            SDL_SetRenderDrawColor(r->sdl, 224, 216, 186, 255);
            SDL_Rect plaque = {px + 7, py + 19, 10, 4};
            SDL_RenderFillRect(r->sdl, &plaque);
        }
        if (type == TILE_OSSUARY_BRAZIER || type == TILE_MEMORIAL_BRAZIER) {
            int flicker = (SDL_GetTicks() / 180 + x + y) % 3;
            SDL_Rect flame = {px + 8, py + 5 - flicker, 8, 9 + flicker};
            SDL_SetRenderDrawColor(r->sdl, 42, 115, 195, 255);
            SDL_RenderFillRect(r->sdl, &flame);
            flame = (SDL_Rect){px + 11, py + 7 - flicker, 3, 6 + flicker};
            SDL_SetRenderDrawColor(r->sdl, 154, 226, 244, 255);
            SDL_RenderFillRect(r->sdl, &flame);
        }
    } else if (type == TILE_BURIAL_LEDGER) {
        SDL_SetRenderDrawColor(r->sdl, 102, 103, 105, 255);
        SDL_Rect plinth = {px + 3, py + 14, 18, 9};
        SDL_RenderFillRect(r->sdl, &plinth);
        SDL_SetRenderDrawColor(r->sdl, 115, 70, 42, 255);
        SDL_Rect book = {px + 5, py + 4, 14, 13};
        SDL_RenderFillRect(r->sdl, &book);
        SDL_SetRenderDrawColor(r->sdl, 224, 216, 186, 255);
        SDL_RenderDrawLine(r->sdl, px + 8, py + 6, px + 8, py + 15);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 9, px + 16, py + 9);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 12, px + 16, py + 12);
    } else if (type == TILE_CATACOMBS_SARCOPHAGUS) {
        SDL_Rect tomb = {px + 4, py + 2, 16, 21};
        SDL_SetRenderDrawColor(r->sdl, 102, 103, 105, 255);
        SDL_RenderFillRect(r->sdl, &tomb);
        SDL_SetRenderDrawColor(r->sdl, 171, 159, 118, 255);
        SDL_RenderDrawRect(r->sdl, &tomb);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 6, px + 12, py + 18);
        SDL_RenderDrawLine(r->sdl, px + 8, py + 10, px + 16, py + 10);
    } else if (type == TILE_BURIAL_PLATE) {
        SDL_Rect plate = {px + 3, py + 3, 18, 18};
        SDL_SetRenderDrawColor(r->sdl, 146, 113, 65, 255);
        SDL_RenderDrawRect(r->sdl, &plate);
        SDL_RenderDrawLine(r->sdl, px + 7, py + 8, px + 12, py + 15);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 15, px + 17, py + 8);
    } else if (!wall && shade == 0) {
        SDL_SetRenderDrawColor(r->sdl, 83, 79, 72, 255);
        SDL_RenderDrawLine(r->sdl, px + 5, py + 16, px + 16, py + 19);
    }
}

static void draw_crownroad_tile(Renderer *r, int sx, int sy, int x, int y, int kind) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_Rect tile = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_Color ground = {49, 50, 38, 255};
    SDL_Color stone = {83, 82, 75, 255};
    SDL_Color rubble = {38, 38, 38, 255};
    SDL_Color base = kind == 1 ? stone : (kind == 2 ? rubble : ground);
    SDL_SetRenderDrawColor(r->sdl, base.r, base.g, base.b, 255);
    SDL_RenderFillRect(r->sdl, &tile);
    if (kind == 1) {
        SDL_SetRenderDrawColor(r->sdl, 46, 47, 43, 255);
        SDL_RenderDrawLine(r->sdl, px, py + 9, px + 19, py + 9);
        SDL_RenderDrawLine(r->sdl, px + 9, py, px + 9, py + 8);
        SDL_RenderDrawLine(r->sdl, px + 5, py + 10, px + 5, py + 19);
        SDL_RenderDrawLine(r->sdl, px + 15, py + 10, px + 15, py + 19);
        SDL_SetRenderDrawColor(r->sdl, 115, 111, 98, 255);
        SDL_RenderDrawLine(r->sdl, px + 1, py + 1, px + 7, py + 1);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 11, px + 18, py + 11);
    } else if (kind == 2) {
        SDL_Rect block = {px + 2, py + 5, 16, 11};
        SDL_SetRenderDrawColor(r->sdl, 90, 84, 75, 255);
        SDL_RenderFillRect(r->sdl, &block);
        SDL_SetRenderDrawColor(r->sdl, 42, 40, 38, 255);
        SDL_RenderDrawLine(r->sdl, px + 2, py + 11, px + 17, py + 11);
        SDL_RenderDrawLine(r->sdl, px + 9, py + 5, px + 9, py + 10);
    } else {
        int seed = (x * 17 + y * 31) & 7;
        SDL_SetRenderDrawColor(r->sdl, 91, 79, 50, 255);
        SDL_RenderDrawLine(r->sdl, px + 3 + seed, py + 12,
            px + 3 + seed, py + 15);
        SDL_SetRenderDrawColor(r->sdl, 31, 38, 31, 255);
        SDL_RenderDrawPoint(r->sdl, px + 15 - seed, py + 6);
        SDL_RenderDrawPoint(r->sdl, px + 11, py + 18);
    }
}

static void draw_castle_front(Renderer *r, int sx, int sy) {
    if (!r->castle_texture) {
        return;
    }
    SDL_Rect destination = {sx * TILE_SIZE, sy * TILE_SIZE,
        TOWN_MOAT_W * TILE_SIZE, TOWN_MOAT_H * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->castle_texture, NULL, &destination);
}

static void draw_dialogue_text(Renderer *r, const char *text, int x, int y, int max_chars, SDL_Color color) {
    char line[64];
    int line_len = 0;
    const char *cursor = text;

    while (*cursor) {
        while (*cursor == ' ') {
            cursor++;
        }
        const char *word = cursor;
        int word_len = 0;
        while (cursor[word_len] && cursor[word_len] != ' ') {
            word_len++;
        }
        if (word_len == 0) {
            break;
        }
        if (line_len > 0 && line_len + word_len + 1 > max_chars) {
            line[line_len] = '\0';
            renderer_draw_text(r, line, x, y, color, r->font_tiny);
            y += 16;
            line_len = 0;
        }
        if (line_len > 0) {
            line[line_len++] = ' ';
        }
        if (word_len > (int)sizeof(line) - line_len - 1) {
            word_len = (int)sizeof(line) - line_len - 1;
        }
        memcpy(line + line_len, word, word_len);
        line_len += word_len;
        cursor += word_len;
    }
    if (line_len > 0) {
        line[line_len] = '\0';
        renderer_draw_text(r, line, x, y, color, r->font_tiny);
    }
}

static void draw_dialogue_bubble(Renderer *r, const GameState *g, const Viewport *v) {
    int shortcut = game_shortcut_prompt_active(g);
    int quest_offer = game_quest_offer_active(g);
    if (!g->dialogue_active ||
        (!shortcut && !town_life_is_interior(g->location) && g->location != LOCATION_TAVERN &&
        g->location != LOCATION_INN &&
        g->location != LOCATION_GUILD &&
        g->location != LOCATION_TOWN_HALL &&
        g->location != LOCATION_TOWN &&
        g->location != LOCATION_TOWN2 &&
        g->location != LOCATION_TOWN3 &&
        g->location != LOCATION_TOWN4 &&
        g->location != LOCATION_CASTLE_INTERIOR &&
        g->location != LOCATION_CASTLE &&
        g->location != LOCATION_JAIL &&
        g->location != LOCATION_ISLAND &&
        g->location != LOCATION_ESCAPE_TUNNEL &&
        g->location != LOCATION_FOREST &&
        g->location != LOCATION_SWAMP &&
        g->location != LOCATION_FROSTFELL &&
        g->location != LOCATION_GLASSDEEP)) {
        return;
    }
    int npc_x = shortcut ? g->player.x : g->dialogue_x;
    int npc_y = shortcut ? g->player.y : g->dialogue_y;
    if (!viewport_is_visible(v, npc_x, npc_y)) {
        return;
    }

    int viewport_w = r->screen_w - INFO_PANEL_W;
    if (g->location == LOCATION_TOWN || g->location == LOCATION_TOWN2 ||
        g->location == LOCATION_TOWN3 || g->location == LOCATION_TOWN4 ||
        g->location == LOCATION_CASTLE) {
        viewport_w = TOWN_W * TILE_SIZE;
    } else if (g->location == LOCATION_TAVERN || g->location == LOCATION_INN ||
        g->location == LOCATION_TOWN_HALL || g->location == LOCATION_GUILD ||
        town_life_is_interior(g->location)) {
        viewport_w = TAVERN_W * TILE_SIZE;
    } else if (g->location == LOCATION_ISLAND) {
        viewport_w = ISLAND_W * TILE_SIZE;
    }
    int bubble_w = viewport_w < 460 ? viewport_w - 16 : 440;
    int bubble_h = shortcut || quest_offer || game_glassdeep_prompt_active(g) ? 132 : 98;
    int npc_screen_x = viewport_to_screen_x(v, npc_x) * TILE_SIZE +
        TILE_SIZE / 2;
    int npc_screen_y = viewport_to_screen_y(v, npc_y) * TILE_SIZE;
    int bubble_x = npc_screen_x - bubble_w / 2;
    int bubble_y = npc_screen_y - bubble_h - 22;
    // An NPC near the top of the map gets the bubble below instead of under it.
    int below = bubble_y < 8;
    if (below) {
        bubble_y = npc_screen_y + TILE_SIZE + 22;
    }
    if (bubble_x < 8) {
        bubble_x = 8;
    }
    if (bubble_x + bubble_w > viewport_w - 8) {
        bubble_x = viewport_w - bubble_w - 8;
    }

    SDL_Rect border = {bubble_x, bubble_y, bubble_w, bubble_h};
    SDL_Rect panel = {bubble_x + 3, bubble_y + 3, bubble_w - 6,
        bubble_h - 6};
    SDL_SetRenderDrawColor(r->sdl, 25, 20, 27, 255);
    SDL_RenderFillRect(r->sdl, &border);
    SDL_SetRenderDrawColor(r->sdl, 236, 224, 190, 255);
    SDL_RenderFillRect(r->sdl, &panel);

    int tail_x = npc_screen_x;
    if (tail_x < bubble_x + 18) {
        tail_x = bubble_x + 18;
    }
    if (tail_x > bubble_x + bubble_w - 18) {
        tail_x = bubble_x + bubble_w - 18;
    }
    for (int row = 0; row < 15; row++) {
        int half_width = (15 - row) / 2;
        int tail_y = below ? bubble_y - 1 - row : bubble_y + bubble_h + row;
        SDL_Rect tail = {tail_x - half_width, tail_y, half_width * 2 + 1, 1};
        SDL_SetRenderDrawColor(r->sdl, 25, 20, 27, 255);
        SDL_RenderFillRect(r->sdl, &tail);
    }
    for (int row = 0; row < 11; row++) {
        int half_width = (11 - row) / 2;
        int tail_y = below ? bubble_y - 1 - row : bubble_y + bubble_h + row;
        SDL_Rect tail = {tail_x - half_width, tail_y, half_width * 2 + 1, 1};
        SDL_SetRenderDrawColor(r->sdl, 236, 224, 190, 255);
        SDL_RenderFillRect(r->sdl, &tail);
    }

    renderer_draw_text(r, g->dialogue_speaker, bubble_x + 14,
        bubble_y + 12, (SDL_Color){71, 82, 138, 255}, r->font_small);
    draw_dialogue_text(r, g->dialogue_text, bubble_x + 14, bubble_y + 36,
        (bubble_w - 28) / 8, (SDL_Color){42, 32, 30, 255});
    if (shortcut) {
        renderer_draw_text(r, "Enter to continue", bubble_x + 14, bubble_y + bubble_h - 20,
            (SDL_Color){71, 82, 138, 255}, r->font_tiny);
    } else if (quest_offer) {
        renderer_draw_text(r, "Accept quest? Y: Yes   N: No", bubble_x + 14, bubble_y + bubble_h - 20,
            (SDL_Color){71, 82, 138, 255}, r->font_tiny);
    }
}

static void draw_weapon_arrow_at(Renderer *r, int cx, int cy, int dx, int dy, int impact) {
    int px = -dy;
    int py = dx;
    int tail_x = cx - dx * 7;
    int tail_y = cy - dy * 7;
    int tip_x = cx + dx * 7;
    int tip_y = cy + dy * 7;
    int head_x = tip_x - dx * 4;
    int head_y = tip_y - dy * 4;

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 18, 16, 24, 220);
    SDL_RenderDrawLine(r->sdl, tail_x + px, tail_y + py,
        tip_x + px, tip_y + py);
    SDL_RenderDrawLine(r->sdl, tail_x - px, tail_y - py,
        tip_x - px, tip_y - py);

    SDL_SetRenderDrawColor(r->sdl, 142, 92, 46, 255);
    SDL_RenderDrawLine(r->sdl, tail_x, tail_y, tip_x, tip_y);

    SDL_SetRenderDrawColor(r->sdl, 210, 216, 220, 255);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x + px * 3, head_y + py * 3);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x - px * 3, head_y - py * 3);

    SDL_SetRenderDrawColor(r->sdl, 172, 54, 42, 255);
    SDL_RenderDrawLine(r->sdl, tail_x + dx * 2, tail_y + dy * 2,
        tail_x + px * 3, tail_y + py * 3);
    SDL_RenderDrawLine(r->sdl, tail_x + dx * 2, tail_y + dy * 2,
        tail_x - px * 3, tail_y - py * 3);

    if (impact) {
        SDL_SetRenderDrawColor(r->sdl, 232, 196, 112, 210);
        SDL_RenderDrawPoint(r->sdl, tip_x + px * 3, tip_y + py * 3);
        SDL_RenderDrawPoint(r->sdl, tip_x - px * 3, tip_y - py * 3);
    }
}

void game_draw_enemy_projectiles(Renderer *r, const EnemyProjectiles *shots, const Viewport *v, Uint32 elapsed) {
    if (elapsed >= ENEMY_PROJECTILE_TOTAL_MS || shots->count == 0) {
        return;
    }
    SDL_Rect old_clip;
    SDL_bool clipped = SDL_RenderIsClipEnabled(r->sdl);
    SDL_RenderGetClipRect(r->sdl, &old_clip);
    SDL_BlendMode old_blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &old_blend);
    SDL_Rect play_area = {0, 0, r->screen_w - INFO_PANEL_W, r->tiles_y * TILE_SIZE};
    SDL_RenderSetClipRect(r->sdl, &play_area);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    int impact = elapsed >= ENEMY_PROJECTILE_TRAVEL_MS;
    float progress = impact ? 1.0f : (float)elapsed / ENEMY_PROJECTILE_TRAVEL_MS;
    for (int i = 0; i < shots->count; i++) {
        const EnemyProjectile *shot = &shots->shots[i];
        int dx = shot->target_x - shot->start_x;
        int dy = shot->target_y - shot->start_y;
        int length = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        if (length == 0) {
            continue;
        }
        float vx = (float)dx / length;
        float vy = (float)dy / length;
        int cx = (int)((shot->start_x - v->cam_x + dx * progress) * TILE_SIZE) + TILE_SIZE / 2;
        int cy = (int)((shot->start_y - v->cam_y + dy * progress) * TILE_SIZE) + TILE_SIZE / 2;
        SDL_Color color = {184, 100, 255, 255};
        if (shot->type == ENEMY_FOREST_NECROMANCER) {
            color = (SDL_Color){100, 240, 130, 255};
        } else if (shot->type == ENEMY_WATER_ELEMENTAL || shot->type == ENEMY_DROWNED_QUEEN) {
            color = (SDL_Color){80, 210, 255, 255};
        } else if (shot->type == ENEMY_SIREN) {
            color = (SDL_Color){240, 130, 220, 255};
        } else if (shot->type == ENEMY_GOBLIN_BOMBER) {
            color = (SDL_Color){255, 160, 55, 255};
        } else if (shot->type == ENEMY_SUN_PRIEST ||
            shot->type == ENEMY_DESERT_PHARAOH ||
            shot->type == ENEMY_FALLEN_SUN_GUARDIAN) {
            color = (SDL_Color){255, 188, 45, 255};
        } else if (shot->type == ENEMY_SERPENT_SPIRIT ||
            shot->type == ENEMY_MOONBOUND_SENTINEL) {
            color = (SDL_Color){75, 224, 232, 255};
        } else if (shot->type == ENEMY_FROST_ARCHER) {
            color = (SDL_Color){168, 228, 255, 255};
        } else if (shot->type == ENEMY_FEY_TRICKSTER || shot->type == ENEMY_LIVING_FLOWER ||
            shot->type == ENEMY_THORN_REGENT) {
            color = (SDL_Color){173, 240, 211, 255};
        } else if (shot->type == ENEMY_PRISM_SOVEREIGN) {
            color = (SDL_Color){177, 147, 255, 255};
        } else if (shot->type == ENEMY_DJINN) {
            color = (SDL_Color){75, 224, 232, 255};
        }
        if (impact) {
            int radius = 4 + (int)(elapsed - ENEMY_PROJECTILE_TRAVEL_MS) / 15;
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 220);
            SDL_RenderDrawLine(r->sdl, cx - radius, cy, cx + radius, cy);
            SDL_RenderDrawLine(r->sdl, cx, cy - radius, cx, cy + radius);
            SDL_Rect burst = {cx - radius / 2, cy - radius / 2, radius, radius};
            SDL_RenderDrawRect(r->sdl, &burst);
        } else if (shot->type == ENEMY_PRISM_SOVEREIGN) {
            int start_x = viewport_to_screen_x(v, shot->start_x) * TILE_SIZE + TILE_SIZE / 2;
            int start_y = viewport_to_screen_y(v, shot->start_y) * TILE_SIZE + TILE_SIZE / 2;
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_RenderDrawLine(r->sdl, start_x - (int)vy, start_y + (int)vx, cx - (int)vy, cy + (int)vx);
            SDL_RenderDrawLine(r->sdl, start_x + (int)vy, start_y - (int)vx, cx + (int)vy, cy - (int)vx);
            SDL_SetRenderDrawColor(r->sdl, 220, 255, 255, 255);
            SDL_RenderDrawLine(r->sdl, start_x, start_y, cx, cy);
        } else if (shot->type == ENEMY_GOBLIN_ARCHER ||
            shot->type == ENEMY_ROAD_ARCHER || shot->type == ENEMY_GRAVE_ARCHER ||
            shot->type == ENEMY_FROST_ARCHER ||
            shot->type == ENEMY_DARK_ELF ||
            shot->type == ENEMY_BLOWDART_HUNTER) {
            draw_weapon_arrow_at(r, cx, cy, (dx > 0) - (dx < 0), (dy > 0) - (dy < 0), 0);
        } else if (shot->type == ENEMY_GOBLIN_BOMBER) {
            SDL_Rect bomb = {cx - 5, cy - 5, 10, 10};
            SDL_SetRenderDrawColor(r->sdl, 36, 32, 40, 255);
            SDL_RenderFillRect(r->sdl, &bomb);
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_RenderDrawRect(r->sdl, &bomb);
            SDL_RenderDrawLine(r->sdl, cx, cy - 5, cx + 4, cy - 9);
            SDL_RenderDrawPoint(r->sdl, cx + 5, cy - 10);
        } else if (shot->type == ENEMY_MOUNTAIN_GOBLIN_KING) {
            SDL_SetRenderDrawColor(r->sdl, 160, 105, 55, 255);
            SDL_RenderDrawLine(r->sdl, cx - (int)(vx * 8), cy - (int)(vy * 8), cx + (int)(vx * 5), cy + (int)(vy * 5));
            SDL_Rect blade = {cx + (int)(vx * 4) - 5, cy + (int)(vy * 4) - 5, 10, 10};
            SDL_SetRenderDrawColor(r->sdl, 210, 220, 225, 255);
            SDL_RenderFillRect(r->sdl, &blade);
        } else {
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 150);
            for (int offset = -2; offset <= 2; offset++) {
                SDL_RenderDrawLine(r->sdl, cx - (int)(vx * 14) - (int)(vy * offset),
                    cy - (int)(vy * 14) + (int)(vx * offset), cx, cy);
            }
            SDL_Rect core = {cx - 3, cy - 3, 6, 6};
            SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(r->sdl, &core);
            SDL_SetRenderDrawColor(r->sdl, 240, 250, 255, 255);
            SDL_RenderDrawPoint(r->sdl, cx, cy);
        }
    }
    SDL_RenderSetClipRect(r->sdl, clipped ? &old_clip : NULL);
    SDL_SetRenderDrawBlendMode(r->sdl, old_blend);
}

static Uint32 combat_feedback_start(const CombatFeedbackEvent *e) {
    if (e->arrival == FEEDBACK_AFTER_PLAYER_SHOT) {
        return e->created_at + SPELL_TRAVEL_MS;
    }
    if (e->arrival == FEEDBACK_AFTER_ENEMY_SHOT) {
        return e->created_at + ENEMY_PROJECTILE_TRAVEL_MS;
    }
    return e->created_at;
}

int game_combat_feedback_active(Uint32 now) {
    for (int i = 0; i < combat_feedback_count(); i++) {
        if (now < combat_feedback_start(combat_feedback_get(i)) + COMBAT_FEEDBACK_MS) {
            return 1;
        }
    }
    return 0;
}

// A dark outline keeps the text readable over any floor.
static void draw_feedback_text(Renderer *r, TTF_Font *font, const char *text, int cx, int y, SDL_Color color, Uint8 alpha) {
    if (!font) {
        return;
    }
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, (SDL_Color){255, 255, 255, 255});
    if (!surface) {
        return;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(r->sdl, surface);
    SDL_Rect dst = {cx - surface->w / 2, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    if (!texture) {
        return;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_SetTextureColorMod(texture, 0, 0, 0);
    static const int outline[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (int i = 0; i < 4; i++) {
        SDL_Rect edge = {dst.x + outline[i][0], dst.y + outline[i][1], dst.w, dst.h};
        SDL_RenderCopy(r->sdl, texture, NULL, &edge);
    }
    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_RenderCopy(r->sdl, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}

// Frozen enemies get an icy tint, a snowflake and the turns they stay frozen.
static void draw_frozen_status(Renderer *r, int left, int top, int turns) {
    SDL_Rect tile = {left, top, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 140, 210, 255, 70);
    SDL_RenderFillRect(r->sdl, &tile);
    draw_icon_frozen(r, left + 2, top + 2);
    char text[8];
    SDL_snprintf(text, sizeof(text), "%d", turns);
    draw_feedback_text(r, r->font_tiny, text, left + 14, top + 1, (SDL_Color){210, 245, 255, 255}, 255);
}

static int combat_feedback_rise(Uint32 age) {
    return (int)(age * 16 / COMBAT_FEEDBACK_MS);
}

// Lifts each result so it never covers one still rising from the same tile.
// Results are placed in start order, each at the lowest clear height.
static void combat_feedback_lifts(int lifts[MAX_COMBAT_FEEDBACK]) {
    int count = combat_feedback_count();
    int placed[MAX_COMBAT_FEEDBACK] = {0};
    for (int k = 0; k < count; k++) {
        int next = -1;
        for (int i = 0; i < count; i++) {
            if (!placed[i] && (next < 0 || combat_feedback_start(combat_feedback_get(i)) <
                combat_feedback_start(combat_feedback_get(next)))) {
                next = i;
            }
        }
        const CombatFeedbackEvent *e = combat_feedback_get(next);
        Uint32 start = combat_feedback_start(e);
        int lift = 0;
        int moved = 1;
        while (moved) {
            moved = 0;
            for (int j = 0; j < count; j++) {
                const CombatFeedbackEvent *other = combat_feedback_get(j);
                Uint32 other_start = combat_feedback_start(other);
                if (!placed[j] || other->location != e->location || other->level != e->level ||
                    other->x != e->x || other->y != e->y || start - other_start >= COMBAT_FEEDBACK_MS) {
                    continue;
                }
                // Critical numbers use the larger font and need a taller line.
                int line = e->kind == FEEDBACK_ENEMY_CRITICAL ||
                    other->kind == FEEDBACK_ENEMY_CRITICAL ? 13 : 10;
                int height = lifts[j] + combat_feedback_rise(start - other_start);
                if (abs(height - lift) < line) {
                    lift = height + line;
                    moved = 1;
                }
            }
        }
        lifts[next] = lift;
        placed[next] = 1;
    }
}

// Gold rays burst outward from the center of a critically hit enemy.
static void draw_critical_burst(Renderer *r, int cx, int cy, Uint32 age) {
    static const int rays[8][2] = {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
    Uint32 half = CRITICAL_BURST_MS / 2;
    int inner = 4 + (int)(age * 6 / CRITICAL_BURST_MS);
    // Full strength for the first half, then fade out.
    Uint8 alpha = age < half ? 255 : (Uint8)((CRITICAL_BURST_MS - age) * 255 / half);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 8; i++) {
        int dx = rays[i][0];
        int dy = rays[i][1];
        // Diagonal rays start closer and are shorter so the burst reads as round.
        int diagonal = dx != 0 && dy != 0;
        int from = diagonal ? inner * 7 / 10 : inner;
        int to = from + (diagonal ? 3 : 5);
        int x0 = cx + dx * from;
        int y0 = cy + dy * from;
        int x1 = cx + dx * to;
        int y1 = cy + dy * to;
        // Rays are two pixels wide with a dark edge on one side.
        int side_x = dy != 0 ? 1 : 0;
        int side_y = dy == 0 ? 1 : 0;
        SDL_SetRenderDrawColor(r->sdl, 120, 72, 12, alpha);
        SDL_RenderDrawLine(r->sdl, x0 + side_x * 2, y0 + side_y * 2, x1 + side_x * 2, y1 + side_y * 2);
        SDL_SetRenderDrawColor(r->sdl, 255, 214, 72, alpha);
        SDL_RenderDrawLine(r->sdl, x0, y0, x1, y1);
        SDL_RenderDrawLine(r->sdl, x0 + side_x, y0 + side_y, x1 + side_x, y1 + side_y);
    }
    if (age < half) {
        int size = age < half / 2 ? 6 : 4;
        SDL_Rect flash = {cx - size / 2, cy - size / 2, size, size};
        SDL_SetRenderDrawColor(r->sdl, 255, 246, 200, alpha);
        SDL_RenderFillRect(r->sdl, &flash);
    }
}

// A blue shield pops over the player when a shield absorbs part of a hit.
static void draw_block_shield(Renderer *r, int cx, int cy, Uint32 age) {
    static const char *shield[12] = {
        "oooooooooo",
        "olllllllbo",
        "olbbbwbbbo",
        "olbbbwbbbo",
        "olwwwwwwbo",
        "olbbbwbbbo",
        "olbbbwbbbo",
        ".olbbwbbo.",
        ".olbbwbbo.",
        "..olbwbo..",
        "...obbo...",
        "....oo....",
    };
    Uint32 half = BLOCK_SHIELD_MS / 2;
    Uint8 alpha = age < half ? 255 : (Uint8)((BLOCK_SHIELD_MS - age) * 255 / half);
    // Double size for the first few frames makes the shield pop into view.
    int scale = age < 50 ? 2 : 1;
    int left = cx - 5 * scale;
    int top = cy - 6 * scale;
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int row = 0; row < 12; row++) {
        for (int col = 0; col < 10; col++) {
            char c = shield[row][col];
            if (c == 'o') {
                SDL_SetRenderDrawColor(r->sdl, 18, 26, 52, alpha);
            } else if (c == 'l') {
                SDL_SetRenderDrawColor(r->sdl, 170, 205, 255, alpha);
            } else if (c == 'b') {
                SDL_SetRenderDrawColor(r->sdl, 70, 120, 200, alpha);
            } else if (c == 'w') {
                SDL_SetRenderDrawColor(r->sdl, 230, 240, 255, alpha);
            } else {
                continue;
            }
            SDL_Rect pixel = {left + col * scale, top + row * scale, scale, scale};
            SDL_RenderFillRect(r->sdl, &pixel);
        }
    }
}

// A struck tile flashes a colour that fades out quickly. The colour's alpha is its peak.
static void draw_tile_flash(Renderer *r, int left, int top, SDL_Color color, Uint32 age) {
    Uint8 alpha = (Uint8)(color.a * (HIT_FLASH_MS - age) / HIT_FLASH_MS);
    SDL_Rect tile = {left, top, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, alpha);
    SDL_RenderFillRect(r->sdl, &tile);
}

// A white slash cuts down across an enemy struck in melee, then fades.
static void draw_melee_slash(Renderer *r, int left, int top, Uint32 age) {
    Uint32 draw_in = SLASH_MS / 3;
    Uint32 hold = SLASH_MS * 2 / 3;
    int length = age < draw_in ? (int)(16 * age / draw_in) : 16;
    Uint8 alpha = age < hold ? 255 : (Uint8)((SLASH_MS - age) * 255 / (SLASH_MS - hold));
    int x0 = left + 20;
    int y0 = top + 3;
    int x1 = x0 - length;
    int y1 = y0 + length;
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    // Three pixels wide with a dark edge along its lower side.
    SDL_SetRenderDrawColor(r->sdl, 24, 24, 36, alpha);
    SDL_RenderDrawLine(r->sdl, x0 + 2, y0 + 1, x1 + 2, y1 + 1);
    SDL_SetRenderDrawColor(r->sdl, 255, 255, 255, alpha);
    for (int offset = -1; offset <= 1; offset++) {
        SDL_RenderDrawLine(r->sdl, x0 + offset, y0, x1 + offset, y1);
    }
}

static int combat_feedback_shown(const CombatFeedbackEvent *e, const GameState *g, const Viewport *v, Uint32 now) {
    Uint32 start = combat_feedback_start(e);
    return e->location == g->location && e->level == g->level &&
        now >= start && now - start < COMBAT_FEEDBACK_MS &&
        viewport_is_visible(v, e->x, e->y);
}

int game_player_hit_flash(const GameState *g, Uint32 now) {
    for (int i = 0; i < combat_feedback_count(); i++) {
        const CombatFeedbackEvent *e = combat_feedback_get(i);
        Uint32 start = combat_feedback_start(e);
        if (e->kind == FEEDBACK_PLAYER_DAMAGE && e->location == g->location &&
            e->level == g->level && now >= start && now - start < HP_FLASH_MS) {
            return 1;
        }
    }
    return 0;
}

static void draw_combat_feedback(Renderer *r, const GameState *g, const Viewport *v) {
    Uint32 now = SDL_GetTicks();
    // Tile effects go down first so every number stays on top of them.
    for (int i = 0; i < combat_feedback_count(); i++) {
        const CombatFeedbackEvent *e = combat_feedback_get(i);
        if (!combat_feedback_shown(e, g, v, now)) {
            continue;
        }
        Uint32 age = now - combat_feedback_start(e);
        int left = viewport_to_screen_x(v, e->x) * TILE_SIZE;
        int top = viewport_to_screen_y(v, e->y) * TILE_SIZE;
        int cx = left + TILE_SIZE / 2;
        int cy = top + TILE_SIZE / 2;
        int melee_hit = e->arrival == FEEDBACK_NOW &&
            (e->kind == FEEDBACK_ENEMY_DAMAGE || e->kind == FEEDBACK_ENEMY_CRITICAL);
        if (melee_hit && age < HIT_FLASH_MS) {
            draw_tile_flash(r, left, top, (SDL_Color){255, 255, 255, 130}, age);
        }
        if (melee_hit && age < SLASH_MS) {
            draw_melee_slash(r, left, top, age);
        }
        if (e->kind == FEEDBACK_PLAYER_DAMAGE && age < HIT_FLASH_MS) {
            draw_tile_flash(r, left, top, (SDL_Color){230, 40, 30, 120}, age);
        }
        if (e->kind == FEEDBACK_ENEMY_CRITICAL && age < CRITICAL_BURST_MS) {
            draw_critical_burst(r, cx, cy, age);
        } else if (e->kind == FEEDBACK_BLOCK && age < BLOCK_SHIELD_MS) {
            draw_block_shield(r, cx, cy, age);
        }
    }
    int lifts[MAX_COMBAT_FEEDBACK];
    combat_feedback_lifts(lifts);
    for (int i = 0; i < combat_feedback_count(); i++) {
        const CombatFeedbackEvent *e = combat_feedback_get(i);
        if (!combat_feedback_shown(e, g, v, now)) {
            continue;
        }
        char text[16];
        SDL_Color color = {245, 245, 235, 255};
        TTF_Font *font = r->font_tiny;
        if (e->kind == FEEDBACK_ENEMY_CRITICAL) {
            SDL_snprintf(text, sizeof(text), "-%d!", e->amount);
            color = (SDL_Color){255, 206, 64, 255};
            font = r->font_small;
        } else if (e->kind == FEEDBACK_PLAYER_DAMAGE) {
            SDL_snprintf(text, sizeof(text), "-%d", e->amount);
            color = (SDL_Color){240, 72, 60, 255};
        } else if (e->kind == FEEDBACK_HEAL) {
            SDL_snprintf(text, sizeof(text), "+%d", e->amount);
            color = (SDL_Color){96, 224, 112, 255};
        } else if (e->kind == FEEDBACK_MANA_LOSS) {
            SDL_snprintf(text, sizeof(text), "-%d MP", e->amount);
            color = (SDL_Color){112, 164, 255, 255};
        } else if (e->kind == FEEDBACK_BLOCK) {
            SDL_snprintf(text, sizeof(text), "BLOCK");
            color = (SDL_Color){150, 190, 255, 255};
        } else if (e->kind == FEEDBACK_DODGE) {
            SDL_snprintf(text, sizeof(text), "DODGE");
            color = (SDL_Color){150, 235, 245, 255};
        } else if (e->kind == FEEDBACK_MISS) {
            SDL_snprintf(text, sizeof(text), "MISS");
            color = (SDL_Color){200, 200, 210, 255};
        } else {
            SDL_snprintf(text, sizeof(text), "-%d", e->amount);
        }
        // Numbers rise for their whole life and fade during the second half.
        Uint32 age = now - combat_feedback_start(e);
        Uint32 remaining = COMBAT_FEEDBACK_MS - age;
        Uint8 alpha = remaining * 2 >= COMBAT_FEEDBACK_MS ? 255 : (Uint8)(remaining * 2 * 255 / COMBAT_FEEDBACK_MS);
        int x = viewport_to_screen_x(v, e->x) * TILE_SIZE + TILE_SIZE / 2;
        int y = viewport_to_screen_y(v, e->y) * TILE_SIZE + 2 - combat_feedback_rise(age) - lifts[i];
        if (y < 0) {
            y = 0;
        }
        draw_feedback_text(r, font, text, x, y, color, alpha);
    }
}

static void draw_magic_arrow(Renderer *r, int tile_x, int tile_y,
                             int dx, int dy, int impact, int frame) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int px = -dy;
    int py = dx;
    int tail_x = cx - dx * 7;
    int tail_y = cy - dy * 7;
    int tip_x = cx + dx * 7;
    int tip_y = cy + dy * 7;
    int head_x = tip_x - dx * 4;
    int head_y = tip_y - dy * 4;

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 30, 92, 230, 90);
    SDL_RenderDrawLine(r->sdl, tail_x + px * 2, tail_y + py * 2,
        tip_x + px * 2, tip_y + py * 2);
    SDL_RenderDrawLine(r->sdl, tail_x - px * 2, tail_y - py * 2,
        tip_x - px * 2, tip_y - py * 2);

    SDL_SetRenderDrawColor(r->sdl, 54, 126, 255, 255);
    SDL_RenderDrawLine(r->sdl, tail_x, tail_y, tip_x, tip_y);
    SDL_SetRenderDrawColor(r->sdl, 176, 246, 255, 255);
    SDL_RenderDrawPoint(r->sdl, cx, cy);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x + px * 3, head_y + py * 3);
    SDL_RenderDrawLine(r->sdl, tip_x, tip_y,
        head_x - px * 3, head_y - py * 3);

    SDL_SetRenderDrawColor(r->sdl, 76, 178, 255, 210);
    SDL_RenderDrawPoint(r->sdl,
        tail_x - dx * 2 + px * (frame % 2 ? 2 : -2),
        tail_y - dy * 2 + py * (frame % 2 ? 2 : -2));
    SDL_RenderDrawPoint(r->sdl,
        tail_x - dx * 4 - px * (frame % 2 ? 1 : -1),
        tail_y - dy * 4 - py * (frame % 2 ? 1 : -1));

    if (impact) {
        SDL_SetRenderDrawColor(r->sdl, 198, 250, 255, 230);
        SDL_RenderDrawLine(r->sdl, tip_x - px * 3, tip_y - py * 3,
            tip_x + px * 3, tip_y + py * 3);
    }
}

static void draw_demonic_sword(Renderer *r, int tile_x, int tile_y, int dx, int dy, int impact, int frame) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2 + dx * (impact ? 2 : frame % 2);
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2 + dy * (impact ? 2 : frame % 2);
    int px = -dy;
    int py = dx;
    int base_x = cx - dx * 2;
    int base_y = cy - dy * 2;
    int tip_x = cx + dx * 10;
    int tip_y = cy + dy * 10;
    int guard_x = cx - dx * 4;
    int guard_y = cy - dy * 4;

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 109, 15, 24, 105);
    SDL_RenderDrawLine(r->sdl, base_x + px * 3, base_y + py * 3,
        tip_x + px, tip_y + py);
    SDL_RenderDrawLine(r->sdl, base_x - px * 3, base_y - py * 3,
        tip_x - px, tip_y - py);
    SDL_SetRenderDrawColor(r->sdl, 208, 35, 45, 255);
    SDL_RenderDrawLine(r->sdl, base_x + px, base_y + py, tip_x, tip_y);
    SDL_RenderDrawLine(r->sdl, base_x - px, base_y - py, tip_x, tip_y);
    SDL_SetRenderDrawColor(r->sdl, 255, 115, 92, 255);
    SDL_RenderDrawLine(r->sdl, base_x, base_y, tip_x, tip_y);
    SDL_SetRenderDrawColor(r->sdl, 67, 23, 35, 255);
    SDL_RenderDrawLine(r->sdl, guard_x + px * 5, guard_y + py * 5,
        guard_x - px * 5, guard_y - py * 5);
    SDL_RenderDrawLine(r->sdl, guard_x - dx, guard_y - dy,
        guard_x - dx * 6, guard_y - dy * 6);
    SDL_SetRenderDrawColor(r->sdl, 236, 72, 68, 255);
    SDL_RenderDrawLine(r->sdl, guard_x + px * 4, guard_y + py * 4,
        guard_x - px * 4, guard_y - py * 4);
    SDL_RenderDrawLine(r->sdl, guard_x - dx * 7 + px * 2,
        guard_y - dy * 7 + py * 2,
        guard_x - dx * 7 - px * 2, guard_y - dy * 7 - py * 2);

    if (impact) {
        SDL_SetRenderDrawColor(r->sdl, 255, 79, 69, 220);
        SDL_RenderDrawLine(r->sdl, cx + px * 8 - dx * 3,
            cy + py * 8 - dy * 3,
            cx - px * 8 + dx * 3, cy - py * 8 + dy * 3);
        SDL_SetRenderDrawColor(r->sdl, 255, 210, 153, 255);
        SDL_RenderDrawPoint(r->sdl, cx + px * 6 + dx * 4,
            cy + py * 6 + dy * 4);
        SDL_RenderDrawPoint(r->sdl, cx - px * 6 + dx * 4,
            cy - py * 6 + dy * 4);
    }
}

static void draw_fireball(Renderer *r, int tile_x, int tile_y,
                          int dx, int dy, int frame) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int flicker = frame % 2;
    SDL_Rect glow = {cx - 7, cy - 7, 14, 14};
    SDL_Rect flame = {cx - 5, cy - 5, 10, 10};
    SDL_Rect hot = {cx - 3, cy - 3, 6, 6};
    SDL_Rect core = {cx - 1, cy - 1, 3, 3};

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 194, 48, 18, 70);
    SDL_RenderFillRect(r->sdl, &glow);
    SDL_SetRenderDrawColor(r->sdl, 190, 44, 16, 255);
    SDL_RenderFillRect(r->sdl, &flame);
    SDL_SetRenderDrawColor(r->sdl, 250, 116, 18, 255);
    SDL_RenderFillRect(r->sdl, &hot);
    SDL_SetRenderDrawColor(r->sdl, 255, 238, 132, 255);
    SDL_RenderFillRect(r->sdl, &core);

    SDL_SetRenderDrawColor(r->sdl, 238, 78, 14, 220);
    for (int i = 1; i <= 3; i++) {
        int size = 5 - i;
        SDL_Rect ember = {
            cx - dx * (5 + i * 3) - size / 2 + (-dy) * (flicker ? i : -i),
            cy - dy * (5 + i * 3) - size / 2 + dx * (flicker ? i : -i),
            size, size
        };
        SDL_RenderFillRect(r->sdl, &ember);
    }
}

static void draw_fireball_impact(Renderer *r, int tile_x, int tile_y,
                                 Uint32 elapsed) {
    int cx = tile_x * TILE_SIZE + TILE_SIZE / 2;
    int cy = tile_y * TILE_SIZE + TILE_SIZE / 2;
    int radius = 5 + (int)(elapsed * (TILE_SIZE + 8) / 220);
    if (radius > TILE_SIZE + 8) {
        radius = TILE_SIZE + 8;
    }
    SDL_Rect ring = {cx - radius, cy - radius, radius * 2, radius * 2};
    SDL_Rect center = {cx - 5, cy - 5, 10, 10};
    Uint8 alpha = (Uint8)(220 - elapsed * 180 / 220);

    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r->sdl, 224, 58, 12, alpha);
    SDL_RenderDrawRect(r->sdl, &ring);
    ring.x += 2;
    ring.y += 2;
    ring.w -= 4;
    ring.h -= 4;
    SDL_SetRenderDrawColor(r->sdl, 255, 136, 18, alpha);
    SDL_RenderDrawRect(r->sdl, &ring);
    SDL_SetRenderDrawColor(r->sdl, 255, 224, 112, alpha);
    SDL_RenderFillRect(r->sdl, &center);

    SDL_SetRenderDrawColor(r->sdl, 250, 92, 12, alpha);
    SDL_RenderDrawPoint(r->sdl, cx + radius + 3, cy);
    SDL_RenderDrawPoint(r->sdl, cx - radius - 3, cy);
    SDL_RenderDrawPoint(r->sdl, cx, cy + radius + 3);
    SDL_RenderDrawPoint(r->sdl, cx, cy - radius - 3);
}

static TileType terrain_display_tile(const GameState *g, TileType tile) {
    // The shortcut uses mountain art without changing its saved terrain or exits.
    if (g->location == LOCATION_HIGH_PASS) {
        switch (tile) {
            case TILE_DRAGON_FLOOR: return TILE_MOUNTAIN_FLOOR;
            case TILE_DRAGON_WALL: return TILE_MOUNTAIN_WALL;
            case TILE_HIGH_PASS_ENTRANCE: return TILE_MOUNTAIN_ENTRANCE;
            case TILE_HIGH_PASS_EXIT: return TILE_MOUNTAIN_EXIT;
            default: break;
        }
    }
    return tile;
}

static TileType floor_item_underlay(const GameState *g, int x, int y) {
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            return terrain_display_tile(g, (TileType)item->underlying_tile);
        }
    }
    if (g->location == LOCATION_FOREST) {
        return TILE_FOREST_FLOOR;
    }
    if (g->location == LOCATION_MOUNTAINS || g->location == LOCATION_HIGH_PASS) {
        return TILE_MOUNTAIN_FLOOR;
    }
    if (g->location == LOCATION_DRAGONSPINE) {
        return TILE_DRAGON_FLOOR;
    }
    if (g->location == LOCATION_COAST) {
        return TILE_COAST_FLOOR;
    }
    if (g->location == LOCATION_FROSTFELL) {
        return TILE_FROST_FLOOR;
    }
    if (g->location == LOCATION_MOONVEIL) {
        return TILE_MOONVEIL_FLOOR;
    }
    if (g->location == LOCATION_ASHEN) {
        return TILE_ASHEN_FLOOR;
    }
    if (g->location == LOCATION_CATACOMBS) {
        return TILE_CATACOMBS_FLOOR;
    }
    if (g->location == LOCATION_GLASSDEEP) {
        return TILE_GLASSDEEP_FLOOR;
    }
    if (g->location == LOCATION_DESERT) {
        return TILE_DESERT_FLOOR;
    }
    if (g->location == LOCATION_SWAMP) {
        return TILE_SWAMP_FLOOR;
    }
    return TILE_FLOOR;
}

static int dungeon_is_floor(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    return tile == TILE_FLOOR || tile == TILE_STAIRS_UP ||
        tile == TILE_STAIRS_DOWN || tile == TILE_RETURN_EXIT ||
        tile == TILE_DUNGEON_STAIRS_SEALED || tile == TILE_DUNGEON_STAIRS_RETURN ||
        tile == TILE_DUNGEON_KEY || tile == TILE_CRYPT_KEY ||
        tile == TILE_CRYPT_CACHE || tile == TILE_PORTAL ||
        tile == TILE_BROKEN_BURIAL_SEAL ||
        tile == TILE_RESTORED_BURIAL_SEAL ||
        tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED ||
        tile == TILE_TRAP_SPIKE || tile == TILE_TRAP_FIRE ||
        tile == TILE_TRAP_POISON;
}

static int dungeon_wall_has_torch(const GameState *g, int x, int y) {
    if (y + 1 >= MAP_H || !dungeon_is_floor(g, x, y + 1)) {
        return 0;
    }
    unsigned int seed = (unsigned int)x * 2246822519u ^
        (unsigned int)y * 3266489917u;
    return seed % 11u == 0u;
}

static int forest_is_tree(TileType tile) {
    return tile == TILE_FOREST_WALL || tile == TILE_FOREST_HIDDEN_TRAIL;
}

static int forest_is_floor(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    return tile == TILE_FOREST_FLOOR || tile == TILE_FOREST_LANDMARK ||
        tile == TILE_FOREST_FALSE_MARKER || tile == TILE_FOREST_WARDEN ||
        tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED ||
        tile == TILE_TRAP_SPIKE || tile == TILE_TRAP_FIRE ||
        tile == TILE_TRAP_POISON;
}

static int swamp_is_floor(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    return tile == TILE_SWAMP_FLOOR || tile == TILE_SWAMP_ENTRANCE ||
        tile == TILE_SWAMP_EXIT;
}

static TileType coast_visible_tile(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        tile = floor_item_underlay(g, x, y);
    }
    if (tile == TILE_TRAP_HIDDEN || tile == TILE_TRAP_REVEALED ||
        tile == TILE_TRAP_SPIKE || tile == TILE_TRAP_FIRE ||
        tile == TILE_TRAP_POISON) {
        return map_coast_trap_underlay(&g->map, x, y);
    }
    return tile;
}

static int coast_is_water_surface(TileType tile) {
    return tile == TILE_COAST_SHALLOW_WATER ||
        tile == TILE_COAST_DEEP_WATER ||
        tile == TILE_COAST_CHANNEL_WATER ||
        tile == TILE_COAST_BEACON_UNLIT ||
        tile == TILE_COAST_BEACON_LIT;
}

static int mountain_crossing_neighbor(const Map *m, int x, int y, int chasm) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        return 0;
    }
    TileType tile = m->tiles[y][x];
    if (chasm) {
        return tile == TILE_MOUNTAIN_BRIDGE ||
            tile == TILE_MOUNTAIN_WEAK_BRIDGE;
    }
    return tile == TILE_MOUNTAIN_CHASM || map_is_walkable(m, x, y);
}

static unsigned int mountain_crossing_neighbors(const Map *m, int x, int y, int chasm) {
    unsigned int edges = 0;
    if (mountain_crossing_neighbor(m, x, y - 1, chasm)) {
        edges |= MOUNTAIN_EDGE_NORTH;
    }
    if (mountain_crossing_neighbor(m, x + 1, y, chasm)) {
        edges |= MOUNTAIN_EDGE_EAST;
    }
    if (mountain_crossing_neighbor(m, x, y + 1, chasm)) {
        edges |= MOUNTAIN_EDGE_SOUTH;
    }
    if (mountain_crossing_neighbor(m, x - 1, y, chasm)) {
        edges |= MOUNTAIN_EDGE_WEST;
    }
    return edges;
}

static void draw_coast_trap_underlay(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    TileType tile = map_coast_trap_underlay(&g->map, map_x, map_y);
    if (tile == TILE_COAST_SHALLOW_WATER) {
        draw_coast_shallow_water(r, screen_x, screen_y, map_x, map_y);
    } else if (tile == TILE_COAST_DEEP_WATER ||
        tile == TILE_COAST_DRAINED_WATER) {
        draw_coast_channel(r, screen_x, screen_y, map_x, map_y, 0,
            tile == TILE_COAST_DEEP_WATER);
    } else {
        draw_coast_floor(r, screen_x, screen_y, map_x, map_y);
    }
}

static void draw_floor_loot(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    int has_gold = 0;
    int has_item = 0;
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == map_x && item->y == map_y) {
            if (item->item.type == ITEM_GOLD) {
                has_gold = 1;
            } else {
                has_item = 1;
            }
        }
    }
    if (has_gold && has_item) {
        draw_floor_gold_and_item(r, screen_x, screen_y);
    } else if (has_gold) {
        draw_floor_gold(r, screen_x, screen_y);
    } else if (has_item) {
        draw_floor_item(r, screen_x, screen_y);
    }
}

static void draw_floor_item_with_underlay(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    TileType underlay = floor_item_underlay(g, map_x, map_y);
    if (underlay == TILE_EMBERFORGE_MECHANISM || underlay == TILE_EMBERFORGE_COLD || underlay == TILE_EMBERFORGE_LIT) {
        draw_emberforge_tile(r, screen_x, screen_y, map_x, map_y, underlay);
    } else if (underlay == TILE_TRAP_HIDDEN && g->location == LOCATION_FOREST) {
        draw_forest_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TRAP_HIDDEN &&
        g->location == LOCATION_MOUNTAINS) {
        draw_mountain_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TRAP_HIDDEN &&
        g->location == LOCATION_COAST) {
        draw_coast_trap_underlay(r, g, map_x, map_y, screen_x, screen_y);
    } else if (underlay == TILE_DUNGEON_STAIRS_SEALED || underlay == TILE_DUNGEON_STAIRS_RETURN) {
        draw_dungeon_return_stairs(r, screen_x, screen_y, underlay == TILE_DUNGEON_STAIRS_RETURN);
    } else if (underlay == TILE_FOREST_SHORTCUT) {
        draw_forest_edge(r, screen_x, screen_y, map_x, map_y, 1);
    } else if (underlay == TILE_SWAMP_SHORTCUT) {
        draw_swamp_edge(r, screen_x, screen_y, map_x, map_y, 1);
    } else if (underlay == TILE_MOUNTAIN_SHORTCUT) {
        draw_mountain_edge(r, screen_x, screen_y, map_x, map_y, 1);
    } else if (underlay == TILE_FOREST_FLOOR) {
        draw_forest_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOUNTAIN_BRIDGE) {
        draw_mountain_bridge(r, screen_x, screen_y,
            mountain_crossing_neighbors(&g->map, map_x, map_y, 0));
    } else if (underlay == TILE_MOUNTAIN_CAVE_FLOOR) {
        draw_mountain_cave_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOUNTAIN_FORTRESS_FLOOR) {
        draw_mountain_fortress_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOUNTAIN_FLOOR) {
        draw_mountain_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_COAST_SHALLOW_WATER) {
        draw_coast_shallow_water(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_COAST_DRAINED_WATER) {
        draw_coast_channel(r, screen_x, screen_y, map_x, map_y, 0, 0);
    } else if (underlay == TILE_COAST_FLOOR) {
        draw_coast_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_SWAMP_FLOOR) {
        draw_swamp_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_FROST_JOURNAL || underlay == TILE_NPC_FROST_SURVIVOR) {
        draw_frostfell_quest_tile(r, screen_x, screen_y, map_x, map_y, underlay);
    } else if (underlay == TILE_FROST_FLOOR) {
        draw_frostfell_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_DESERT_FLOOR || underlay == TILE_DESERT_ENTRANCE ||
        underlay == TILE_DESERT_EXIT) {
        if (underlay == TILE_DESERT_FLOOR) {
            draw_desert_floor(r, screen_x, screen_y, map_x, map_y);
        } else {
            draw_desert_edge(r, screen_x, screen_y, map_x, map_y);
        }
    } else if (underlay == TILE_ASHEN_FLOOR) {
        draw_ashen_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ASHEN_RUIN) {
        draw_ashen_ruin(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ASHEN_ENTRANCE || underlay == TILE_ASHEN_EXIT) {
        draw_ashen_edge(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_CATACOMBS_FLOOR || underlay == TILE_BURIAL_PLATE) {
        draw_catacombs_tile(r, &g->map, screen_x, screen_y, map_x, map_y, underlay);
    } else if (underlay == TILE_GLASSDEEP_RESONATOR || underlay == TILE_GLASSDEEP_RESONATOR_LIT) {
        draw_glassdeep_resonator(r, screen_x, screen_y, map_x, map_y, underlay);
    } else if (underlay == TILE_GLASSDEEP_FLOOR) {
        draw_glassdeep_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_GLASSDEEP_RUIN) {
        draw_glassdeep_ruin(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_GLASSDEEP_ENTRANCE || underlay == TILE_GLASSDEEP_EXIT) {
        draw_glassdeep_edge(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOONVEIL_SEED_POD || underlay == TILE_MOONVEIL_SPRING ||
        underlay == TILE_MOONVEIL_PLANTING_CIRCLE || underlay == TILE_MOONVEIL_MOONFLOWER || underlay == TILE_MOONVEIL_BLOSSOMS) {
        draw_moonveil_quest_tile(r, screen_x, screen_y, map_x, map_y, underlay);
    } else if (underlay == TILE_MOONVEIL_FLOOR) {
        draw_moonveil_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOONVEIL_CIRCLE) {
        draw_moonveil_circle(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_MOONVEIL_ENTRANCE || underlay == TILE_MOONVEIL_EXIT) {
        draw_moonveil_edge(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_DESERT_LAMP) {
        draw_desert_lamp(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_FROST_LAKE) {
        draw_frostfell_lake(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_FROST_ICE) {
        draw_frostfell_ice(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_DRAGON_FLOOR ||
        underlay == TILE_DRAGON_ASH || underlay == TILE_DRAGON_HOARD) {
        int terrain = underlay == TILE_DRAGON_ASH ? 1 :
            (underlay == TILE_DRAGON_HOARD ? 2 : 0);
        draw_dragonspine_floor(r, screen_x, screen_y, map_x, map_y, terrain);
    } else if (underlay == TILE_DRAGON_TREASURE) {
        draw_dragon_goblet(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TOWN_FLOOR) {
        if (game_is_king_road(g)) {
            draw_crownroad_tile(r, screen_x, screen_y, map_x, map_y, 0);
        } else {
            draw_town_floor(r, screen_x, screen_y);
        }
    } else if (underlay == TILE_TOWN_PATH) {
        if (game_is_king_road(g)) {
            draw_crownroad_tile(r, screen_x, screen_y, map_x, map_y, 1);
        } else {
            draw_town_path(r, screen_x, screen_y);
        }
    } else if (underlay == TILE_TAVERN_FLOOR) {
        draw_tavern_floor(r, screen_x, screen_y);
    } else if (underlay == TILE_LABYRINTH_FLOOR ||
        g->location == LOCATION_LABYRINTH) {
        draw_labyrinth_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_WATER ||
        underlay == TILE_ISLAND_DOCK) {
        draw_island_water(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_SAND) {
        draw_island_sand(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_GRASS) {
        draw_island_grass(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_ISLAND_PATH) {
        draw_island_path(r, screen_x, screen_y, map_x, map_y);
    } else if (underlay == TILE_TEMPLE_FLOOR ||
        g->location == LOCATION_TEMPLE) {
        draw_temple_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_DUNGEON) {
        draw_dungeon_floor(r, screen_x, screen_y, map_x, map_y);
    } else {
        draw_floor(r, screen_x, screen_y);
    }
    draw_floor_loot(r, g, map_x, map_y, screen_x, screen_y);
}

static void draw_trap_underlay(Renderer *r, const GameState *g, int map_x, int map_y, int screen_x, int screen_y) {
    int cave_neighbors = 0;
    int fortress_neighbors = 0;
    int bridge_neighbors = 0;
    for (int y = map_y - 1; y <= map_y + 1; y++) {
        for (int x = map_x - 1; x <= map_x + 1; x++) {
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
                continue;
            }
            TileType tile = g->map.tiles[y][x];
            cave_neighbors += tile == TILE_MOUNTAIN_CAVE_FLOOR;
            fortress_neighbors += tile == TILE_MOUNTAIN_FORTRESS_FLOOR;
            bridge_neighbors += tile == TILE_MOUNTAIN_BRIDGE;
        }
    }
    if (g->location == LOCATION_FOREST ||
        g->location == LOCATION_FOREST_ROAD) {
        draw_forest_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_COAST) {
        draw_coast_trap_underlay(r, g, map_x, map_y, screen_x, screen_y);
    } else if (g->location == LOCATION_MOUNTAINS &&
        cave_neighbors >= fortress_neighbors &&
        cave_neighbors >= bridge_neighbors && cave_neighbors > 0) {
        draw_mountain_cave_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_MOUNTAINS &&
        fortress_neighbors >= bridge_neighbors && fortress_neighbors > 0) {
        draw_mountain_fortress_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_MOUNTAINS && bridge_neighbors > 0) {
        draw_mountain_bridge(r, screen_x, screen_y,
            mountain_crossing_neighbors(&g->map, map_x, map_y, 0));
    } else if (g->location == LOCATION_MOUNTAINS || g->location == LOCATION_HIGH_PASS) {
        draw_mountain_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_DRAGONSPINE) {
        draw_dragonspine_floor(r, screen_x, screen_y, map_x, map_y, 0);
    } else if (g->location == LOCATION_FROSTFELL) {
        draw_frostfell_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_DESERT) {
        draw_desert_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_MOONVEIL) {
        draw_moonveil_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_ASHEN) {
        draw_ashen_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_GLASSDEEP) {
        draw_glassdeep_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_DUNGEON) {
        draw_dungeon_floor(r, screen_x, screen_y, map_x, map_y);
    } else if (g->location == LOCATION_LABYRINTH) {
        draw_labyrinth_floor(r, screen_x, screen_y, map_x, map_y);
    } else {
        draw_floor(r, screen_x, screen_y);
    }
}

// World-space particles keep their positions when the camera moves or resizes.
// Clock-based motion and local seeds leave turns, saves and gameplay RNG alone.
static void draw_region_weather(Renderer *r, const Viewport *v, int desert) {
    SDL_Rect area = {0, 0, v->tiles_x * TILE_SIZE, v->tiles_y * TILE_SIZE};
    if (area.w > r->screen_w - INFO_PANEL_W) {
        area.w = r->screen_w - INFO_PANEL_W;
    }
    if (area.h > r->tiles_y * TILE_SIZE) {
        area.h = r->tiles_y * TILE_SIZE;
    }
    SDL_Rect old_clip;
    SDL_bool clipped = SDL_RenderIsClipEnabled(r->sdl);
    SDL_RenderGetClipRect(r->sdl, &old_clip);
    if (clipped) {
        SDL_IntersectRect(&area, &old_clip, &area);
    }
    if (area.w <= 0 || area.h <= 0) {
        return;
    }
    SDL_BlendMode old_blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &old_blend);
    SDL_RenderSetClipRect(r->sdl, &area);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);

    if (desert) {
        SDL_SetRenderDrawColor(r->sdl, 196, 151, 80, 18);
    } else {
        SDL_SetRenderDrawColor(r->sdl, 202, 222, 242, 12);
    }
    SDL_RenderFillRect(r->sdl, &area);
    Uint64 now = SDL_GetTicks();
    int field_w = v->map_w * TILE_SIZE + 32;
    int field_h = v->map_h * TILE_SIZE + 32;
    int count = field_w * field_h / (desert ? 2400 : 3200);
    for (int layer = 0; layer < 3; layer++) {
        for (int i = 0; i < count; i++) {
            unsigned int seed = (unsigned int)(i + 1) * 2654435761u ^
                (unsigned int)(layer + 1) * 2246822519u;
            seed ^= seed >> 16;
            seed *= 3266489917u;
            seed ^= seed >> 15;
            int wind = 50 + layer * 35 + (int)(seed % 19u);
            int fall = 28 + layer * 15 + (int)((seed >> 8) % 13u);
            if (desert) {
                wind = 180 + layer * 65 + (int)(seed % 29u);
                fall = 12 + layer * 8 + (int)((seed >> 8) % 9u);
            }
            int drift = (int)(now * wind / 1000 % field_w);
            int drop = (int)(now * fall / 1000 % field_h);
            int x = ((int)(seed % field_w) + field_w - drift) % field_w -
                16 - v->cam_x * TILE_SIZE;
            int y = ((int)((seed >> 12) % field_h) + drop) % field_h -
                16 - v->cam_y * TILE_SIZE;
            if (x < area.x - 12 || y < area.y - 6 ||
                x >= area.x + area.w + 6 || y >= area.y + area.h + 6) {
                continue;
            }
            if (desert) {
                SDL_SetRenderDrawColor(r->sdl, 225, 184, 110, 65 + layer * 40);
                SDL_RenderDrawLine(r->sdl, x, y, x + 4 + layer * 3, y - 1 - layer);
                SDL_SetRenderDrawColor(r->sdl, 246, 211, 147, 100 + layer * 45);
                SDL_RenderDrawPoint(r->sdl, x, y);
                continue;
            }
            int size = layer == 0 ? 1 : 2;
            SDL_Rect flake = {x, y, size, size};
            // A faint blue edge keeps white flakes visible over snowfields.
            SDL_SetRenderDrawColor(r->sdl, 94, 130, 167, 45 + layer * 20);
            SDL_RenderDrawLine(r->sdl, x + 1, y + size, x + size, y + size);
            SDL_SetRenderDrawColor(r->sdl, 244, 250, 255, 95 + layer * 60);
            SDL_RenderFillRect(r->sdl, &flake);
            if (layer == 2) {
                SDL_RenderDrawLine(r->sdl, x + 1, y, x + 4, y - 2);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, old_blend);
    SDL_RenderSetClipRect(r->sdl, clipped ? &old_clip : NULL);
}

// True after the Kraken's warning turn, when its tentacles are about to strike.
static int kraken_tentacles_raised(const GameState *g) {
    if (g->location != LOCATION_FROSTFELL) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_POLAR_KRAKEN && e->move_timer % 2 == 1) {
            return 1;
        }
    }
    return 0;
}

// While an ice slide plays, moves the player's drawn position back along the
// slide so the sprite glides at a steady speed onto the tile where it stops.
static void ice_slide_offset(const GameState *g, int *px, int *py) {
    if (g->trail_frames <= 0 || g->trail_effect != TRAIL_EFFECT_ICE_SLIDE || g->trail_count <= 0) {
        return;
    }
    Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
    Uint32 duration = (Uint32)g->trail_count * ICE_SLIDE_TILE_MS;
    if (elapsed >= duration) {
        return;
    }
    int remaining = (int)((duration - elapsed) * TILE_SIZE / ICE_SLIDE_TILE_MS);
    *px -= g->player.last_dx * remaining;
    *py -= g->player.last_dy * remaining;
}

static void draw_kraken_target(Renderer *r, const GameState *g, const Viewport *v) {
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_POLAR_KRAKEN || e->move_timer % 2 != 1) {
            continue;
        }
        SDL_BlendMode blend;
        SDL_GetRenderDrawBlendMode(r->sdl, &blend);
        SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !map_is_walkable(&g->map, x, y)) {
                    continue;
                }
                int targeted = x == e->attack_target_x && y == e->attack_target_y;
                int danger = targeted;
                for (int dy = -1; dy <= 1 && !danger; dy++) {
                    for (int dx = -1; dx <= 1 && !danger; dx++) {
                        int tx = x + dx;
                        int ty = y + dy;
                        danger = tx >= 0 && tx < MAP_W && ty >= 0 && ty < MAP_H &&
                            g->map.tiles[ty][tx] == TILE_FROST_LAKE_HOLE;
                    }
                }
                if (!danger) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 230, 82, 28, 65);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 255, 155, 57, targeted ? 255 : 145);
                SDL_RenderDrawRect(r->sdl, &tile);
                if (targeted) {
                    SDL_RenderDrawLine(r->sdl, px + 7, py + 7, px + 16, py + 16);
                    SDL_RenderDrawLine(r->sdl, px + 16, py + 7, px + 7, py + 16);
                }
            }
        }
        SDL_SetRenderDrawBlendMode(r->sdl, blend);
    }
}

static void draw_kraken_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_FROSTFELL) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_POLAR_KRAKEN || e->max_hp <= 0 ||
            !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
            continue;
        }
        int width = r->screen_w - INFO_PANEL_W - 16;
        if (width > 360) {
            width = 360;
        }
        if (width < 160) {
            return;
        }
        int x = (r->screen_w - INFO_PANEL_W - width) / 2;
        SDL_Rect panel = {x, 8, width, 62};
        SDL_SetRenderDrawColor(r->sdl, 16, 28, 43, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 111, 172, 203, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, "POLAR KRAKEN", x + 10, 16, (SDL_Color){218, 243, 255, 255}, r->font_tiny);
        if (width >= 240) {
            char hp[32];
            snprintf(hp, sizeof(hp), "%d / %d", e->hp, e->max_hp);
            int text_w = 0;
            TTF_SizeText(r->font_tiny, hp, &text_w, NULL);
            renderer_draw_text(r, hp, x + width - text_w - 10, 16, (SDL_Color){168, 211, 230, 255}, r->font_tiny);
        }
        SDL_Rect bar = {x + 10, 31, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 39, 58, 77, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 80, 201, 217, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        int warning = e->move_timer % 2 == 1;
        const char *hint = warning ? "STRIKE NEXT TURN - LEAVE ORANGE ICE" : "TENTACLES LOWERED";
        if (width < 300 && warning) {
            hint = "STRIKE NEXT TURN";
        }
        renderer_draw_text(r, hint, x + 10, 49, warning ? (SDL_Color){255, 177, 81, 255} : (SDL_Color){157, 196, 219, 255}, r->font_tiny);
    }
}

static void draw_prism_warning(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_GLASSDEEP) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (!enemy->active || enemy->type != ENEMY_PRISM_SOVEREIGN ||
            enemy->attack_target_x < 0 || enemy->attack_target_y < 0) {
            continue;
        }
        int dx = (enemy->attack_target_x > enemy->x) - (enemy->attack_target_x < enemy->x);
        int dy = (enemy->attack_target_y > enemy->y) - (enemy->attack_target_y < enemy->y);
        for (int step = 1; step <= 8; step++) {
            int x = enemy->x + dx * step;
            int y = enemy->y + dy * step;
            if (!map_is_walkable(&g->map, x, y)) {
                break;
            }
            int blocked = 0;
            for (int j = 0; j < g->enemy_count; j++) {
                blocked |= j != i && g->enemies[j].active && g->enemies[j].x == x && g->enemies[j].y == y;
            }
            if (blocked) {
                break;
            }
            if (viewport_is_visible(v, x, y)) {
                SDL_Rect outline = {viewport_to_screen_x(v, x) * TILE_SIZE + 2,
                    viewport_to_screen_y(v, y) * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4};
                SDL_SetRenderDrawColor(r->sdl, 168, 112, 238, 255);
                SDL_RenderDrawRect(r->sdl, &outline);
            }
        }
    }
}

static void draw_catacombs_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CATACOMBS) {
        return;
    }
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
            int marked = 0;
            for (int i = 0; i < g->map.burial_trap_count; i++) {
                const BurialTrap *trap = &g->map.burial_traps[i];
                marked |= trap->timer > 0 && catacombs_trap_marks(&g->map, trap, x, y);
            }
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *e = &g->enemies[i];
                marked |= e->active && e->type == ENEMY_GRAVE_MARSHAL && catacombs_sweep_marks(&g->map, e, x, y);
                if (!e->active && e->revive_timer > 0 && e->x == x && e->y == y) {
                    int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                    int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                    SDL_SetRenderDrawColor(r->sdl, 170, 225, 234, 255);
                    SDL_RenderDrawLine(r->sdl, px + 5, py + 17, px + 18, py + 20);
                    SDL_RenderDrawLine(r->sdl, px + 7, py + 21, px + 17, py + 15);
                }
            }
            if (marked) {
                SDL_Rect outline = {viewport_to_screen_x(v, x) * TILE_SIZE + 2, viewport_to_screen_y(v, y) * TILE_SIZE + 2, TILE_SIZE - 4, TILE_SIZE - 4};
                SDL_SetRenderDrawColor(r->sdl, 235, 116, 62, 255);
                SDL_RenderDrawRect(r->sdl, &outline);
                SDL_RenderDrawLine(r->sdl, outline.x + 3, outline.y + 3, outline.x + outline.w - 3, outline.y + outline.h - 3);
            }
        }
    }
}

static void draw_low_health_warning(Renderer *r, const GameState *g) {
    if (!game_player_low_health(g)) {
        return;
    }
    int width = r->screen_w - INFO_PANEL_W;
    int height = r->tiles_y * TILE_SIZE;
    int band = (width < height ? width : height) / 160;
    if (band < 1) {
        band = 1;
    } else if (band > 6) {
        band = 6;
    }
    if (width <= band * 16 || height <= band * 16) {
        return;
    }
    // A slow 1.6-second pulse never disappears entirely while health is low.
    int phase = SDL_GetTicks() % 1600;
    int pulse = phase <= 800 ? phase : 1600 - phase;
    int alpha = 48 + pulse * 48 / 800;
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int layer = 0; layer < 8; layer++) {
        int inset = layer * band;
        SDL_SetRenderDrawColor(r->sdl, 225, 20, 28, alpha * (8 - layer) / 8);
        SDL_Rect edges[4] = {
            {inset, inset, width - inset * 2, band},
            {inset, height - inset - band, width - inset * 2, band},
            {inset, inset + band, band, height - inset * 2 - band * 2},
            {width - inset - band, inset + band, band, height - inset * 2 - band * 2}
        };
        SDL_RenderFillRects(r->sdl, edges, 4);
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void game_draw(Renderer *r, GameState *g, Viewport *v) {
    Viewport town_view;
    int temple_remaining = game_temple_remaining_enemies(g);
    int kraken_warning = kraken_tentacles_raised(g);
    int town_scaled = g->location == LOCATION_TOWN ||
        g->location == LOCATION_TOWN2 ||
        g->location == LOCATION_TOWN3 || g->location == LOCATION_TOWN4 ||
        g->location == LOCATION_CASTLE;
    int tavern_scaled = g->location == LOCATION_TAVERN ||
        g->location == LOCATION_INN || g->location == LOCATION_WORKSHOP ||
        g->location == LOCATION_TOWN_HALL || g->location == LOCATION_GUILD ||
        town_life_is_interior(g->location);
    int island_scaled = g->location == LOCATION_ISLAND;
    int labyrinth_scaled = g->location == LOCATION_LABYRINTH;
    int road_scaled = g->location == LOCATION_FOREST_ROAD ||
        g->location == LOCATION_HIGH_PASS ||
        game_is_king_road(g) || g->location == LOCATION_SWAMP_ROAD;
    int town_road_gate = g->location == LOCATION_TOWN &&
        (g->defeated_bosses & (1 << LOCATION_FOREST)) &&
        g->map.tiles[TOWN_ROAD_EXIT_Y][0] == TILE_TOWN_EXIT &&
        g->map.tiles[TOWN_ROAD_EXIT_Y][4] == TILE_TOWN_PATH;
    int jail_scaled = g->location == LOCATION_JAIL;
    if (town_scaled || island_scaled || labyrinth_scaled || jail_scaled) {
        // Keep the entire fixed map inside the play area.
        int map_w = jail_scaled ? JAIL_ROOM_W : town_scaled ? TOWN_W :
            (island_scaled ? ISLAND_W : LABYRINTH_W);
        int map_h = jail_scaled ? JAIL_ROOM_H : town_scaled ? TOWN_H :
            (island_scaled ? ISLAND_H : LABYRINTH_H);
        viewport_init(&town_view, map_w, map_h, map_w, map_h);
        v = &town_view;
        int play_w = r->screen_w - INFO_PANEL_W;
        int play_h = r->tiles_y * TILE_SIZE;
        if (play_w < 1) {
            play_w = 1;
        }
        if (play_h < 1) {
            play_h = 1;
        }
        SDL_RenderSetScale(r->sdl,
            (float)play_w / (map_w * TILE_SIZE),
            (float)play_h / (map_h * TILE_SIZE));
    } else if (g->location == LOCATION_ESCAPE_TUNNEL) {
        viewport_init(&town_view, v->tiles_x < ESCAPE_TUNNEL_W ? v->tiles_x : ESCAPE_TUNNEL_W,
            v->tiles_y < ESCAPE_TUNNEL_H ? v->tiles_y : ESCAPE_TUNNEL_H, ESCAPE_TUNNEL_W, ESCAPE_TUNNEL_H);
        viewport_center_on(&town_view, g->player.x, g->player.y);
        v = &town_view;
    } else if (road_scaled) {
        int road_w = g->location == LOCATION_HIGH_PASS ? HIGH_PASS_W :
            (game_is_king_road(g) ? CROWNROAD_W :
            (g->location == LOCATION_SWAMP_ROAD ? SWAMP_ROAD_W : FOREST_ROAD_W));
        int road_h = g->location == LOCATION_HIGH_PASS ? HIGH_PASS_H :
            (game_is_king_road(g) ? CROWNROAD_H :
            (g->location == LOCATION_SWAMP_ROAD ? SWAMP_ROAD_H : FOREST_ROAD_H));
        int view_w = v->tiles_x < road_w ? v->tiles_x : road_w;
        int view_h = v->tiles_y < road_h ? v->tiles_y : road_h;
        viewport_init(&town_view, view_w, view_h,
            road_w, road_h);
        viewport_center_on(&town_view, g->player.x, g->player.y);
        v = &town_view;
    } else if (g->location == LOCATION_CASTLE_INTERIOR) {
        viewport_init(&town_view, v->tiles_x < CASTLE_W ? v->tiles_x : CASTLE_W, v->tiles_y < CASTLE_H ? v->tiles_y : CASTLE_H, CASTLE_W, CASTLE_H);
        viewport_center_on(&town_view, g->player.x, g->player.y);
        v = &town_view;
    } else if (g->location == LOCATION_CATACOMBS) {
        viewport_init(&town_view, v->tiles_x < CATACOMBS_W ? v->tiles_x : CATACOMBS_W, v->tiles_y < CATACOMBS_H ? v->tiles_y : CATACOMBS_H, CATACOMBS_W, CATACOMBS_H);
        viewport_center_on(&town_view, g->player.x, g->player.y);
        v = &town_view;
    } else if (g->location == LOCATION_SWAMP ||
        g->location == LOCATION_DRAGONSPINE ||
        g->location == LOCATION_DESERT ||
        g->location == LOCATION_MOONVEIL ||
        g->location == LOCATION_ASHEN ||
        g->location == LOCATION_GLASSDEEP ||
        g->location == LOCATION_FROSTFELL) {
        int view_w = v->tiles_x < SWAMP_MAP_W ? v->tiles_x : SWAMP_MAP_W;
        int view_h = v->tiles_y < SWAMP_MAP_H ? v->tiles_y : SWAMP_MAP_H;
        viewport_init(&town_view, view_w, view_h,
            SWAMP_MAP_W, SWAMP_MAP_H);
        viewport_center_on(&town_view, g->player.x, g->player.y);
        v = &town_view;
    } else if (tavern_scaled) {
        // Fit the entire room without stretching sprites or showing unused map tiles.
        viewport_init(&town_view, TAVERN_W, TAVERN_H, MAP_W, MAP_H);
        town_view.cam_x = TAVERN_X;
        town_view.cam_y = TAVERN_Y;
        v = &town_view;
        int play_w = r->screen_w - INFO_PANEL_W;
        int play_h = r->tiles_y * TILE_SIZE;
        if (play_w < 1) {
            play_w = 1;
        }
        if (play_h < 1) {
            play_h = 1;
        }
        float scale_x = (float)play_w / (TAVERN_W * TILE_SIZE);
        float scale_y = (float)play_h / (TAVERN_H * TILE_SIZE);
        float scale = scale_x < scale_y ? scale_x : scale_y;
        int room_w = (int)(TAVERN_W * TILE_SIZE * scale);
        int room_h = (int)(TAVERN_H * TILE_SIZE * scale);
        SDL_Rect room_view = {(play_w - room_w) / 2, (play_h - room_h) / 2,
            room_w, room_h};
        SDL_RenderSetViewport(r->sdl, &room_view);
        SDL_RenderSetScale(r->sdl, scale, scale);
    }
    int landmark_x = -1;
    int landmark_y = -1;
    if (g->location == LOCATION_FOREST && g->map.room_count > 1) {
        int room = g->level == FOREST_BOSS_LEVEL ? g->map.room_count - 2 :
            g->map.room_count - 1;
        map_room_center(&g->map.rooms[room], &landmark_x, &landmark_y);
    }

    // Draw map tiles
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (!viewport_is_visible(v, x, y)) continue;
            map_mark_explored(&g->map, x, y);
            int sx = viewport_to_screen_x(v, x);
            int sy = viewport_to_screen_y(v, y);
            if (g->location == LOCATION_JAIL || g->location == LOCATION_ESCAPE_TUNNEL) {
                jail_draw_tile(r, g, sx, sy, x, y, g->map.tiles[y][x] == TILE_ITEM ? floor_item_underlay(g, x, y) : g->map.tiles[y][x]);
                continue;
            }
            if (g->location == LOCATION_CASTLE_INTERIOR) {
                TileType tile = g->map.tiles[y][x];
                castle_draw_tile(r, g, sx, sy, x, y, tile == TILE_ITEM ? floor_item_underlay(g, x, y) : tile);
                continue;
            }
            switch (terrain_display_tile(g, g->map.tiles[y][x])) {
                case TILE_FLOOR:
                    if (g->location == LOCATION_DUNGEON) {
                        draw_dungeon_floor(r, sx, sy, x, y);
                    } else {
                        draw_floor(r, sx, sy);
                    }
                    break;
                case TILE_WALL: {
                    if (g->location == LOCATION_CASTLE &&
                        x >= TOWN_MOAT_X && x < TOWN_MOAT_X + TOWN_MOAT_W &&
                        y >= TOWN_MOAT_Y && y < TOWN_MOAT_Y + TOWN_MOAT_H) {
                        draw_town_floor(r, sx, sy);
                    } else if (g->location == LOCATION_TOWN3 &&
                        x >= TOWN_GUILD_X && x < TOWN_GUILD_X + TOWN_GUILD_W &&
                        y >= TOWN_GUILD_Y && y < TOWN_GUILD_Y + TOWN_GUILD_H) {
                        draw_town_floor(r, sx, sy);
                    } else if (g->location == LOCATION_TOWN4 &&
                        x >= TOWN4_WORKSHOP_X &&
                        x < TOWN4_WORKSHOP_X + TOWN4_WORKSHOP_W &&
                        y >= TOWN4_WORKSHOP_Y &&
                        y < TOWN4_WORKSHOP_Y + TOWN4_WORKSHOP_H) {
                        draw_town_floor(r, sx, sy);
                    } else if (g->location == LOCATION_TOWN4 &&
                        x >= TOWN4_HALL_X && x < TOWN4_HALL_X + TOWN4_HALL_W &&
                        y >= TOWN4_HALL_Y && y < TOWN4_HALL_Y + TOWN4_HALL_H) {
                        draw_town_floor(r, sx, sy);
                    } else if (game_is_king_road(g)) {
                        draw_crownroad_tile(r, sx, sy, x, y, 2);
                    } else if (g->location == LOCATION_DUNGEON) {
                        draw_dungeon_wall(r, sx, sy, x, y);
                        const int dx[4] = {0, 1, 0, -1};
                        const int dy[4] = {-1, 0, 1, 0};
                        const unsigned int side[4] = {
                            DUNGEON_EDGE_NORTH, DUNGEON_EDGE_EAST,
                            DUNGEON_EDGE_SOUTH, DUNGEON_EDGE_WEST
                        };
                        for (int i = 0; i < 4; i++) {
                            int nx = x + dx[i];
                            int ny = y + dy[i];
                            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
                                continue;
                            }
                            TileType neighbor = g->map.tiles[ny][nx];
                            if (neighbor == TILE_LOCKED_DOOR || neighbor == TILE_CRYPT_DOOR) {
                                draw_dungeon_door_support(r, sx, sy, side[i],
                                    neighbor == TILE_CRYPT_DOOR);
                            }
                        }
                        if (dungeon_wall_has_torch(g, x, y)) {
                            draw_dungeon_torch(r, sx, sy, x, y);
                        }
                    } else {
                        draw_wall(r, sx, sy);
                    }
                    break;
                }
                case TILE_FOREST_WALL:
                    draw_forest_wall(r, sx, sy, x, y); break;
                case TILE_FOREST_HIDDEN_TRAIL:
                    draw_forest_wall(r, sx, sy, x, y); break;
                case TILE_FOREST_FLOOR:
                    draw_forest_floor(r, sx, sy, x, y); break;
                case TILE_FOREST_ENTRANCE:
                    draw_forest_edge(r, sx, sy, x, y, 0); break;
                case TILE_FOREST_EXIT:
                    draw_forest_edge(r, sx, sy, x, y, 1); break;
                case TILE_FOREST_SHORTCUT:
                    draw_forest_edge(r, sx, sy, x, y, 1);
                    renderer_draw_text(r, "SHORTCUT", sx * TILE_SIZE - 20, sy * TILE_SIZE - 12,
                        (SDL_Color){128, 235, 143, 255}, r->font_tiny);
                    break;
                case TILE_FOREST_LANDMARK:
                    draw_forest_landmark(r, sx, sy, x, y); break;
                case TILE_FOREST_FALSE_MARKER:
                    draw_forest_false_marker(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_FLOOR:
                    draw_mountain_floor(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_WALL:
                    draw_mountain_wall(r, sx, sy, x, y);
                    break;
                case TILE_MOUNTAIN_ENTRANCE:
                    draw_mountain_edge(r, sx, sy, x, y, 0); break;
                case TILE_MOUNTAIN_EXIT:
                    draw_mountain_edge(r, sx, sy, x, y, 1); break;
                case TILE_MOUNTAIN_SHORTCUT:
                    draw_mountain_edge(r, sx, sy, x, y, 1);
                    renderer_draw_text(r, "SHORTCUT", sx * TILE_SIZE - 20, sy * TILE_SIZE - 12,
                        (SDL_Color){128, 235, 143, 255}, r->font_tiny);
                    break;
                case TILE_MOUNTAIN_HIDDEN_CAVE:
                    draw_mountain_wall(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_ROCKFALL:
                    draw_mountain_rockfall(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_CHASM:
                    draw_mountain_chasm(r, sx, sy,
                        mountain_crossing_neighbors(&g->map, x, y, 1)); break;
                case TILE_MOUNTAIN_GATE:
                    draw_mountain_gate(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_CACHE:
                    draw_crypt_cache(r, sx, sy); break;
                case TILE_MOUNTAIN_WEAK_BRIDGE:
                case TILE_MOUNTAIN_BRIDGE:
                    draw_mountain_bridge(r, sx, sy,
                        mountain_crossing_neighbors(&g->map, x, y, 0)); break;
                case TILE_MOUNTAIN_CAVE_FLOOR:
                    draw_mountain_cave_floor(r, sx, sy, x, y); break;
                case TILE_MOUNTAIN_FORTRESS_FLOOR:
                    draw_mountain_fortress_floor(r, sx, sy, x, y); break;
                case TILE_COAST_FLOOR:
                    draw_coast_floor(r, sx, sy, x, y);
                    if (x + 1 < MAP_W &&
                        g->map.tiles[y][x + 1] == TILE_COAST_SLUICE_CONTROL) {
                        draw_coast_sluice_conduit(r, sx, sy);
                    }
                    break;
                case TILE_COAST_WALL:
                    draw_coast_wall(r, sx, sy, x, y);
                    if (x + 2 < MAP_W &&
                        g->map.tiles[y][x + 2] == TILE_COAST_SLUICE_CONTROL) {
                        draw_coast_sluice_intake(r, sx, sy);
                    }
                    break;
                case TILE_COAST_ENTRANCE:
                    draw_coast_edge(r, sx, sy, x, y, 0); break;
                case TILE_COAST_EXIT: {
                    int open = g->map.tiles[g->map.stairs_down_y]
                        [g->map.stairs_down_x] != TILE_COAST_DEEP_WATER;
                    draw_coast_edge(r, sx, sy, x, y, open ? 1 : 2);
                    break;
                }
                case TILE_COAST_SHALLOW_WATER:
                    draw_coast_shallow_water(r, sx, sy, x, y); break;
                case TILE_COAST_DRAINED_WATER:
                    draw_coast_channel(r, sx, sy, x, y, 0, 0); break;
                case TILE_COAST_DEEP_WATER:
                    draw_coast_channel(r, sx, sy, x, y, 0, 1); break;
                case TILE_COAST_CHANNEL_DRY:
                    draw_coast_channel(r, sx, sy, x, y, 1, 0); break;
                case TILE_COAST_CHANNEL_WATER:
                    draw_coast_channel(r, sx, sy, x, y, 1, 1); break;
                case TILE_COAST_SLUICE_CONTROL:
                    draw_coast_sluice(r, sx, sy, x, y); break;
                case TILE_COAST_CACHE:
                    draw_coast_cache(r, sx, sy, x, y); break;
                case TILE_COAST_TIDE_CONTROL:
                    draw_coast_tide_control(r, sx, sy, x, y); break;
                case TILE_SWAMP_FLOOR:
                    draw_swamp_floor(r, sx, sy, x, y); break;
                case TILE_SWAMP_WALL:
                    draw_swamp_wall(r, sx, sy, x, y); break;
                case TILE_SWAMP_ENTRANCE:
                    draw_swamp_edge(r, sx, sy, x, y, 0); break;
                case TILE_SWAMP_EXIT:
                    draw_swamp_edge(r, sx, sy, x, y, 1); break;
                case TILE_SWAMP_SHORTCUT:
                    draw_swamp_edge(r, sx, sy, x, y, 1);
                    renderer_draw_text(r, "SHORTCUT", sx * TILE_SIZE - 20, sy * TILE_SIZE - 12,
                        (SDL_Color){128, 235, 143, 255}, r->font_tiny);
                    break;
                case TILE_SWAMP_DAUGHTER:
                    draw_swamp_daughter(r, sx, sy, x, y); break;
                case TILE_FROST_FLOOR:
                    draw_frostfell_floor(r, sx, sy, x, y); break;
                case TILE_FROST_JOURNAL:
                case TILE_NPC_FROST_SURVIVOR:
                case TILE_NPC_BRENNA:
                case TILE_NPC_ORIN:
                    draw_frostfell_quest_tile(r, sx, sy, x, y, g->map.tiles[y][x]);
                    break;
                case TILE_ASHEN_FLOOR:
                    draw_ashen_floor(r, sx, sy, x, y);
                    break;
                case TILE_ASHEN_WALL:
                    draw_ashen_wall(r, sx, sy, x, y);
                    break;
                case TILE_ASHEN_LAVA:
                    draw_ashen_lava(r, sx, sy, x, y);
                    break;
                case TILE_ASHEN_RUIN:
                    draw_ashen_ruin(r, sx, sy, x, y);
                    break;
                case TILE_EMBERFORGE_MECHANISM:
                case TILE_EMBERFORGE_COLD:
                case TILE_EMBERFORGE_LIT:
                    draw_emberforge_tile(r, sx, sy, x, y, g->map.tiles[y][x]);
                    break;
                case TILE_ASHEN_ENTRANCE:
                case TILE_ASHEN_EXIT:
                    draw_ashen_edge(r, sx, sy, x, y);
                    break;
                case TILE_CATACOMBS_FLOOR:
                case TILE_CATACOMBS_WALL:
                case TILE_OSSUARY_BRAZIER:
                case TILE_OSSUARY_COLD:
                case TILE_MEMORIAL_BRAZIER:
                case TILE_MEMORIAL_COLD:
                case TILE_BURIAL_LEDGER:
                case TILE_BURIAL_PLATE:
                case TILE_CATACOMBS_SARCOPHAGUS:
                    draw_catacombs_tile(r, &g->map, sx, sy, x, y, g->map.tiles[y][x]);
                    break;
                case TILE_GLASSDEEP_RESONATOR:
                case TILE_GLASSDEEP_RESONATOR_LIT:
                    draw_glassdeep_resonator(r, sx, sy, x, y, g->map.tiles[y][x]);
                    break;
                case TILE_GLASSDEEP_FLOOR:
                    draw_glassdeep_floor(r, sx, sy, x, y);
                    break;
                case TILE_GLASSDEEP_WALL:
                    draw_glassdeep_wall(r, sx, sy, x, y);
                    break;
                case TILE_GLASSDEEP_POOL:
                    draw_glassdeep_pool(r, sx, sy, x, y);
                    break;
                case TILE_GLASSDEEP_RUIN:
                    draw_glassdeep_ruin(r, sx, sy, x, y);
                    break;
                case TILE_GLASSDEEP_ENTRANCE:
                case TILE_GLASSDEEP_EXIT:
                    draw_glassdeep_edge(r, sx, sy, x, y);
                    break;
                case TILE_NPC_LIORA:
                case TILE_MOONVEIL_SEED_POD:
                case TILE_MOONVEIL_SPRING:
                case TILE_MOONVEIL_PLANTING_CIRCLE:
                case TILE_MOONVEIL_MOONFLOWER:
                case TILE_MOONVEIL_BLOSSOMS:
                    draw_moonveil_quest_tile(r, sx, sy, x, y, g->map.tiles[y][x]);
                    break;
                case TILE_MOONVEIL_FLOOR:
                    draw_moonveil_floor(r, sx, sy, x, y);
                    break;
                case TILE_MOONVEIL_WALL:
                    draw_moonveil_wall(r, sx, sy, x, y);
                    break;
                case TILE_MOONVEIL_POOL:
                    draw_moonveil_pool(r, sx, sy, x, y);
                    break;
                case TILE_MOONVEIL_CIRCLE:
                    draw_moonveil_circle(r, sx, sy, x, y);
                    break;
                case TILE_MOONVEIL_ENTRANCE:
                case TILE_MOONVEIL_EXIT:
                    draw_moonveil_edge(r, sx, sy, x, y);
                    break;
                case TILE_DESERT_FLOOR:
                    draw_desert_floor(r, sx, sy, x, y);
                    break;
                case TILE_DESERT_WALL:
                    draw_desert_wall(r, sx, sy, x, y);
                    break;
                case TILE_DESERT_ENTRANCE:
                case TILE_DESERT_EXIT:
                    draw_desert_edge(r, sx, sy, x, y);
                    break;
                case TILE_FROST_WALL:
                    draw_frostfell_wall(r, sx, sy, x, y); break;
                case TILE_FROST_ENTRANCE:
                case TILE_FROST_EXIT:
                    draw_frostfell_edge(r, sx, sy, x, y); break;
                case TILE_FROST_LAKE:
                    draw_frostfell_lake(r, sx, sy, x, y); break;
                case TILE_FROST_LAKE_HOLE:
                    draw_frostfell_lake_hole(r, sx, sy, x, y, kraken_warning); break;
                case TILE_FROST_ICE:
                    draw_frostfell_ice(r, sx, sy, x, y); break;
                case TILE_FROST_THIN_ICE:
                    draw_frostfell_thin_ice(r, sx, sy, x, y); break;
                case TILE_FROST_BROKEN_ICE:
                    // Collapsed shortcuts are open water, like the lake holes.
                    draw_frostfell_lake_hole(r, sx, sy, x, y, 0); break;
                case TILE_DRAGON_FLOOR:
                    draw_dragonspine_floor(r, sx, sy, x, y, 0); break;
                case TILE_DRAGON_ASH:
                    draw_dragonspine_floor(r, sx, sy, x, y, 1); break;
                case TILE_DRAGON_HOARD:
                    draw_dragonspine_floor(r, sx, sy, x, y, 2); break;
                case TILE_DRAGON_TREASURE:
                    draw_dragon_goblet(r, sx, sy, x, y); break;
                case TILE_DESERT_LAMP:
                    draw_desert_lamp(r, sx, sy, x, y); break;
                case TILE_DRAGON_WALL:
                    draw_dragonspine_wall(r, sx, sy, x, y); break;
                case TILE_DRAGON_ENTRANCE:
                case TILE_HIGH_PASS_ENTRANCE:
                    draw_dragonspine_edge(r, sx, sy, 0); break;
                case TILE_DRAGON_EXIT:
                case TILE_HIGH_PASS_EXIT:
                    draw_dragonspine_edge(r, sx, sy, 1); break;
                case TILE_COAST_BEACON_UNLIT:
                    draw_coast_beacon(r, sx, sy, x, y, 0); break;
                case TILE_COAST_BEACON_LIT:
                    draw_coast_beacon(r, sx, sy, x, y, 1); break;
                case TILE_DUNGEON_STAIRS_SEALED:
                case TILE_DUNGEON_STAIRS_RETURN:
                    draw_dungeon_return_stairs(r, sx, sy, g->map.tiles[y][x] == TILE_DUNGEON_STAIRS_RETURN);
                    break;
                case TILE_STAIRS_UP:
                case TILE_STAIRS_DOWN:
                case TILE_RETURN_EXIT:
                    if (g->location == LOCATION_CATACOMBS) {
                        draw_catacombs_tile(r, &g->map, sx, sy, x, y, TILE_CATACOMBS_FLOOR);
                        TileType stair = g->map.tiles[y][x];
                        SDL_SetRenderDrawColor(r->sdl, stair == TILE_RETURN_EXIT ? 111 : 202, stair == TILE_RETURN_EXIT ? 219 : 190, stair == TILE_RETURN_EXIT ? 225 : 154, 255);
                        for (int step = 0; step < 5; step++) {
                            int width = stair == TILE_STAIRS_UP ? 16 - step * 2 : 8 + step * 2;
                            int px = sx * TILE_SIZE + (TILE_SIZE - width) / 2;
                            int py = sy * TILE_SIZE + 5 + step * 3;
                            SDL_RenderDrawLine(r->sdl, px, py, px + width, py);
                        }
                    } else if (g->map.tiles[y][x] == TILE_STAIRS_UP) {
                        draw_stairs_up(r, sx, sy);
                        if (g->location == LOCATION_TEMPLE && temple_remaining > 0) {
                            SDL_SetRenderDrawColor(r->sdl, 190, 136, 255, 255);
                            SDL_Rect seal = {sx * TILE_SIZE + 3, sy * TILE_SIZE + 9, 18, 2};
                            SDL_RenderFillRect(r->sdl, &seal);
                            SDL_RenderDrawLine(r->sdl, sx * TILE_SIZE + 12, sy * TILE_SIZE + 4, sx * TILE_SIZE + 12, sy * TILE_SIZE + 18);
                        }
                    } else if (g->map.tiles[y][x] == TILE_STAIRS_DOWN) {
                        draw_stairs_down(r, sx, sy);
                    } else {
                        draw_return_exit(r, sx, sy);
                    }
                    break;
                case TILE_LOCKED_DOOR: draw_locked_door(r, sx, sy); break;
                case TILE_DUNGEON_KEY: draw_dungeon_key(r, sx, sy); break;
                case TILE_CRYPT_DOOR: draw_crypt_door(r, sx, sy); break;
                case TILE_CRYPT_KEY: draw_crypt_key(r, sx, sy); break;
                case TILE_CRYPT_CACHE: draw_crypt_cache(r, sx, sy); break;
                case TILE_PORTAL: draw_portal(r, sx, sy); break;
                case TILE_BROKEN_BURIAL_SEAL:
                    draw_broken_burial_seal(r, sx, sy); break;
                case TILE_RESTORED_BURIAL_SEAL:
                    draw_restored_burial_seal(r, sx, sy); break;
                case TILE_TOWN_FLOOR:
                    if (game_is_king_road(g)) {
                        draw_crownroad_tile(r, sx, sy, x, y, 0);
                    } else {
                        draw_town_floor(r, sx, sy);
                    }
                    break;
                case TILE_TOWN_PATH:
                    if (game_is_king_road(g)) {
                        draw_crownroad_tile(r, sx, sy, x, y, 1);
                    } else {
                        draw_town_path(r, sx, sy);
                    }
                    break;
                case TILE_LABYRINTH_ENTRANCE:
                    draw_town_floor(r, sx, sy); break;
                case TILE_BLACKSMITH_DOOR:
                case TILE_ALCHEMIST_DOOR:
                case TILE_HEALER_DOOR:
                case TILE_WITCH_DOOR:
                case TILE_TAVERN_DOOR:
                case TILE_GUILD_DOOR:
                case TILE_WORKSHOP_DOOR:
                case TILE_TOWN_HALL_DOOR:
                case TILE_LOCAL_DOOR:
                    draw_town_path(r, sx, sy); break;
                case TILE_LOCAL_BUILDING:
                    draw_town_floor(r, sx, sy);
                    break;
                case TILE_TAVERN_FLOOR: draw_tavern_floor(r, sx, sy); break;
                case TILE_TAVERN_WALL: draw_tavern_wall(r, sx, sy); break;
                case TILE_TAVERN_EXIT: draw_tavern_exit(r, sx, sy); break;
                case TILE_TAVERN_TABLE:
                    draw_tavern_table(r, sx, sy);
                    if (town_life_is_interior(g->location) && (y == 7 || y == 17)) {
                        town_life_draw_goods(r, sx, sy, town_life_building(g->location)->style);
                    }
                    break;
                case TILE_NPC_RESIDENT: {
                    const TownBuilding *b = town_life_building(g->location);
                    town_life_draw_resident(r, sx, sy, b ? b->style : 0);
                    break;
                }
                case TILE_NPC_ELOWEN: draw_elowen(r, sx, sy); break;
                case TILE_NPC_DAIN:
                    draw_town_floor(r, sx, sy);
                    draw_dain(r, sx, sy);
                    break;
                case TILE_NPC_SHARPENER:
                    draw_tavern_floor(r, sx, sy);
                    draw_dain(r, sx, sy);
                    break;
                case TILE_NPC_ALDER: draw_alder(r, sx, sy); break;
                case TILE_NPC_MARA: draw_mara(r, sx, sy); break;
                case TILE_NPC_ROOK: draw_rook(r, sx, sy); break;
                case TILE_NPC_GUILD_SEEKER: draw_guild_seeker(r, sx, sy); break;
                case TILE_NPC_STEWARD: {
                    draw_alder(r, sx, sy);
                    SDL_SetRenderDrawColor(r->sdl, 218, 175, 78, 255);
                    SDL_Rect badge = {sx * TILE_SIZE + 13, sy * TILE_SIZE + 15, 3, 3};
                    SDL_RenderFillRect(r->sdl, &badge);
                    break;
                }
                case TILE_JAIL_BUILDING:
                case TILE_JAIL_DOOR:
                    draw_town_floor(r, sx, sy);
                    break;
                case TILE_NPC_INFORMANT:
                    draw_town_floor(r, sx, sy);
                    jail_draw_person(r, sx, sy, 0);
                    break;
                case TILE_NPC_OSWIN:
                    draw_oswin(r, sx, sy);
                    break;
                case TILE_NPC_INNKEEPER: draw_innkeeper(r, sx, sy); break;
                case TILE_NPC_CAIN: draw_cain(r, sx, sy); break;
                case TILE_NPC_BRAM:
                    draw_town_path(r, sx, sy);
                    jail_draw_person(r, sx, sy, 0);
                    break;
                case TILE_NPC_ROWAN: draw_rowan(r, sx, sy); break;
                case TILE_NPC_DRAGON_SEEKER:
                    draw_dragon_seeker(r, sx, sy); break;
                case TILE_NPC_ROYAL_GUARD:
                    // Each guard holds the halberd on the side away from the road.
                    draw_royal_guard(r, sx, sy,
                        game_is_king_road(g) ? y < CROWNROAD_Y :
                        (g->location == LOCATION_TOWN4 ?
                        y < TOWN4_KING_GATE_Y : y < TOWN3_KING_GATE_Y));
                    break;
                case TILE_FOREST_WARDEN:
                    draw_forest_warden(r, sx, sy, x, y); break;
                case TILE_ISLAND_WATER:
                    draw_island_water(r, sx, sy, x, y); break;
                case TILE_ISLAND_SAND:
                    draw_island_sand(r, sx, sy, x, y); break;
                case TILE_ISLAND_GRASS:
                    draw_island_grass(r, sx, sy, x, y); break;
                case TILE_ISLAND_JUNGLE:
                    draw_island_jungle(r, sx, sy, x, y); break;
                case TILE_ISLAND_PATH:
                    draw_island_path(r, sx, sy, x, y); break;
                case TILE_ISLAND_DOCK:
                case TILE_ISLAND_SHIP:
                case TILE_NPC_ISLAND_CAPTAIN:
                    draw_island_water(r, sx, sy, x, y); break;
                case TILE_NPC_ISLAND_NAHLA:
                    draw_island_path(r, sx, sy, x, y);
                    draw_nahla(r, sx, sy); break;
                case TILE_ISLAND_CAMP:
                case TILE_ISLAND_LAGOON:
                    draw_island_grass(r, sx, sy, x, y); break;
                case TILE_ISLAND_MARKER:
                case TILE_ISLAND_STATUE:
                case TILE_ISLAND_TEMPLE_GATE:
                    draw_island_path(r, sx, sy, x, y); break;
                case TILE_TEMPLE_FLOOR:
                    draw_temple_floor(r, sx, sy, x, y); break;
                case TILE_TEMPLE_WALL:
                    draw_temple_wall(r, sx, sy, x, y); break;
                case TILE_TEMPLE_ENTRANCE:
                    draw_temple_entrance(r, sx, sy); break;
                case TILE_TEMPLE_ALTAR:
                    draw_temple_altar(r, sx, sy, g->temple_alignment); break;
                case TILE_TEMPLE_MOON_DOOR_CLOSED:
                    draw_temple_moon_door(r, sx, sy, 0); break;
                case TILE_TEMPLE_MOON_DOOR_OPEN:
                    draw_temple_moon_door(r, sx, sy, 1); break;
                case TILE_TEMPLE_SOLAR_TRAP:
                    draw_temple_solar_trap(r, sx, sy,
                        !g->temple_alignment); break;
                case TILE_TEMPLE_DORMANT_SENTINEL:
                    draw_temple_dormant_sentinel(r, sx, sy); break;
                case TILE_TEMPLE_VAULT_DOOR:
                    draw_temple_vault_door(r, sx, sy); break;
                case TILE_TEMPLE_TREASURE:
                    draw_temple_treasure(r, sx, sy);
                    if (temple_remaining > 0 && g->temple_treasure_state < 2) {
                        SDL_SetRenderDrawColor(r->sdl, 190, 136, 255, 255);
                        SDL_Rect seal = {sx * TILE_SIZE + 4, sy * TILE_SIZE + 10, 16, 2};
                        SDL_RenderFillRect(r->sdl, &seal);
                    }
                    break;
                case TILE_TEMPLE_WATER:
                    draw_temple_water(r, sx, sy, x, y); break;
                case TILE_TEMPLE_RUBBLE:
                    draw_temple_rubble(r, sx, sy); break;
                case TILE_LABYRINTH_FLOOR:
                    draw_labyrinth_floor(r, sx, sy, x, y); break;
                case TILE_LABYRINTH_WALL:
                    draw_labyrinth_wall(r, sx, sy, x, y); break;
                case TILE_LABYRINTH_EXIT:
                    draw_labyrinth_exit(r, sx, sy); break;
                case TILE_LABYRINTH_STAIRS:
                    draw_labyrinth_stairs(r, sx, sy); break;
                case TILE_LABYRINTH_SWITCH_OFF:
                    draw_labyrinth_switch(r, sx, sy, 0); break;
                case TILE_LABYRINTH_SWITCH_ON:
                    draw_labyrinth_switch(r, sx, sy, 1); break;
                case TILE_LABYRINTH_GATE:
                    draw_labyrinth_gate(r, sx, sy); break;
                case TILE_LABYRINTH_RELIC:
                    draw_labyrinth_relic(r, sx, sy); break;
                case TILE_TOWN_EXIT:
                    if (game_is_king_road(g)) {
                        draw_crownroad_tile(r, sx, sy, x, y, 1);
                    } else {
                        draw_town_path(r, sx, sy);
                    }
                    break;
                case TILE_SHOP_BLACKSMITH:
                case TILE_SHOP_ALCHEMIST:
                case TILE_HEALER:
                case TILE_WITCH:
                case TILE_WATCHTOWER:
                case TILE_TAVERN: draw_town_floor(r, sx, sy); break;
                case TILE_ITEM:
                    draw_floor_item_with_underlay(r, g, x, y, sx, sy); break;
                case TILE_TRAP_HIDDEN:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    break;
                case TILE_TRAP_REVEALED:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_warning(r, sx, sy);
                    break;
                case TILE_TRAP_SPIKE:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_spike(r, sx, sy);
                    break;
                case TILE_TRAP_FIRE:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_fire(r, sx, sy);
                    break;
                case TILE_TRAP_POISON:
                    draw_trap_underlay(r, g, x, y, sx, sy);
                    draw_trap_poison(r, sx, sy);
                    break;
                default: draw_floor(r, sx, sy); break;
            }
            if (g->location == LOCATION_DUNGEON && dungeon_is_floor(g, x, y)) {
                unsigned int edges = 0;
                if (y > 0 && g->map.tiles[y - 1][x] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_NORTH;
                }
                if (x < MAP_W - 1 && g->map.tiles[y][x + 1] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_EAST;
                }
                if (y < MAP_H - 1 && g->map.tiles[y + 1][x] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_SOUTH;
                }
                if (x > 0 && g->map.tiles[y][x - 1] == TILE_WALL) {
                    edges |= DUNGEON_EDGE_WEST;
                }
                if (edges != 0) {
                    draw_dungeon_wall_edge(r, sx, sy, x, y, edges);
                }
            }
            if (g->location == LOCATION_FOREST && forest_is_floor(g, x, y)) {
                unsigned int edges = 0;
                if (y > 0 && forest_is_tree(g->map.tiles[y - 1][x])) {
                    edges |= FOREST_EDGE_NORTH;
                }
                if (x < MAP_W - 1 && forest_is_tree(g->map.tiles[y][x + 1])) {
                    edges |= FOREST_EDGE_EAST;
                }
                if (y < MAP_H - 1 && forest_is_tree(g->map.tiles[y + 1][x])) {
                    edges |= FOREST_EDGE_SOUTH;
                }
                if (x > 0 && forest_is_tree(g->map.tiles[y][x - 1])) {
                    edges |= FOREST_EDGE_WEST;
                }
                if (edges != 0) {
                    draw_forest_tree_edge(r, sx, sy, x, y, edges);
                }
            }
            if (game_is_king_road(g) &&
                (g->map.tiles[y][x] == TILE_TOWN_FLOOR ||
                g->map.tiles[y][x] == TILE_TOWN_PATH)) {
                unsigned int edges = 0;
                if (y > 0 && g->map.tiles[y - 1][x] == TILE_FOREST_WALL) {
                    edges |= FOREST_EDGE_NORTH;
                }
                if (x < MAP_W - 1 &&
                    g->map.tiles[y][x + 1] == TILE_FOREST_WALL) {
                    edges |= FOREST_EDGE_EAST;
                }
                if (y < MAP_H - 1 &&
                    g->map.tiles[y + 1][x] == TILE_FOREST_WALL) {
                    edges |= FOREST_EDGE_SOUTH;
                }
                if (x > 0 && g->map.tiles[y][x - 1] == TILE_FOREST_WALL) {
                    edges |= FOREST_EDGE_WEST;
                }
                if (edges != 0) {
                    draw_forest_tree_edge(r, sx, sy, x, y, edges);
                }
            }
            if (g->location == LOCATION_SWAMP && swamp_is_floor(g, x, y)) {
                unsigned int edges = 0;
                if (y > 0 && g->map.tiles[y - 1][x] == TILE_SWAMP_WALL) {
                    edges |= FOREST_EDGE_NORTH;
                }
                if (x < MAP_W - 1 &&
                    g->map.tiles[y][x + 1] == TILE_SWAMP_WALL) {
                    edges |= FOREST_EDGE_EAST;
                }
                if (y < MAP_H - 1 &&
                    g->map.tiles[y + 1][x] == TILE_SWAMP_WALL) {
                    edges |= FOREST_EDGE_SOUTH;
                }
                if (x > 0 && g->map.tiles[y][x - 1] == TILE_SWAMP_WALL) {
                    edges |= FOREST_EDGE_WEST;
                }
                if (edges != 0) {
                    draw_swamp_bank(r, sx, sy, x, y, edges);
                }
            }
            if (g->location == LOCATION_FOREST &&
                x >= landmark_x - 1 && x <= landmark_x + 1 &&
                y >= landmark_y - 1 && y <= landmark_y + 1 &&
                g->map.tiles[y][x] == TILE_FOREST_FLOOR) {
                draw_forest_ruin(r, sx, sy, x - landmark_x, y - landmark_y);
            }
            if (g->location == LOCATION_COAST) {
                TileType tile = coast_visible_tile(g, x, y);
                if (coast_is_water_surface(tile)) {
                    unsigned int edges = 0;
                    if (y > 0 && !coast_is_water_surface(coast_visible_tile(g, x, y - 1))) {
                        edges |= COAST_SHORE_NORTH;
                    }
                    if (x < MAP_W - 1 && !coast_is_water_surface(coast_visible_tile(g, x + 1, y))) {
                        edges |= COAST_SHORE_EAST;
                    }
                    if (y < MAP_H - 1 && !coast_is_water_surface(coast_visible_tile(g, x, y + 1))) {
                        edges |= COAST_SHORE_SOUTH;
                    }
                    if (x > 0 && !coast_is_water_surface(coast_visible_tile(g, x - 1, y))) {
                        edges |= COAST_SHORE_WEST;
                    }
                    if (edges != 0) {
                        draw_coast_shore(r, sx, sy, x, y, edges, tile);
                    }
                }
            }
        }
    }

    // Loot on mechanisms and return passages keeps the underlying tile visible.
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *item = &g->floor_items[i];
        if (!item->active || !viewport_is_visible(v, item->x, item->y)) {
            continue;
        }
        TileType tile = g->map.tiles[item->y][item->x];
        if ((tile != TILE_ITEM || g->location == LOCATION_CASTLE_INTERIOR || g->location == LOCATION_JAIL ||
            g->location == LOCATION_ESCAPE_TUNNEL) && g->location != LOCATION_ISLAND) {
            draw_floor_loot(r, g, item->x, item->y,
                viewport_to_screen_x(v, item->x),
                viewport_to_screen_y(v, item->y));
        }
    }

    // Gates and buildings span several town cells; draw them over the map.
    const TownBuilding *local_building = town_life_building(g->location);
    if (local_building && local_building->town == g->location) {
        town_life_draw_building(r, v, local_building);
    }
    if (g->location == LOCATION_TOWN) {
        draw_town_gate(r,
            viewport_to_screen_x(v, 18), viewport_to_screen_y(v, 0),
            TOWN_EXIT_MOUNTAINS);
        draw_town_gate(r,
            viewport_to_screen_x(v, 0), viewport_to_screen_y(v, 10),
            TOWN_EXIT_FOREST);
        if (town_road_gate) {
            draw_town_gate(r,
                viewport_to_screen_x(v, 0),
                viewport_to_screen_y(v, TOWN_ROAD_GATE_Y),
                TOWN_EXIT_ROAD);
        }
        draw_town_gate(r,
            viewport_to_screen_x(v, TOWN_W - 3), viewport_to_screen_y(v, 10),
            TOWN_EXIT_DUNGEON);
        if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
            draw_town_gate(r,
                viewport_to_screen_x(v, TOWN4_ROAD_X - 2),
                viewport_to_screen_y(v, 0), TOWN_EXIT_MOUNTAINS);
        }
        draw_town_gate(r,
            viewport_to_screen_x(v, 18),
            viewport_to_screen_y(v, TOWN_H - 2), TOWN_EXIT_COAST);
        draw_shop_blacksmith(r,
            viewport_to_screen_x(v, TOWN_BLACKSMITH_X), viewport_to_screen_y(v, TOWN_BLACKSMITH_Y));
        draw_shop_alchemist(r,
            viewport_to_screen_x(v, TOWN_ALCHEMIST_X),
            viewport_to_screen_y(v, TOWN_ALCHEMIST_Y));
        draw_tavern(r,
            viewport_to_screen_x(v, TOWN_TAVERN_X),
            viewport_to_screen_y(v, TOWN_TAVERN_Y));
        draw_harbor(r,
            viewport_to_screen_x(v, TOWN_HARBOR_X),
            viewport_to_screen_y(v, TOWN_HARBOR_Y));
    }

    if (g->location == LOCATION_TOWN4) {
        draw_workshop(r,
            viewport_to_screen_x(v, TOWN4_WORKSHOP_X),
            viewport_to_screen_y(v, TOWN4_WORKSHOP_Y));
        draw_town_hall(r, viewport_to_screen_x(v, TOWN4_HALL_X),
            viewport_to_screen_y(v, TOWN4_HALL_Y));
        draw_town_gate(r, viewport_to_screen_x(v, RIDGESHIRE_ASHEN_GATE_X - 2),
            viewport_to_screen_y(v, 0), TOWN_EXIT_ASHEN);
        draw_town_gate(r, viewport_to_screen_x(v, 0),
            viewport_to_screen_y(v, 10), TOWN_EXIT_ROAD);
        draw_town_gate_south(r, viewport_to_screen_x(v, 20 - 2),
            viewport_to_screen_y(v, TOWN_H - 3));
        if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
            draw_town_gate_south(r, viewport_to_screen_x(v, RIDGESHIRE_MOUNTAIN_ROAD_X - 2),
                viewport_to_screen_y(v, TOWN_H - 3));
        }
        draw_town_gate(r,
            viewport_to_screen_x(v, TOWN_W - 3),
            viewport_to_screen_y(v, TOWN4_DRAGON_GATE_Y - 2),
            TOWN_EXIT_DRAGONSPINE);
    }

    if (g->location == LOCATION_TOWN2) {
        draw_town_gate(r, viewport_to_screen_x(v, 0),
            viewport_to_screen_y(v, 10), TOWN_EXIT_DESERT);
        draw_town_gate(r, viewport_to_screen_x(v, STILLBURY_GLASSDEEP_GATE_X - 2),
            viewport_to_screen_y(v, TOWN_H - 2), TOWN_EXIT_GLASSDEEP);
        if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
            draw_town_gate(r, viewport_to_screen_x(v, TOWN_W - 3),
                viewport_to_screen_y(v, TOWN_ROAD_GATE_Y), TOWN_EXIT_ROAD);
        }
        draw_town_gate(r,
            viewport_to_screen_x(v, 18), viewport_to_screen_y(v, 0), TOWN_EXIT_SWAMP);
        if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
            draw_town_gate(r, viewport_to_screen_x(v, TOWN3_ROAD_X - 2),
                viewport_to_screen_y(v, 0), TOWN_EXIT_SWAMP);
        }
        draw_town_gate(r,
            viewport_to_screen_x(v, TOWN_W - 3), viewport_to_screen_y(v, 10),
            TOWN_EXIT_FOREST);
        draw_witch_hut(r,
            viewport_to_screen_x(v, TOWN_WITCH_X),
            viewport_to_screen_y(v, TOWN_WITCH_Y));
        draw_inn(r,
            viewport_to_screen_x(v, TOWN_INN_X),
            viewport_to_screen_y(v, TOWN_INN_Y));
        draw_healer_house(r, viewport_to_screen_x(v, TOWN_HEALER_X),
            viewport_to_screen_y(v, TOWN_HEALER_Y));
        draw_labyrinth_entrance(r,
            viewport_to_screen_x(v, TOWN_LABYRINTH_X),
            viewport_to_screen_y(v, TOWN_LABYRINTH_Y),
            game_labyrinth_is_open(g));
    }

    if (g->location == LOCATION_TOWN3) {
        draw_town_gate(r, viewport_to_screen_x(v, 0),
            viewport_to_screen_y(v, ROSEMOOR_MOONVEIL_GATE_Y - 2), TOWN_EXIT_MOONVEIL);
        draw_adventurers_guild(r,
            viewport_to_screen_x(v, TOWN_GUILD_X),
            viewport_to_screen_y(v, TOWN_GUILD_Y));
        draw_apothecary(r,
            viewport_to_screen_x(v, TOWN_APOTHECARY_X),
            viewport_to_screen_y(v, TOWN_APOTHECARY_Y));
        draw_town_gate(r, viewport_to_screen_x(v, 18),
            viewport_to_screen_y(v, 0), TOWN_EXIT_FROST);
        draw_town_gate(r, viewport_to_screen_x(v, TOWN_W - 3),
            viewport_to_screen_y(v, TOWN3_KING_GATE_Y - 2), TOWN_EXIT_DUNGEON);
        draw_town_gate(r, viewport_to_screen_x(v, 18),
            viewport_to_screen_y(v, TOWN_H - 2), TOWN_EXIT_SWAMP);
        if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
            draw_town_gate(r, viewport_to_screen_x(v, ROSEMOOR_SWAMP_ROAD_X - 2),
                viewport_to_screen_y(v, TOWN_H - 2), TOWN_EXIT_SWAMP);
        }
    }
    if (g->location == LOCATION_CASTLE) {
        jail_draw_building(r, v);
        draw_castle_front(r,
            viewport_to_screen_x(v, TOWN_MOAT_X),
            viewport_to_screen_y(v, TOWN_MOAT_Y));
        draw_town_gate(r, viewport_to_screen_x(v, 0),
            viewport_to_screen_y(v, CASTLE_ROAD_Y - 2), TOWN_EXIT_ROAD);
        draw_town_gate(r, viewport_to_screen_x(v, TOWN_W - 3),
            viewport_to_screen_y(v, CASTLE_ROAD_Y - 2), TOWN_EXIT_DUNGEON);
    }

    if (g->location == LOCATION_ISLAND) {
        draw_island_temple(r, viewport_to_screen_x(v, 10),
            viewport_to_screen_y(v, 0));
        draw_island_camp(r, viewport_to_screen_x(v, 4),
            viewport_to_screen_y(v, 12));
        draw_island_marker(r, viewport_to_screen_x(v, 8),
            viewport_to_screen_y(v, 7));
        draw_island_statue(r, viewport_to_screen_x(v, 27),
            viewport_to_screen_y(v, 7));
        draw_island_lagoon(r, viewport_to_screen_x(v, 26),
            viewport_to_screen_y(v, 11));
        draw_island_dock(r, viewport_to_screen_x(v, 14),
            viewport_to_screen_y(v, 18));
        draw_island_ship(r, viewport_to_screen_x(v, 25),
            viewport_to_screen_y(v, 17));
        draw_rowan(r, viewport_to_screen_x(v, ISLAND_CAPTAIN_X),
            viewport_to_screen_y(v, ISLAND_CAPTAIN_Y));
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && viewport_is_visible(v, item->x, item->y)) {
                draw_floor_loot(r, g, item->x, item->y,
                    viewport_to_screen_x(v, item->x),
                    viewport_to_screen_y(v, item->y));
            }
        }
    }

    if (g->location == LOCATION_FROSTFELL) {
        draw_kraken_target(r, g, v);
    }

    // Draw enemies
    if (g->location == LOCATION_ESCAPE_TUNNEL || g->location == LOCATION_CASTLE_INTERIOR || g->location == LOCATION_CATACOMBS || g->location == LOCATION_DUNGEON ||
        g->location == LOCATION_FOREST ||
        g->location == LOCATION_MOUNTAINS ||
        g->location == LOCATION_DRAGONSPINE ||
        g->location == LOCATION_COAST ||
        g->location == LOCATION_SWAMP ||
        g->location == LOCATION_DESERT ||
        g->location == LOCATION_MOONVEIL ||
        g->location == LOCATION_ASHEN ||
        g->location == LOCATION_GLASSDEEP ||
        g->location == LOCATION_FROSTFELL ||
        game_is_king_road(g) ||
        g->location == LOCATION_TEMPLE ||
        g->location == LOCATION_LABYRINTH) {
        for (int i = 0; i < g->enemy_count; i++) {
            Enemy *e = &g->enemies[i];
            if (!e->active) continue;
            if (!viewport_is_visible(v, e->x, e->y)) continue;
            int sx = viewport_to_screen_x(v, e->x);
            int sy = viewport_to_screen_y(v, e->y);
            if (g->location == LOCATION_CASTLE_INTERIOR) {
                castle_draw_enemy(r, sx, sy, e);
            } else if (e->type == ENEMY_FALLEN_SUN_GUARDIAN &&
                e->hp <= e->max_hp / 2) {
                draw_fallen_sun_guardian_broken(r, sx, sy);
            } else {
                draw_enemy(r, sx, sy, e->type);
            }
            if (e->dain_fragment) {
                int px = sx * TILE_SIZE;
                int py = sy * TILE_SIZE;
                SDL_SetRenderDrawColor(r->sdl, 242, 190, 55, 255);
                SDL_Rect marker = {px + 2, py + 2, TILE_SIZE - 4,
                    TILE_SIZE - 4};
                SDL_RenderDrawRect(r->sdl, &marker);
                SDL_RenderDrawRect(r->sdl, &marker);
            }
            if (e->type != ENEMY_POLAR_KRAKEN) {
                // Draw health bar above enemy
                int bar_w = TILE_SIZE - 4;
                int bar_h = 3;
                int bar_x = sx * TILE_SIZE + 2;
                int bar_y = sy * TILE_SIZE - 5;
                if (g->location == LOCATION_CASTLE_INTERIOR && e->is_boss) {
                    bar_w = 32;
                    bar_x = sx * TILE_SIZE - 4;
                    bar_y = sy * TILE_SIZE - 17;
                }
                int fill_w = (bar_w * e->hp) / e->max_hp;
                SDL_Rect bg = {bar_x, bar_y, bar_w, bar_h};
                SDL_Rect fill = {bar_x, bar_y, fill_w, bar_h};
                SDL_SetRenderDrawColor(r->sdl, 60, 20, 20, 255);
                SDL_RenderFillRect(r->sdl, &bg);
                SDL_SetRenderDrawColor(r->sdl, 200, 60, 60, 255);
                SDL_RenderFillRect(r->sdl, &fill);
            }
            if (e->frozen_turns > 0) {
                draw_frozen_status(r, sx * TILE_SIZE, sy * TILE_SIZE, e->frozen_turns);
            }
        }
    }

    // Area labels share a palette; town destinations use wooden signs.
    if (g->location == LOCATION_TOWN) {
        SDL_Color label = {220, 180, 60, 255};
        int blacksmith_w = 0;
        int alchemist_w = 0;
        int forest_w = 0;
        int forest_h = 0;
        int dungeon_w = 0;
        int dungeon_h = 0;
        int mountains_w = 0;
        int mountains_h = 0;
        int tavern_w = 0;
        int harbor_w = 0;
        int coast_w = 0;
        int coast_h = 0;
        TTF_SizeText(r->font_tiny, "BLACKSMITH", &blacksmith_w, NULL);
        TTF_SizeText(r->font_tiny, "ALCHEMIST", &alchemist_w, NULL);
        TTF_SizeText(r->font_tiny, "FOREST", &forest_w, &forest_h);
        TTF_SizeText(r->font_tiny, "DUNGEON", &dungeon_w, &dungeon_h);
        TTF_SizeText(r->font_tiny, "MOUNTAINS", &mountains_w, &mountains_h);
        TTF_SizeText(r->font_tiny, "TAVERN", &tavern_w, NULL);
        TTF_SizeText(r->font_tiny, "HARBOR", &harbor_w, NULL);
        TTF_SizeText(r->font_tiny, "SUNKEN COAST", &coast_w, &coast_h);
        int bx = viewport_to_screen_x(v, TOWN_BLACKSMITH_X) * TILE_SIZE
            + (5 * TILE_SIZE - blacksmith_w) / 2;
        int by = viewport_to_screen_y(v, TOWN_BLACKSMITH_Y - 1) * TILE_SIZE;
        int ax = viewport_to_screen_x(v, TOWN_ALCHEMIST_X) * TILE_SIZE
            + (5 * TILE_SIZE - alchemist_w) / 2;
        int ay = viewport_to_screen_y(v, TOWN_ALCHEMIST_Y - 1) * TILE_SIZE;
        int tavern_x = viewport_to_screen_x(v, TOWN_TAVERN_X) * TILE_SIZE
            + (TOWN_TAVERN_W * TILE_SIZE - tavern_w) / 2;
        int tavern_y = viewport_to_screen_y(v, TOWN_TAVERN_Y - 1) * TILE_SIZE;
        int harbor_x = viewport_to_screen_x(v, TOWN_HARBOR_X) * TILE_SIZE
            + (TOWN_HARBOR_W * TILE_SIZE - harbor_w) / 2;
        int harbor_y = viewport_to_screen_y(v, TOWN_HARBOR_Y - 1) * TILE_SIZE;
        if (bx > 0 && by > 0) {
            renderer_draw_text(r, "BLACKSMITH", bx, by, label, r->font_tiny);
        }
        if (ax > 0 && ay > 0) {
            renderer_draw_text(r, "ALCHEMIST", ax, ay, label, r->font_tiny);
        }
        if (tavern_x > 0 && tavern_y > 0) {
            renderer_draw_text(r, "TAVERN", tavern_x, tavern_y, label,
                r->font_tiny);
        }
        if (harbor_x > 0 && harbor_y > 0) {
            renderer_draw_text(r, "HARBOR", harbor_x, harbor_y,
                label, r->font_tiny);
        }
        int gate_top = viewport_to_screen_y(v, 10) * TILE_SIZE;
        int forest_x = viewport_to_screen_x(v, 1) * TILE_SIZE + 8;
        int forest_y = gate_top + (5 * TILE_SIZE - forest_h) / 2;
        int mountains_x = viewport_to_screen_x(v, 18) * TILE_SIZE
            + (5 * TILE_SIZE - mountains_w) / 2;
        int mountains_y = viewport_to_screen_y(v, 1) * TILE_SIZE
            + (TILE_SIZE - mountains_h) / 2;
        int dungeon_x = viewport_to_screen_x(v, TOWN_W - 1) * TILE_SIZE
            - dungeon_w - 8;
        int dungeon_y = gate_top + (5 * TILE_SIZE - dungeon_h) / 2;
        renderer_draw_text(r, "FOREST", forest_x, forest_y,
            area_label_color(LOCATION_FOREST), r->font_tiny);
        renderer_draw_text(r, "DUNGEON", dungeon_x, dungeon_y, area_label_color(LOCATION_DUNGEON),
            r->font_tiny);
        renderer_draw_text(r, "MOUNTAINS", mountains_x, mountains_y,
            area_label_color(LOCATION_MOUNTAINS), r->font_tiny);
        if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
            draw_town_road_sign(r, viewport_to_screen_x(v, TOWN4_ROAD_X - 5),
                viewport_to_screen_y(v, 2), "RIDGESHIRE");
        }
        int coast_x = viewport_to_screen_x(v, 18) * TILE_SIZE
            + (5 * TILE_SIZE - coast_w) / 2;
        int coast_y = viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE
            + (TILE_SIZE - coast_h) / 2;
        renderer_draw_text(r, "SUNKEN COAST", coast_x, coast_y,
            area_label_color(LOCATION_COAST), r->font_tiny);
        if (town_road_gate) {
            draw_town_road_sign(r, viewport_to_screen_x(v, 1),
                viewport_to_screen_y(v, TOWN_ROAD_GATE_Y - 1), "STILLBURY");
        }
    }

    if (g->location == LOCATION_TOWN4) {
        int width = 0;
        TTF_SizeText(r->font_tiny, "ASHEN HOLLOW", &width, NULL);
        renderer_draw_text(r, "ASHEN HOLLOW",
            viewport_to_screen_x(v, RIDGESHIRE_ASHEN_GATE_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, 1) * TILE_SIZE,
            area_label_color(LOCATION_ASHEN), r->font_tiny);
        TTF_SizeText(r->font_tiny, "DRAGONSPINE", &width, NULL);
        renderer_draw_text(r, "DRAGONSPINE",
            viewport_to_screen_x(v, TOWN_W - 1) * TILE_SIZE - width - 8,
            viewport_to_screen_y(v, TOWN4_DRAGON_GATE_Y) * TILE_SIZE,
            area_label_color(LOCATION_DRAGONSPINE), r->font_tiny);
        TTF_SizeText(r->font_tiny, "MOUNTAINS", &width, NULL);
        renderer_draw_text(r, "MOUNTAINS",
            viewport_to_screen_x(v, 20) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE,
            area_label_color(LOCATION_MOUNTAINS), r->font_tiny);
        if (g->defeated_bosses & (1 << LOCATION_MOUNTAINS)) {
            draw_town_road_sign(r, viewport_to_screen_x(v, RIDGESHIRE_MOUNTAIN_ROAD_X + 3),
                viewport_to_screen_y(v, TOWN_H - 4), "OAKHAVEN");
        }
    }

    if (g->location == LOCATION_TOWN2) {
        SDL_Color label = {220, 180, 60, 255};
        int width = 0;
        renderer_draw_text(r, "SUNSCAR WASTES",
            viewport_to_screen_x(v, 1) * TILE_SIZE,
            viewport_to_screen_y(v, 12) * TILE_SIZE,
            area_label_color(LOCATION_DESERT), r->font_tiny);
        TTF_SizeText(r->font_tiny, "GLASSDEEP", &width, NULL);
        renderer_draw_text(r, "GLASSDEEP",
            viewport_to_screen_x(v, STILLBURY_GLASSDEEP_GATE_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE + 8,
            area_label_color(LOCATION_GLASSDEEP), r->font_tiny);
        TTF_SizeText(r->font_tiny, "SWAMP", &width, NULL);
        renderer_draw_text(r, "SWAMP",
            viewport_to_screen_x(v, CROWNROAD_X) * TILE_SIZE +
                (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, 1) * TILE_SIZE + 3,
            area_label_color(LOCATION_SWAMP), r->font_tiny);
        TTF_SizeText(r->font_tiny, "HEALER", &width, NULL);
        renderer_draw_text(r, "HEALER",
            viewport_to_screen_x(v, TOWN_HEALER_X) * TILE_SIZE +
                (TOWN_HEALER_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_HEALER_Y - 1) * TILE_SIZE,
            label, r->font_tiny);
        TTF_SizeText(r->font_tiny, "WITCH", &width, NULL);
        renderer_draw_text(r, "WITCH",
            viewport_to_screen_x(v, TOWN_WITCH_X) * TILE_SIZE +
                (TOWN_WITCH_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_WITCH_Y - 1) * TILE_SIZE,
            label, r->font_tiny);
        TTF_SizeText(r->font_tiny, "INN", &width, NULL);
        renderer_draw_text(r, "INN",
            viewport_to_screen_x(v, TOWN_INN_X) * TILE_SIZE +
                (TOWN_INN_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_INN_Y - 1) * TILE_SIZE,
            label, r->font_tiny);
        TTF_SizeText(r->font_tiny, "LABYRINTH", &width, NULL);
        renderer_draw_text(r, "LABYRINTH",
            viewport_to_screen_x(v, TOWN_LABYRINTH_X) * TILE_SIZE +
                (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_LABYRINTH_Y - 3) * TILE_SIZE,
            game_labyrinth_is_open(g) ?
                label : (SDL_Color){105, 105, 90, 255}, r->font_tiny);
        TTF_SizeText(r->font_tiny, "FOREST", &width, NULL);
        renderer_draw_text(r, "FOREST",
            viewport_to_screen_x(v, TOWN_W - 1) * TILE_SIZE - width - 8,
            viewport_to_screen_y(v, 12) * TILE_SIZE,
            area_label_color(LOCATION_FOREST), r->font_tiny);
        if (g->defeated_bosses & (1 << LOCATION_FOREST)) {
            draw_town_road_sign(r, viewport_to_screen_x(v, TOWN_W - 6),
                viewport_to_screen_y(v, TOWN_ROAD_GATE_Y - 1), "OAKHAVEN");
        }
        if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
            draw_town_road_sign(r, viewport_to_screen_x(v, TOWN3_ROAD_X + 3),
                viewport_to_screen_y(v, 2), "ROSEMOOR");
        }
    }

    if (g->location == LOCATION_TOWN3) {
        SDL_Color label = {220, 180, 60, 255};
        int width = 0;
        TTF_SizeText(r->font_tiny, "ADVENTURER'S GUILD", &width, NULL);
        renderer_draw_text(r, "ADVENTURER'S GUILD",
            viewport_to_screen_x(v, TOWN_GUILD_X) * TILE_SIZE +
                (TOWN_GUILD_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_GUILD_Y - 1) * TILE_SIZE,
            label, r->font_tiny);
        TTF_SizeText(r->font_tiny, "FROSTFELL", &width, NULL);
        renderer_draw_text(r, "FROSTFELL",
            viewport_to_screen_x(v, 20) * TILE_SIZE - width / 2,
            viewport_to_screen_y(v, 1) * TILE_SIZE,
            area_label_color(LOCATION_FROSTFELL), r->font_tiny);
        renderer_draw_text(r, "MOONVEIL",
            viewport_to_screen_x(v, 2) * TILE_SIZE + 8,
            viewport_to_screen_y(v, ROSEMOOR_MOONVEIL_GATE_Y) * TILE_SIZE,
            area_label_color(LOCATION_MOONVEIL), r->font_tiny);
        TTF_SizeText(r->font_tiny, "CROWN ROAD EAST", &width, NULL);
        renderer_draw_text(r, "CROWN ROAD EAST",
            viewport_to_screen_x(v, TOWN_W - 1) * TILE_SIZE - width - 8,
            viewport_to_screen_y(v, TOWN3_KING_GATE_Y) * TILE_SIZE,
            area_label_color(LOCATION_CROWNROAD), r->font_tiny);
        TTF_SizeText(r->font_tiny, "SWAMP", &width, NULL);
        renderer_draw_text(r, "SWAMP",
            viewport_to_screen_x(v, 20) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE, area_label_color(LOCATION_SWAMP), r->font_tiny);
        if (g->defeated_bosses & (1 << LOCATION_SWAMP)) {
            draw_town_road_sign(r, viewport_to_screen_x(v, ROSEMOOR_SWAMP_ROAD_X + 3),
                viewport_to_screen_y(v, TOWN_H - 4), "STILLBURY");
        }
        TTF_SizeText(r->font_tiny, "APOTHECARY", &width, NULL);
        renderer_draw_text(r, "APOTHECARY",
            viewport_to_screen_x(v, TOWN_APOTHECARY_X) * TILE_SIZE +
                (TOWN_APOTHECARY_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN_APOTHECARY_Y - 1) * TILE_SIZE,
            label, r->font_tiny);
    }
    if (g->location == LOCATION_TOWN4) {
        renderer_draw_text(r, "CROWN ROAD WEST",
            viewport_to_screen_x(v, 1) * TILE_SIZE + 8,
            viewport_to_screen_y(v, 12) * TILE_SIZE,
            area_label_color(LOCATION_KING_ROAD_WEST), r->font_tiny);
        int width = 0;
        TTF_SizeText(r->font_tiny, "WORKSHOP", &width, NULL);
        renderer_draw_text(r, "WORKSHOP",
            viewport_to_screen_x(v, TOWN4_WORKSHOP_X) * TILE_SIZE +
                (TOWN4_WORKSHOP_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN4_WORKSHOP_Y - 1) * TILE_SIZE,
            (SDL_Color){220, 180, 60, 255}, r->font_tiny);
        TTF_SizeText(r->font_tiny, "TOWN HALL", &width, NULL);
        renderer_draw_text(r, "TOWN HALL",
            viewport_to_screen_x(v, TOWN4_HALL_X) * TILE_SIZE + (TOWN4_HALL_W * TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, TOWN4_HALL_Y - 1) * TILE_SIZE,
            (SDL_Color){220, 180, 60, 255}, r->font_tiny);
    }
    if (g->location == LOCATION_CASTLE) {
        if (g->castle_minibosses & 1) {
            int sx = viewport_to_screen_x(v, 17);
            int sy = viewport_to_screen_y(v, 12);
            castle_draw_tile(r, g, sx, sy, 17, 12, TILE_CASTLE_PASSAGE);
            renderer_draw_text(r, "KEEP (A)", sx * 24 - 20, sy * 24 + 25, (SDL_Color){126, 224, 166, 255}, r->font_tiny);
        }
        if (g->castle_minibosses & 2) {
            int sx = viewport_to_screen_x(v, 23);
            int sy = viewport_to_screen_y(v, 12);
            castle_draw_tile(r, g, sx, sy, 23, 12, TILE_CASTLE_PASSAGE);
            renderer_draw_text(r, "CHAPEL (A)", sx * 24 - 28, sy * 24 + 25, (SDL_Color){126, 224, 166, 255}, r->font_tiny);
        }
        int gx = viewport_to_screen_x(v, CROWNROAD_X - 2) * TILE_SIZE;
        int gy = viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE;
        SDL_Rect left_post = {gx, gy, 14, TILE_SIZE * 2};
        SDL_Rect right_post = {gx + TILE_SIZE * 5 - 14, gy, 14, TILE_SIZE * 2};
        SDL_SetRenderDrawColor(r->sdl, 91, 88, 81, 255);
        SDL_RenderFillRect(r->sdl, &left_post);
        SDL_RenderFillRect(r->sdl, &right_post);
        SDL_SetRenderDrawColor(r->sdl, 224, 216, 186, 255);
        SDL_RenderDrawLine(r->sdl, gx + 3, gy + 3, gx + 3, gy + TILE_SIZE * 2 - 3);
        SDL_RenderDrawLine(r->sdl, right_post.x + 3, gy + 3, right_post.x + 3, gy + TILE_SIZE * 2 - 3);
        int gate_width = 0;
        TTF_SizeText(r->font_tiny, "CATACOMBS", &gate_width, NULL);
        renderer_draw_text(r, "CATACOMBS", viewport_to_screen_x(v, CROWNROAD_X) * TILE_SIZE + (TILE_SIZE - gate_width) / 2,
            viewport_to_screen_y(v, TOWN_H - 2) * TILE_SIZE, area_label_color(LOCATION_CATACOMBS), r->font_tiny);
        SDL_Color label = {220, 180, 60, 255};
        int width = 0;
        TTF_SizeText(r->font_tiny, "CASTLE OF NO RETURN", &width, NULL);
        renderer_draw_text(r, "CASTLE OF NO RETURN",
            viewport_to_screen_x(v, 20) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, 1) * TILE_SIZE, label, r->font_tiny);
        draw_town_road_sign(r, viewport_to_screen_x(v, 1),
            viewport_to_screen_y(v, CASTLE_ROAD_Y - 4), "ROSEMOOR");
        draw_town_road_sign(r, viewport_to_screen_x(v, TOWN_W - 6),
            viewport_to_screen_y(v, CASTLE_ROAD_Y - 4), "RIDGESHIRE");
    }
    if (game_is_king_road(g)) {
        int east = g->location == LOCATION_CROWNROAD;
        draw_town_road_sign(r, viewport_to_screen_x(v, 1),
            viewport_to_screen_y(v, CROWNROAD_Y - 3), east ? "ROSEMOOR" : "CASTLE");
        draw_town_road_sign(r, viewport_to_screen_x(v, CROWNROAD_W - 6),
            viewport_to_screen_y(v, CROWNROAD_Y - 3), east ? "CASTLE" : "RIDGESHIRE");
    }

    if (g->location == LOCATION_TAVERN) {
        SDL_Color name = {182, 214, 232, 255};
        int name_w = 0;
        TTF_SizeText(r->font_tiny, "ELOWEN", &name_w, NULL);
        renderer_draw_text(r, "ELOWEN", viewport_to_screen_x(v, ELOWEN_TAVERN_X) * TILE_SIZE + (TILE_SIZE - name_w) / 2,
            viewport_to_screen_y(v, ELOWEN_TAVERN_Y - 1) * TILE_SIZE, name, r->font_tiny);
        TTF_SizeText(r->font_tiny, "MARA", &name_w, NULL);
        int name_x = viewport_to_screen_x(v, MARA_TAVERN_X) * TILE_SIZE + (TILE_SIZE - name_w) / 2;
        int name_y = viewport_to_screen_y(v, MARA_TAVERN_Y - 1) * TILE_SIZE;
        renderer_draw_text(r, "MARA", name_x, name_y, area_label_color(LOCATION_COAST), r->font_tiny);
        TTF_SizeText(r->font_tiny, "ILYA", &name_w, NULL);
        name_x = viewport_to_screen_x(v, ILYA_TAVERN_X) * TILE_SIZE + (TILE_SIZE - name_w) / 2;
        name_y = viewport_to_screen_y(v, ILYA_TAVERN_Y - 1) * TILE_SIZE;
        renderer_draw_text(r, "ILYA", name_x, name_y, area_label_color(LOCATION_DRAGONSPINE), r->font_tiny);
    }

    if (g->location == LOCATION_MOONVEIL) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                TileType tile = g->map.tiles[y][x];
                const char *label = tile == TILE_MOONVEIL_SEED_POD ? "MOONSEED (A)" :
                    tile == TILE_MOONVEIL_SPRING ? (g->moonveil_quest_progress & MOONVEIL_WATER_GATHERED ? "MOONLIT SPRING" : "MOONWATER (A)") :
                    tile == TILE_MOONVEIL_PLANTING_CIRCLE ? "PLANTING CIRCLE (A)" :
                    tile == TILE_MOONVEIL_MOONFLOWER ? "MOONFLOWER" : NULL;
                if (!label || !viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y)) {
                    continue;
                }
                int width = 0;
                TTF_SizeText(r->font_tiny, label, &width, NULL);
                renderer_draw_text(r, label, viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2,
                    viewport_to_screen_y(v, y) * TILE_SIZE - 12, area_label_color(LOCATION_MOONVEIL), r->font_tiny);
            }
        }
    }

    if (g->location == LOCATION_GLASSDEEP) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                TileType tile = g->map.tiles[y][x];
                if ((tile != TILE_GLASSDEEP_RESONATOR && tile != TILE_GLASSDEEP_RESONATOR_LIT) ||
                    !viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y)) {
                    continue;
                }
                const char *label = tile == TILE_GLASSDEEP_RESONATOR_LIT ? "RESTORED" : "RESONATOR (A)";
                int width = 0;
                TTF_SizeText(r->font_tiny, label, &width, NULL);
                renderer_draw_text(r, label, viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2,
                    viewport_to_screen_y(v, y) * TILE_SIZE - 12, area_label_color(LOCATION_GLASSDEEP), r->font_tiny);
            }
        }
    }

    if (g->location == LOCATION_CATACOMBS && g->catacombs_quest_state) {
        static const char *names[3] = {"SOLDIERS' MEMORIAL", "WATCHERS' MEMORIAL", "CHOIR MEMORIAL"};
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                TileType tile = g->map.tiles[y][x];
                const char *label = tile == TILE_BURIAL_LEDGER ?
                    (g->defeated_bosses & (1 << LOCATION_CATACOMBS) ? "BURIAL LEDGER (A)" : "SEALED LEDGER") :
                    (tile == TILE_MEMORIAL_BRAZIER || tile == TILE_MEMORIAL_COLD) && g->level >= 2 && g->level <= 4 ? names[g->level - 2] : NULL;
                if (!label || !viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y)) {
                    continue;
                }
                int width = 0;
                TTF_SizeText(r->font_tiny, label, &width, NULL);
                renderer_draw_text(r, label, viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2,
                    viewport_to_screen_y(v, y) * TILE_SIZE - 12, area_label_color(LOCATION_CATACOMBS), r->font_tiny);
            }
        }
    }

    if (g->location == LOCATION_FROSTFELL) {
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                TileType tile = g->map.tiles[y][x];
                if ((tile != TILE_FROST_JOURNAL && tile != TILE_NPC_FROST_SURVIVOR) ||
                    !viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y)) {
                    continue;
                }
                const char *label = tile == TILE_FROST_JOURNAL ? "JOURNAL (A)" : "SURVEYOR FEN";
                int width = 0;
                TTF_SizeText(r->font_tiny, label, &width, NULL);
                renderer_draw_text(r, label, viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2,
                    viewport_to_screen_y(v, y) * TILE_SIZE - 12, area_label_color(LOCATION_FROSTFELL), r->font_tiny);
            }
        }
    }

    if (g->location == LOCATION_TOWN3) {
        int width = 0;
        TTF_SizeText(r->font_tiny, "LIORA", &width, NULL);
        renderer_draw_text(r, "LIORA", viewport_to_screen_x(v, LIORA_TOWN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, LIORA_TOWN_Y - 1) * TILE_SIZE, area_label_color(LOCATION_MOONVEIL), r->font_tiny);
    }
    if (g->location == LOCATION_INN) {
        int width = 0;
        TTF_SizeText(r->font_tiny, "BRENNA", &width, NULL);
        renderer_draw_text(r, "BRENNA", viewport_to_screen_x(v, BRENNA_INN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, BRENNA_INN_Y - 1) * TILE_SIZE, area_label_color(LOCATION_FROSTFELL), r->font_tiny);
        TTF_SizeText(r->font_tiny, "ZARA", &width, NULL);
        renderer_draw_text(r, "ZARA", viewport_to_screen_x(v, ZARA_INN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, ZARA_INN_Y - 1) * TILE_SIZE, (SDL_Color){233, 201, 133, 255}, r->font_tiny);
        TTF_SizeText(r->font_tiny, "ALDER", &width, NULL);
        renderer_draw_text(r, "ALDER", viewport_to_screen_x(v, ALDER_INN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, ALDER_INN_Y - 1) * TILE_SIZE, (SDL_Color){126, 190, 112, 255}, r->font_tiny);
        renderer_draw_text(r, "ROOK",
            viewport_to_screen_x(v, 10) * TILE_SIZE - 8,
            viewport_to_screen_y(v, 17) * TILE_SIZE,
            (SDL_Color){182, 214, 232, 255}, r->font_tiny);
        renderer_draw_text(r, "BRAM",
            viewport_to_screen_x(v, 28) * TILE_SIZE - 8,
            viewport_to_screen_y(v, 6) * TILE_SIZE,
            (SDL_Color){233, 201, 133, 255}, r->font_tiny);
    }
    if (g->location == LOCATION_WORKSHOP) {
        renderer_draw_text(r, "GARRICK - SHARPENING", viewport_to_screen_x(v, WORKSHOP_SMITH_X) * TILE_SIZE - 60,
            viewport_to_screen_y(v, WORKSHOP_SMITH_Y - 1) * TILE_SIZE, (SDL_Color){233, 201, 133, 255}, r->font_tiny);
    }
    if (g->location == LOCATION_TOWN_HALL) {
        renderer_draw_text(r, "STEWARD HADRIN", viewport_to_screen_x(v, HALL_STEWARD_X) * TILE_SIZE - 45,
            viewport_to_screen_y(v, HALL_STEWARD_Y - 1) * TILE_SIZE, (SDL_Color){233, 201, 133, 255}, r->font_tiny);
        int width = 0;
        TTF_SizeText(r->font_tiny, "BROTHER OSWIN", &width, NULL);
        renderer_draw_text(r, "BROTHER OSWIN", viewport_to_screen_x(v, HALL_OSWIN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, HALL_OSWIN_Y - 1) * TILE_SIZE, area_label_color(LOCATION_CATACOMBS), r->font_tiny);
    }
    if (g->location == LOCATION_ASHEN && g->emberforge_quest_state && g->map.room_count &&
        (g->level == EMBERFORGE_MECHANISM_LEVEL || g->level == EMBERFORGE_FURNACE_LEVEL)) {
        int x;
        int y;
        map_room_center(&g->map.rooms[g->map.room_count - 1], &x, &y);
        if (viewport_is_visible(v, x, y) && map_is_explored(&g->map, x, y) &&
            (g->level == EMBERFORGE_FURNACE_LEVEL || !(g->emberforge_progress & EMBERFORGE_MECHANISM_RECOVERED))) {
            const char *label = g->level == EMBERFORGE_MECHANISM_LEVEL ? "MECHANISM (A)" :
                (g->emberforge_progress & EMBERFORGE_RESTORED ? "EMBERFORGE" : "EMBERFORGE (A)");
            int width = 0;
            TTF_SizeText(r->font_tiny, label, &width, NULL);
            renderer_draw_text(r, label, viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2,
                viewport_to_screen_y(v, y) * TILE_SIZE - 12, (SDL_Color){240, 190, 95, 255}, r->font_tiny);
        }
    }
    if (g->location == LOCATION_TOWN4) {
        int width = 0;
        TTF_SizeText(r->font_tiny, "DAIN", &width, NULL);
        renderer_draw_text(r, "DAIN", viewport_to_screen_x(v, DAIN_TOWN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, DAIN_TOWN_Y - 1) * TILE_SIZE, (SDL_Color){218, 164, 84, 255}, r->font_tiny);
    }
    if (g->location == LOCATION_GUILD) {
        int width = 0;
        TTF_SizeText(r->font_tiny, "ORIN", &width, NULL);
        renderer_draw_text(r, "ORIN", viewport_to_screen_x(v, GUILD_ORIN_X) * TILE_SIZE + (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, GUILD_ORIN_Y - 1) * TILE_SIZE, area_label_color(LOCATION_GLASSDEEP), r->font_tiny);
    }

    if (g->location == LOCATION_ISLAND) {
        int width = 0;
        TTF_SizeText(r->font_tiny, "RUINED TEMPLE", &width, NULL);
        renderer_draw_text(r, "RUINED TEMPLE",
            viewport_to_screen_x(v, 20) * TILE_SIZE - width / 2,
            viewport_to_screen_y(v, 1) * TILE_SIZE, area_label_color(LOCATION_TEMPLE), r->font_tiny);
        TTF_SizeText(r->font_tiny, "CAPTAIN ROWAN", &width, NULL);
        renderer_draw_text(r, "CAPTAIN ROWAN",
            viewport_to_screen_x(v, ISLAND_CAPTAIN_X) * TILE_SIZE +
                (TILE_SIZE - width) / 2,
            viewport_to_screen_y(v, ISLAND_CAPTAIN_Y - 1) * TILE_SIZE,
            (SDL_Color){182, 214, 232, 255}, r->font_tiny);
    }

    // Draw spell/projectile trail
    if (g->trail_frames > 0) {
        int timed_fireball = g->trail_effect == TRAIL_EFFECT_FIREBALL &&
            g->trail_count > 0;
        if (timed_fireball) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_TRAVEL_MS) {
                int lead = (int)(elapsed * g->trail_count / SPELL_TRAVEL_MS);
                if (lead >= g->trail_count) {
                    lead = g->trail_count - 1;
                }
                TrailTile *t = &g->trail[lead];
                if (viewport_is_visible(v, t->x, t->y)) {
                    draw_fireball(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), g->player.last_dx,
                        g->player.last_dy, (int)(elapsed / 60));
                }
            } else if (elapsed < SPELL_FIREBALL_MS) {
                TrailTile *t = &g->trail[g->trail_count - 1];
                if (viewport_is_visible(v, t->x, t->y)) {
                    draw_fireball_impact(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), elapsed - SPELL_TRAVEL_MS);
                }
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_MAGIC_ARROW &&
            g->trail_count > 0) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_ARROW_MS) {
                int impact = elapsed >= SPELL_TRAVEL_MS;
                int lead = impact ? g->trail_count - 1
                    : (int)(elapsed * g->trail_count / SPELL_TRAVEL_MS);
                TrailTile *t = &g->trail[lead];
                if (t->active && viewport_is_visible(v, t->x, t->y)) {
                    draw_magic_arrow(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), g->player.last_dx,
                        g->player.last_dy, impact && t->is_impact,
                        (int)(elapsed / 60));
                }
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_DEMONIC_SWORD &&
            g->trail_count > 0) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_ARROW_MS) {
                int impact = elapsed >= SPELL_TRAVEL_MS;
                int lead = impact ? g->trail_count - 1
                    : (int)(elapsed * g->trail_count / SPELL_TRAVEL_MS);
                TrailTile *t = &g->trail[lead];
                if (t->active && viewport_is_visible(v, t->x, t->y)) {
                    draw_demonic_sword(r, viewport_to_screen_x(v, t->x),
                        viewport_to_screen_y(v, t->y), g->player.last_dx,
                        g->player.last_dy, impact && t->is_impact,
                        (int)(elapsed / 60));
                }
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_WEAPON_ARROW) {
            Uint32 elapsed = SDL_GetTicks() - g->trail_started_at;
            if (elapsed < SPELL_ARROW_MS && g->trail_count > 0) {
                int impact = elapsed >= SPELL_TRAVEL_MS;
                float progress = impact ? 1.0f
                    : (float)elapsed / SPELL_TRAVEL_MS;
                TrailTile *target = &g->trail[g->trail_count - 1];
                int start_x = viewport_to_screen_x(v, g->player.x) *
                    TILE_SIZE + TILE_SIZE / 2;
                int start_y = viewport_to_screen_y(v, g->player.y) *
                    TILE_SIZE + TILE_SIZE / 2;
                int target_x = viewport_to_screen_x(v, target->x) *
                    TILE_SIZE + TILE_SIZE / 2;
                int target_y = viewport_to_screen_y(v, target->y) *
                    TILE_SIZE + TILE_SIZE / 2;
                int arrow_x = start_x + (int)((target_x - start_x) * progress);
                int arrow_y = start_y + (int)((target_y - start_y) * progress);
                draw_weapon_arrow_at(r, arrow_x, arrow_y,
                    g->player.last_dx, g->player.last_dy,
                    impact && target->is_impact);
            } else {
                g->trail_frames = 0;
            }
        } else if (g->trail_effect == TRAIL_EFFECT_ICE_SLIDE) {
            // The slide is shown by moving the player sprite, drawn below.
        } else for (int i = 0; i < g->trail_count; i++) {
            TrailTile *t = &g->trail[i];
            if (!t->active) continue;
            if (!viewport_is_visible(v, t->x, t->y)) continue;
            int sx = viewport_to_screen_x(v, t->x);
            int sy = viewport_to_screen_y(v, t->y);
            SDL_Rect tr = {
                sx * TILE_SIZE, sy * TILE_SIZE,
                TILE_SIZE, TILE_SIZE
            };
            Uint8 alpha = t->is_impact ? 200 : 120;
            SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r->sdl, t->r, t->g, t->b, alpha);
            SDL_RenderFillRect(r->sdl, &tr);
            if (t->is_impact) {
                SDL_SetRenderDrawColor(r->sdl, t->r, t->g, t->b, 255);
                SDL_RenderDrawLine(r->sdl,
                    sx * TILE_SIZE + TILE_SIZE / 2,
                    sy * TILE_SIZE + 4,
                    sx * TILE_SIZE + TILE_SIZE / 2,
                    sy * TILE_SIZE + TILE_SIZE - 4);
                SDL_RenderDrawLine(r->sdl,
                    sx * TILE_SIZE + 4,
                    sy * TILE_SIZE + TILE_SIZE / 2,
                    sx * TILE_SIZE + TILE_SIZE - 4,
                    sy * TILE_SIZE + TILE_SIZE / 2);
            }
        }
        if (!timed_fireball &&
            g->trail_effect != TRAIL_EFFECT_MAGIC_ARROW &&
            g->trail_effect != TRAIL_EFFECT_DEMONIC_SWORD &&
            g->trail_effect != TRAIL_EFFECT_WEAPON_ARROW &&
            g->trail_effect != TRAIL_EFFECT_ICE_SLIDE) {
            g->trail_frames--;
        }
    }

    if (g->dialogue_active && g->location == LOCATION_FOREST &&
        strcmp(g->dialogue_speaker, "Forest Warden") == 0 &&
        viewport_is_visible(v, g->dialogue_x, g->dialogue_y)) {
        draw_forest_warden(r,
            viewport_to_screen_x(v, g->dialogue_x),
            viewport_to_screen_y(v, g->dialogue_y),
            g->dialogue_x, g->dialogue_y);
    }
    if (g->dialogue_active && g->location == LOCATION_SWAMP &&
        strcmp(g->dialogue_speaker, "Mira") == 0 &&
        viewport_is_visible(v, g->dialogue_x, g->dialogue_y)) {
        draw_swamp_daughter(r,
            viewport_to_screen_x(v, g->dialogue_x),
            viewport_to_screen_y(v, g->dialogue_y),
            g->dialogue_x, g->dialogue_y);
    }

    // Draw player
    const Item *equipped_weapon = NULL;
    if (g->equipped_main_hand >= 0 &&
        g->equipped_main_hand < g->inventory_count) {
        equipped_weapon = &g->inventory[g->equipped_main_hand];
    }
    const Item *off_hand_weapon = NULL;
    if (g->equipped_off_hand >= 0 &&
        g->equipped_off_hand < g->inventory_count) {
        off_hand_weapon = &g->inventory[g->equipped_off_hand];
    }
    const Item *equipped_armor = NULL;
    if (g->equipped_armor >= 0 &&
        g->equipped_armor < g->inventory_count) {
        equipped_armor = &g->inventory[g->equipped_armor];
    }
    if (jail_prisoner_at(g, g->prisoner_x, g->prisoner_y) && viewport_is_visible(v, g->prisoner_x, g->prisoner_y)) {
        jail_draw_person(r, viewport_to_screen_x(v, g->prisoner_x), viewport_to_screen_y(v, g->prisoner_y), 1);
    }
    int player_px = viewport_to_screen_x(v, g->player.x) * TILE_SIZE;
    int player_py = viewport_to_screen_y(v, g->player.y) * TILE_SIZE;
    ice_slide_offset(g, &player_px, &player_py);
    draw_player_at(r, player_px, player_py,
        g->player.player_class, equipped_weapon, off_hand_weapon,
        equipped_armor,
        g->player.last_dx, g->player.last_dy);
    if (g->player.poison_turns > 0) {
        draw_icon_poison(r, player_px + 16, player_py + 1);
    }
    if (g->player.frozen_turns > 0) {
        draw_frozen_status(r, player_px, player_py, g->player.frozen_turns);
    }

    if (g->location == LOCATION_FROSTFELL) {
        draw_region_weather(r, v, 0);
    } else if (g->location == LOCATION_DESERT) {
        draw_region_weather(r, v, 1);
    }
    draw_prism_warning(r, g, v);
    draw_catacombs_warnings(r, g, v);
    castle_draw_warnings(r, g, v);

    draw_combat_feedback(r, g, v);

    draw_dialogue_bubble(r, g, v);

    if (town_scaled || tavern_scaled || island_scaled || labyrinth_scaled || jail_scaled) {
        SDL_RenderSetScale(r->sdl, 1.0f, 1.0f);
    }
    if (tavern_scaled) {
        SDL_RenderSetViewport(r->sdl, NULL);
    }

    draw_low_health_warning(r, g);

    if (g->location == LOCATION_TEMPLE && temple_remaining > 0) {
        int notice_x = r->screen_w - INFO_PANEL_W - 312;
        if (notice_x < 8) {
            notice_x = 8;
        }
        SDL_Rect notice = {notice_x, 8, 304, 40};
        SDL_SetRenderDrawColor(r->sdl, 20, 16, 35, 255);
        SDL_RenderFillRect(r->sdl, &notice);
        SDL_SetRenderDrawColor(r->sdl, 128, 87, 177, 255);
        SDL_RenderDrawRect(r->sdl, &notice);
        char status[48];
        SDL_snprintf(status, sizeof(status), "%s SEALED: %d FOES LEFT",
            g->level == TEMPLE_DEPTH ? "TREASURE" : "STAIRS", temple_remaining);
        renderer_draw_text(r, status, notice_x + 8, 16, (SDL_Color){220, 187, 255, 255}, r->font_tiny);
        renderer_draw_text(r, game_temple_dormant_sentinels(g) > 0 ?
            "Use altar to awaken Moon foes" : "Defeat all Sun and Moon foes",
            notice_x + 8, 32, (SDL_Color){230, 218, 195, 255}, r->font_tiny);
    }

    draw_kraken_status(r, g, v);

    // Draw info panel
    info_panel_draw(r, g);

    // Draw message bar
    message_bar_draw(r, g);

    // Draw minimap overlay in top-left corner of the viewport
    minimap_draw(r, g);
}
