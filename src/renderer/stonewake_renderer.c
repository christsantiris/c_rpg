#include "stonewake_renderer.h"
#include "../game/stonewake.h"
#include "sprites.h"
#include "world_props.h"
#include "controls_renderer.h"
#include <stdlib.h>
#include <stdio.h>

static void prop(Renderer *r, int sx, int sy, int index) {
    if (r->stonewake_prop_texture) {
        SDL_Rect src = {index * 24, 0, 24, 24};
        SDL_Rect dest = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopy(r->sdl, r->stonewake_prop_texture, &src, &dest);
    }
}

void draw_stonewake_machinery(Renderer *r, int sx, int sy, int repaired) {
    prop(r, sx, sy, repaired ? 1 : 0);
}

void draw_stonewake_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    if (tile == TILE_STONEWAKE_GAP) {
        draw_glassdeep_pool(r, sx, sy, mx, my);
        return;
    }
    if (tile == TILE_STONEWAKE_BOUNDARY || tile == TILE_STONEWAKE_LEGACY_BOUNDARY) {
        draw_glassdeep_wall(r, sx, sy, mx, my);
        return;
    }
    draw_glassdeep_floor(r, sx, sy, mx, my);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_STONEWAKE_LEDGE) {
        SDL_Rect edge = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
        SDL_SetRenderDrawColor(r->sdl, 199, 188, 154, 255);
        SDL_RenderDrawRect(r->sdl, &edge);
    } else if (tile == TILE_STONEWAKE_REMOTE) {
        draw_world_prop(r, sx, sy, PROP_LEVER, (SDL_Color){194, 171, 107, 255});
    } else if (tile == TILE_STONEWAKE_BRIDGE) {
        draw_glassdeep_pool(r, sx, sy, mx, my);
        for (int row = 3; row < 22; row += 4) {
            SDL_Rect plank = {px, py + row, TILE_SIZE, 3};
            SDL_SetRenderDrawColor(r->sdl, 130, 94, 54, 255);
            SDL_RenderFillRect(r->sdl, &plank);
            SDL_SetRenderDrawColor(r->sdl, 183, 143, 87, 255);
            SDL_RenderDrawLine(r->sdl, px, py + row, px + TILE_SIZE - 1, py + row);
        }
        SDL_SetRenderDrawColor(r->sdl, 68, 46, 30, 255);
        SDL_RenderDrawLine(r->sdl, px, py + 2, px + TILE_SIZE - 1, py + 2);
        SDL_RenderDrawLine(r->sdl, px, py + 22, px + TILE_SIZE - 1, py + 22);
    } else if (tile == TILE_STONEWAKE_SUPPLIES || tile == TILE_STONEWAKE_EMPTY) {
        prop(r, sx, sy, tile == TILE_STONEWAKE_SUPPLIES ? 2 : 3);
    }
}

void draw_stonewake_props(Renderer *r, const GameState *g, const Viewport *v) {
    if (!stonewake_map(&g->map) || g->location != LOCATION_GLASSDEEP) {
        return;
    }
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            TileType tile = g->map.tiles[y][x];
            if (tile == TILE_ITEM) {
                for (int n = 0; n < g->floor_item_count; n++) {
                    if (g->floor_items[n].active && g->floor_items[n].x == x && g->floor_items[n].y == y) {
                        tile = g->floor_items[n].underlying_tile;
                    }
                }
            } else if (tile == TILE_PORTAL && g->portal_active && g->portal_x == x && g->portal_y == y && g->portal_location == g->location && g->portal_level == g->level) {
                tile = g->portal_origin_tile;
            }
            if (tile == TILE_STONEWAKE_WORKSHOP && x > 0 && x + 1 < SWAMP_MAP_W && y + 1 < SWAMP_MAP_H &&
                g->map.tiles[y][x - 1] == tile && g->map.tiles[y][x + 1] == tile && g->map.tiles[y + 1][x] != tile &&
                map_is_explored(&g->map, x, y) && r->stonewake_workshop_texture) {
                SDL_Rect dest = {viewport_to_screen_x(v, x) * TILE_SIZE - 36, viewport_to_screen_y(v, y) * TILE_SIZE - 48, 96, 72};
                SDL_RenderCopy(r->sdl, r->stonewake_workshop_texture, NULL, &dest);
            }
            if (!stonewake_crane_tile(tile) || (!map_is_explored(&g->map, x, y) && !map_is_explored(&g->map, x + 10, y))) {
                continue;
            }
            int px = viewport_to_screen_x(v, x) * TILE_SIZE;
            int py = viewport_to_screen_y(v, y) * TILE_SIZE;
            int mode = stonewake_crane_mode(tile);
            int crane_seen = map_is_explored(&g->map, x, y);
            if (crane_seen && r->stonewake_crane_texture) {
                SDL_Rect source = {mode * 72, 0, 72, 72};
                SDL_Rect dest = {px - 18, py - 48, 72, 72};
                SDL_RenderCopy(r->sdl, r->stonewake_crane_texture, &source, &dest);
            }
            SDL_Color text_color = {218, 214, 169, 255};
            const char *status = mode == 1 ? "CROSSING" : mode == 2 ? "SALVAGE" : "RAISED";
            if (crane_seen) {
                renderer_draw_text(r, status, px - 12, py - 58, text_color, r->font_tiny);
            }
            int near = abs(g->player.x - x) + abs(g->player.y - y) <= 1;
            int remote = abs(g->player.x - x - 10) + abs(g->player.y - y) <= 1;
            if (near || remote) {
                const char *next = mode == 0 ? "LOWER CROSSING" : mode == 1 && stonewake_has_salvage(g, x, y) ? "SALVAGE ROUTE" : "RAISE GANGWAYS";
                char text[64];
                snprintf(text, sizeof(text), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), next);
                renderer_draw_text(r, text, px + (remote ? 10 * TILE_SIZE : 0) - 40, py - 74, text_color, r->font_tiny);
            }

        }
    }
}

int draw_stonewake_enemy(Renderer *r, int sx, int sy, EnemyType type) {
    int index;
    switch (type) {
        case ENEMY_BLIND_STALKER: index = 1; break;
        case ENEMY_SHARD_GOLEM: index = 2; break;
        case ENEMY_PRISM_SOVEREIGN: index = 4; break;
        default: return 0;
    }
    if (!r->stonewake_enemy_texture) {
        return 0;
    }
    SDL_Rect source = {index * 48, 0, 48, 48};
    SDL_Rect dest = {sx * TILE_SIZE - 12, sy * TILE_SIZE - 24, 48, 48};
    SDL_RenderCopy(r->sdl, r->stonewake_enemy_texture, &source, &dest);
    return 1;
}
