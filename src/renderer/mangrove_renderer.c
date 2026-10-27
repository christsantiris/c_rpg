#include "mangrove_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "controls_renderer.h"
#include "../game/mangrove.h"
#include "../game/spice_skiff.h"

static int water(const Map *map, int x, int y) {
    return x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || map->tiles[y][x] == TILE_MANGROVE_WATER;
}

void mangrove_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (g->location == LOCATION_MANGROVE_PATH && tile == TILE_MANGROVE_ROOT) {
        if (y < MANGROVE_GATE_Y) {
            draw_town_floor(r, sx, sy);
            return;
        }
        tile = TILE_MANGROVE_WATER;
    }
    renderer_draw_tile_bg(r, sx, sy, (SDL_Color){9, 42, 52, 255});
    if (tile == TILE_MANGROVE_WATER) {
        int phase = (SDL_GetTicks() / 190 + x * 3 + y * 7) % 8;
        SDL_SetRenderDrawColor(r->sdl, 24 + phase, 66 + phase * 2, 77 + phase * 2, 255);
        SDL_RenderDrawLine(r->sdl, px + 3 + phase, py + 6, px + 7 + phase, py + 6);
        SDL_RenderDrawLine(r->sdl, px + 11 - phase / 2, py + 18, px + 17 - phase / 2, py + 18);
        return;
    }
    int north = water(&g->map, x, y - 1);
    int south = water(&g->map, x, y + 1);
    int west = water(&g->map, x - 1, y);
    int east = water(&g->map, x + 1, y);
    for (int row = 0; row < TILE_SIZE; row++) {
        int edge = row < 5 && north ? 5 - row : row > 18 && south ? row - 18 : 0;
        int left = west ? edge : 0;
        int right = east ? edge : 0;
        SDL_SetRenderDrawColor(r->sdl, 115 + row % 3, 111 + row % 3, 73 + row % 3, 255);
        SDL_RenderDrawLine(r->sdl, px + left, py + row, px + 23 - right, py + row);
    }
    SDL_SetRenderDrawColor(r->sdl, 90, 94, 57, 255);
    int offset = (x * 7 + y * 11) % 9;
    SDL_RenderDrawLine(r->sdl, px + 4 + offset, py + 8, px + 7 + offset, py + 8);
    SDL_RenderDrawPoint(r->sdl, px + 9, py + 18);
    if ((x * 11 + y * 7) % 3 == 0) {
        SDL_SetRenderDrawColor(r->sdl, 67, 96, 45, 255);
        for (int blade = 0; blade < 4; blade++) {
            SDL_RenderDrawLine(r->sdl, px + 7 + blade * 2, py + 16, px + 6 + blade * 3, py + 11 + blade % 3);
        }
    }
    if (tile == TILE_MANGROVE_DECK) {
        SDL_SetRenderDrawColor(r->sdl, 90, 67, 42, 255);
        SDL_Rect deck = {px, py, TILE_SIZE, TILE_SIZE};
        SDL_RenderFillRect(r->sdl, &deck);
        for (int row = 0; row < TILE_SIZE; row += 6) {
            SDL_SetRenderDrawColor(r->sdl, 135, 102, 59, 255);
            SDL_RenderDrawLine(r->sdl, px, py + row, px + 23, py + row);
            SDL_SetRenderDrawColor(r->sdl, 45, 39, 29, 255);
            SDL_RenderDrawLine(r->sdl, px, py + row + 5, px + 23, py + row + 5);
        }
    }
    if (tile == TILE_MANGROVE_ROOT) {
        if (g->location == LOCATION_MANGROVES) {
            SDL_Rect dest = {px - 12, py - 48, 48, 72};
            SDL_RenderCopy(r->sdl, r->mangrove_root_texture, NULL, &dest);
        }
    } else if (tile == TILE_MANGROVE_LANDING) {
        for (int row = 2; row < 23; row += 5) {
            SDL_SetRenderDrawColor(r->sdl, 64, 48, 33, 255);
            SDL_Rect plank = {px + 1, py + row, 22, 4};
            SDL_RenderFillRect(r->sdl, &plank);
            SDL_SetRenderDrawColor(r->sdl, 163, 125, 73, 255);
            SDL_RenderDrawLine(r->sdl, px + 2, py + row, px + 21, py + row);
        }
    } else if (tile == TILE_MANGROVE_CACHE) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){169, 137, 73, 255});
    } else if (tile == TILE_MANGROVE_RETURN || tile == TILE_MANGROVE_EXIT) {
        const char *label = tile == TILE_MANGROVE_RETURN ? (g->level == 1 ? "TO DAWNMERE" : "BACK") :
            g->level == MANGROVE_DEPTH ? (g->mangrove_defeated || g->debug_passages_unlocked ? "RETURN" : "SEALED") : "ONWARD";
        for (int row = 0; row < TILE_SIZE; row += 6) {
            SDL_SetRenderDrawColor(r->sdl, 133, 102, 61, 255);
            SDL_Rect plank = {px, py + row, TILE_SIZE, 5};
            SDL_RenderFillRect(r->sdl, &plank);
        }
        int width;
        TTF_SizeText(r->font_tiny, label, &width, NULL);
        int lx = px + 12 - width / 2;
        if (x == 0) {
            lx = px + 4;
        } else if (x == MANGROVE_W - 1) {
            lx = px + 20 - width;
        }
        renderer_draw_text(r, label, lx, py + (y == 0 ? 2 : -12), (SDL_Color){223, 211, 140, 255}, r->font_tiny);
    }
}

