#include "crownroad_north_renderer.h"
#include "../game/crownroad_north.h"
#include "sprites.h"
#include "world_props.h"
#include "controls_renderer.h"
#include "info_panel.h"
#include <stdlib.h>

static void box(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &rect);
}

void northroad_draw_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_FOREST_WALL) {
        draw_elderwood_wall(r, sx, sy, x, y);
        return;
    }
    if (tile == TILE_TOWN_PATH || tile == TILE_CROWN_EXIT) {
        draw_town_path(r, sx, sy);
    } else {
        draw_elderwood_floor(r, sx, sy, x, y);
    }
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_CROWN_BARRICADE) {
        for (int stake = 0; stake < 4; stake++) {
            int xx = px + 2 + stake * 6;
            int top = py + 4 + (x + stake) % 3;
            box(r, xx - 1, top, 5, 19 - (top - py), (SDL_Color){28, 24, 20, 255});
            box(r, xx, top, 3, 17 - (top - py), (SDL_Color){113, 80, 44, 255});
            box(r, xx, top + 2, 1, 11, (SDL_Color){164, 119, 64, 255});
            box(r, xx + 2, top + 4, 1, 9, (SDL_Color){66, 48, 30, 255});
        }
        box(r, px, py + 12, 24, 4, (SDL_Color){51, 37, 25, 255});
        box(r, px, py + 12, 24, 1, (SDL_Color){147, 106, 57, 255});
        box(r, px + 4, py + 13, 1, 2, (SDL_Color){173, 171, 146, 255});
        box(r, px + 19, py + 13, 1, 2, (SDL_Color){173, 171, 146, 255});
    } else if (tile == TILE_CROWN_SUPPLIES) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){162, 123, 64, 255});
    } else if (tile == TILE_CROWN_EMPTY) {
        draw_world_prop(r, sx, sy, PROP_STRAW, (SDL_Color){115, 106, 64, 255});
    } else if (tile == TILE_CROWN_EXIT) {
        // The paving meets the map boundary; gateposts sit beside the road.
        box(r, px - 3, py, 4, 24, (SDL_Color){86, 65, 42, 255});
        box(r, px + 23, py, 4, 24, (SDL_Color){86, 65, 42, 255});
        SDL_SetRenderDrawColor(r->sdl, 218, 205, 156, 255);
        for (int n = 0; n < 2; n++) {
            int yy = py + 7 + n * 8;
            int direction = y == 0 ? -1 : 1;
            SDL_RenderDrawLine(r->sdl, px + 7, yy, px + 12, yy + direction * 4);
            SDL_RenderDrawLine(r->sdl, px + 12, yy + direction * 4, px + 17, yy);
        }
    }
}

void northroad_draw_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CROWNROAD_NORTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        if (enemy->attack_phase && enemy->active) {
            for (int y = enemy->attack_target_y - 1; y <= enemy->attack_target_y + 1; y++) {
                for (int x = enemy->attack_target_x - 1; x <= enemy->attack_target_x + 1; x++) {
                    if (!viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y) || !northroad_attack_marks(g, enemy, x, y)) {
                        continue;
                    }
                    SDL_Rect cell = {viewport_to_screen_x(v, x) * TILE_SIZE + 1, viewport_to_screen_y(v, y) * TILE_SIZE + 1, 22, 22};
                    box(r, cell.x, cell.y, 22, 22, (SDL_Color){182, 70, 33, 95});
                    SDL_SetRenderDrawColor(r->sdl, 246, 175, 91, 255);
                    SDL_RenderDrawRect(r->sdl, &cell);
                    char timer[8];
                    snprintf(timer, sizeof(timer), "%d", enemy->move_timer);
                    renderer_draw_text(r, timer, cell.x + 8, cell.y + 5, (SDL_Color){255, 224, 154, 255}, r->font_tiny);
                }
            }
        }
        if (viewport_is_visible(v, enemy->x, enemy->y) && map_is_explored(&g->map, enemy->x, enemy->y) && northroad_shielded(g, enemy)) {
            int px = viewport_to_screen_x(v, enemy->x) * TILE_SIZE + 12 + enemy->facing_dx * 10;
            int py = viewport_to_screen_y(v, enemy->y) * TILE_SIZE + 12 + enemy->facing_dy * 10;
            box(r, px - (enemy->facing_dy ? 5 : 1), py - (enemy->facing_dx ? 5 : 1),
                enemy->facing_dy ? 11 : 3, enemy->facing_dx ? 11 : 3, (SDL_Color){136, 190, 196, 255});
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
    if (viewport_is_visible(v, CROWN_NORTH_X, 2)) {
        draw_town_road_sign(r, viewport_to_screen_x(v, CROWN_NORTH_X - 3), viewport_to_screen_y(v, 2), "CASTLE GROUNDS");
    }
    if (viewport_is_visible(v, CROWN_NORTH_X, CROWN_NORTH_H - 3)) {
        draw_town_road_sign(r, viewport_to_screen_x(v, CROWN_NORTH_X - 4), viewport_to_screen_y(v, CROWN_NORTH_H - 3), "ROYAL CATACOMBS");
    }
}

void northroad_draw_hint(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_CROWNROAD_NORTH || !northroad_has_interaction(g)) {
        return;
    }
    char text[128];
    snprintf(text, sizeof(text), "%s: SEARCH PATROL SUPPLIES", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]));
    int width = 0;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    if (width > r->screen_w - INFO_PANEL_W - 170) {
        width = r->screen_w - INFO_PANEL_W - 170;
    }
    box(r, 146, 4, width + 16, 22, (SDL_Color){17, 29, 21, 255});
    renderer_draw_text(r, text, 154, 9, (SDL_Color){220, 200, 144, 255}, r->font_tiny);
}
