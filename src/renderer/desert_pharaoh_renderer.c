#include "desert_pharaoh_renderer.h"
#include "../game/desert_pharaoh.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_desert_pharaoh_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    draw_desert_floor(r, sx, sy, mx, my);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_PHARAOH_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){221, 178, 84, 255});
        renderer_draw_text(r, "SANDSTORM STAFF", px - 26, py - 12, (SDL_Color){255, 225, 166, 255}, r->font_tiny);
    } else if (tile == TILE_PHARAOH_OBELISK) {
        // Pointed sandstone shaft: lit left face, dark right face, carved sun and weathered base.
        for (int row = 1; row <= 20; row++) {
            int half = row < 6 ? row / 2 : row < 18 ? 3 : 6;
            SDL_SetRenderDrawColor(r->sdl, 66, 46, 33, 255);
            SDL_RenderDrawLine(r->sdl, px + 12 - half - 1, py + row, px + 12 + half + 1, py + row);
            SDL_SetRenderDrawColor(r->sdl, 229, 194, 125, 255);
            SDL_RenderDrawLine(r->sdl, px + 12 - half, py + row, px + 11, py + row);
            SDL_SetRenderDrawColor(r->sdl, 153, 108, 58, 255);
            SDL_RenderDrawLine(r->sdl, px + 12, py + row, px + 12 + half, py + row);
        }
        SDL_SetRenderDrawColor(r->sdl, 77, 67, 46, 255);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 8, px + 13, py + 8);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 9, px + 14, py + 9);
        SDL_RenderDrawLine(r->sdl, px + 11, py + 10, px + 13, py + 10);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 12, px + 12, py + 16);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 14, px + 14, py + 14);
        SDL_SetRenderDrawColor(r->sdl, 245, 214, 156, 255);
        SDL_RenderDrawLine(r->sdl, px + 7, py + 18, px + 16, py + 18);
        renderer_draw_text(r, "OBELISK", px - 8, py - 12, (SDL_Color){255, 225, 166, 255}, r->font_tiny);
    } else {
        SDL_SetRenderDrawColor(r->sdl, 106, 69, 40, 255);
        for (int row = 6; row <= 20; row += 4) {
            SDL_RenderDrawLine(r->sdl, px + 4, py + row, px + 12, py + row - 2);
            SDL_RenderDrawLine(r->sdl, px + 12, py + row - 2, px + 20, py + row);
        }
        char label[8];
        snprintf(label, sizeof(label), "P%d", desert_pharaoh_sand_turns(tile));
        renderer_draw_text(r, label, px + 4, py + 3, (SDL_Color){255, 235, 176, 255}, r->font_tiny);
    }
}

void draw_desert_pharaoh_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_DESERT_PHARAOH || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                int shadow = desert_pharaoh_shadow(g, e, x, y);
                int marked = desert_pharaoh_attack_marks(g, e, x, y);
                if (!map_is_explored(&g->map, x, y) || (!shadow && !marked)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, shadow ? 24 : 209, shadow ? 76 : 75, shadow ? 90 : 37, 130);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, shadow ? 125 : 255, shadow ? 235 : 196, shadow ? 235 : 97, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                if (e->boss_attack == PHARAOH_STORM) {
                    int dx = e->facing_dx;
                    int dy = e->facing_dy;
                    SDL_RenderDrawLine(r->sdl, px + 12 - dx * 6, py + 18 - dy * 3, px + 12 + dx * 6, py + 18 + dy * 3);
                    SDL_RenderDrawLine(r->sdl, px + 12 + dx * 6, py + 18 + dy * 3, px + 12 + dx * 2 + dy * 2, py + 18 + dy - dx * 2);
                }
                char label[8];
                snprintf(label, sizeof(label), shadow ? "S" : "%d", e->move_timer);
                renderer_draw_text(r, label, px + 9, py + 4, (SDL_Color){255, 242, 184, 255}, r->font_tiny);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_desert_pharaoh_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_DESERT || g->level != DESERT_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_DESERT_PHARAOH || e->max_hp <= 0 ||
            !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
            continue;
        }
        int width = r->screen_w - INFO_PANEL_W - 16;
        if (width > 380) {
            width = 380;
        }
        if (width < 160) {
            return;
        }
        int x = (r->screen_w - INFO_PANEL_W - width) / 2;
        int minimap_width = DESERT_MAP_W / 2 + 12;
        int top = x < minimap_width ? DESERT_MAP_H / 2 + 16 : 8;
        SDL_Rect panel = {x, top, width, 78};
        SDL_SetRenderDrawColor(r->sdl, 49, 34, 26, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 209, 166, 87, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, width < 365 ? (e->attack_phase ? "PHARAOH: PHASE 2" : "PHARAOH: PHASE 1") :
            (e->attack_phase ? "DESERT PHARAOH: SANDBOUND" : "DESERT PHARAOH: SUN-CROWNED"),
            x + 10, top + 8, (SDL_Color){255, 222, 145, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 84, 49, 38, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 227, 171, 83, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == PHARAOH_BEAM ? "CURSE BEAM" : e->boss_attack == PHARAOH_BURIAL ? "BURIAL SAND" : "STORM BAND";
            snprintf(hint, sizeof(hint), width < 280 ? "%s: %d%s" : "%s: %d ACTION%s", attack, e->move_timer,
                width < 280 ? (e->frozen_turns > 0 ? " *" : "") : (e->frozen_turns > 0 ? " (FROZEN)" : e->move_timer == 1 ? "" : "S"));
        } else if (e->move_timer < 0) {
            snprintf(hint, sizeof(hint), width < 280 ? "RECOVERY: %d" : "RECOVERY: %d ACTION TO ATTACK", -e->move_timer);
        } else {
            snprintf(hint, sizeof(hint), "PREPARING ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){255, 205, 148, 255}, r->font_tiny);
        const char *wind = e->facing_dx > 0 ? "E" : e->facing_dx < 0 ? "W" : e->facing_dy > 0 ? "S" : "N";
        if (e->boss_attack == PHARAOH_STORM && e->move_timer > 0) {
            snprintf(hint, sizeof(hint), width < 280 ? "WIND %s / S = SAFE" : "WIND %s: S = SHELTER  /  P = BURIAL SAND", wind);
        } else {
            snprintf(hint, sizeof(hint), width < 280 ? "OBELISKS BLOCK BEAMS" : "OBELISKS BLOCK BEAMS; S SHELTERS FROM STORMS");
        }
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){158, 226, 229, 255}, r->font_tiny);
    }
}