void mangrove_draw_landmark(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MANGROVE_PATH) {
        return;
    }
    int x = viewport_to_screen_x(v, MANGROVE_GATE_X) * TILE_SIZE;
    int y = viewport_to_screen_y(v, MANGROVE_GATE_Y) * TILE_SIZE;
    SDL_Rect dest = {x - 3 * TILE_SIZE, y - 5 * TILE_SIZE, 168, 144};
    SDL_RenderCopy(r->sdl, r->mangrove_entrance_texture, NULL, &dest);
    renderer_draw_text(r, "SALTWIND MARSH", x - 85, y - 6 * TILE_SIZE, (SDL_Color){163, 207, 105, 255}, r->font_tiny);
}

static void quest_label(Renderer *r, const Viewport *v, const char *text, int x, int y) {
    int width;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE - 14;
    renderer_draw_text(r, text, px + 1, py + 1, (SDL_Color){28, 34, 23, 255}, r->font_tiny);
    renderer_draw_text(r, text, px, py, (SDL_Color){235, 211, 139, 255}, r->font_tiny);
}

void mangrove_draw_spice_quest(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MANGROVES || !r->spice_skiff_texture) {
        return;
    }
    if (g->level == 2 && viewport_is_visible(v, SPICE_WRECK_X, SPICE_WRECK_Y)) {
        SDL_Rect src = {0, 0, 36, 36};
        SDL_Rect dst = {viewport_to_screen_x(v, SPICE_WRECK_X) * TILE_SIZE - 6,
            viewport_to_screen_y(v, SPICE_WRECK_Y) * TILE_SIZE - 6, 36, 36};
        SDL_RenderCopy(r->sdl, r->spice_skiff_texture, &src, &dst);
        quest_label(r, v, "STRANDED SPICE SKIFF", SPICE_WRECK_X, SPICE_WRECK_Y - 1);
    } else if (g->level == 3 && viewport_is_visible(v, SPICE_CASKET_X, SPICE_CASKET_Y)) {
        int px = viewport_to_screen_x(v, SPICE_CASKET_X) * TILE_SIZE;
        int py = viewport_to_screen_y(v, SPICE_CASKET_Y) * TILE_SIZE;
        SDL_Rect arch_src = {84, 0, 72, 72};
        SDL_Rect arch_dst = {px - 24, py - 120, 72, 72};
        SDL_RenderCopy(r->sdl, r->spice_skiff_texture, &arch_src, &arch_dst);
        int empty = !!(g->spice_skiff_progress & SPICE_CARGO);
        SDL_Rect src = {empty ? 60 : 36, 0, 24, 24};
        SDL_Rect dst = {px, py, 24, 24};
        SDL_RenderCopy(r->sdl, r->spice_skiff_texture, &src, &dst);
        quest_label(r, v, "ARCHED MANGROVE ROOT", SPICE_CASKET_X, SPICE_CASKET_Y - 6);
        quest_label(r, v, empty ? "CARGO RECOVERED" : "SPICE CASKET", SPICE_CASKET_X, SPICE_CASKET_Y);
    }
}

void mangrove_draw_boat(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MANGROVES || !g->mangroves.initialized ||
        !map_is_explored(&g->map, g->mangroves.boat_x, g->mangroves.boat_y)) {
        return;
    }
    int direction = g->mangroves.boat_dy < 0 ? 0 : g->mangroves.boat_dx > 0 ? 1 : g->mangroves.boat_dy > 0 ? 2 : 3;
    SDL_Rect source = {direction * 36, 0, 36, 36};
    SDL_Rect dest = {viewport_to_screen_x(v, g->mangroves.boat_x) * TILE_SIZE - 6,
        viewport_to_screen_y(v, g->mangroves.boat_y) * TILE_SIZE - 6, 36, 36};
    SDL_RenderCopy(r->sdl, r->skiff_texture, &source, &dest);
}

void mangrove_draw_hint(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_MANGROVES) {
        return;
    }
    const char *hint = mangrove_hint(g);
    char text[180];
    SDL_snprintf(text, sizeof(text), "%s | %s | %s", mangrove_stage_name(g->level),
        g->mangroves.aboard ? "PADDLING" : "ASHORE", hint ? hint : "FIND TIMBER LANDINGS ALONG THE CHANNELS");
    if (hint) {
        SDL_snprintf(text, sizeof(text), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), hint);
    }
    renderer_draw_text(r, text, 12, r->tiles_y * TILE_SIZE - 24, (SDL_Color){211, 222, 172, 255}, r->font_tiny);
}

void mangrove_draw_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MANGROVES || g->mangroves.aboard) {
        return;
    }
    for (int y = 0; y < MANGROVE_H; y++) {
        for (int x = 0; x < MANGROVE_W; x++) {
            if (!viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y) || !map_is_walkable(&g->map, x, y)) {
                continue;
            }
            for (int i = 0; i < g->enemy_count; i++) {
                if (mangrove_attack_marks(&g->enemies[i], x, y)) {
                    SDL_Rect warning = {viewport_to_screen_x(v, x) * TILE_SIZE + 2, viewport_to_screen_y(v, y) * TILE_SIZE + 2, 20, 20};
                    SDL_SetRenderDrawColor(r->sdl, 225, 157, 62, 255);
                    SDL_RenderDrawRect(r->sdl, &warning);
                    break;
                }
            }
        }
    }
}
