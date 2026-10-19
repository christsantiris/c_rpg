#include "drowned_queen_renderer.h"
#include "../game/drowned_queen.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_drowned_queen_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_QUEEN_WATER || tile == TILE_QUEEN_ISLAND) {
        draw_coast_shallow_water(r, sx, sy, x, y);
        return;
    }
    draw_coast_channel(r, sx, sy, x, y, 0, 0);
    if (tile == TILE_QUEEN_DRY) {
        return;
    }
    if (tile == TILE_QUEEN_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){183, 210, 185, 255});
        renderer_draw_text(r, "TIDECALLER ROBES", sx * TILE_SIZE - 40, sy * TILE_SIZE - 14,
            (SDL_Color){178, 237, 239, 255}, r->font_tiny);
        return;
    }
    int row = tile == TILE_QUEEN_CONTROL_H || tile == TILE_QUEEN_CONTROL_H_DRAINED;
    int drained = tile == TILE_QUEEN_CONTROL_H_DRAINED || tile == TILE_QUEEN_CONTROL_V_DRAINED;
    draw_world_prop(r, sx, sy, drained ? PROP_GATE_OPEN : PROP_LEVER,
        drained ? (SDL_Color){140, 217, 188, 255} : (SDL_Color){229, 184, 86, 255});
    renderer_draw_text(r, row ? "H" : "V", sx * TILE_SIZE + 8, sy * TILE_SIZE - 12,
        (SDL_Color){185, 224, 233, 255}, r->font_tiny);
}

void draw_drowned_queen_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_COAST || g->level != COAST_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_DROWNED_QUEEN || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !drowned_queen_attack_marks(g, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 62, 148, 191, 100);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 150, 232, 255, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                SDL_RenderDrawLine(r->sdl, px + 3, py + 18, px + 8, py + 16);
                SDL_RenderDrawLine(r->sdl, px + 8, py + 16, px + 14, py + 19);
                SDL_RenderDrawLine(r->sdl, px + 14, py + 19, px + 20, py + 16);
                char label[8];
                snprintf(label, sizeof(label), "%d", e->move_timer);
                renderer_draw_text(r, label, px + 9, py + 4, (SDL_Color){245, 241, 197, 255}, r->font_tiny);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_drowned_queen_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_COAST || g->level != COAST_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_DROWNED_QUEEN || e->max_hp <= 0 ||
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
        if (x < MAP_W * 2 + 12 && r->screen_w - INFO_PANEL_W >= MAP_W * 2 + width + 20) {
            x = MAP_W * 2 + 12;
        }
        int top = x < MAP_W * 2 + 12 ? MAP_H * 2 + 20 : 8;
        SDL_Rect panel = {x, top, width, 78};
        SDL_SetRenderDrawColor(r->sdl, 17, 30, 40, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 79, 175, 191, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        const char *name = width < 280 ? (e->attack_phase ? "QUEEN: PHASE 2" : "QUEEN: PHASE 1") : width < 365 ? (e->attack_phase ? "DROWNED QUEEN: PHASE 2" : "DROWNED QUEEN: PHASE 1") :
            (e->attack_phase ? "DROWNED QUEEN: STORM THRONE" : "DROWNED QUEEN: RISING TIDE");
        renderer_draw_text(r, name, x + 10, top + 8, (SDL_Color){233, 216, 250, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 52, 39, 70, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 93, 190, 206, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == QUEEN_POCKETS ? (width < 280 ? "POOLS" : "FLOODED POCKETS") : e->boss_attack == QUEEN_ROW ? "TIDAL ROW" : "TIDAL COLUMN";
            snprintf(hint, sizeof(hint), "%s: %d TURN%s%s", attack, e->move_timer, e->move_timer == 1 ? "" : "S", e->frozen_turns > 0 ? (width < 280 ? " *" : " (FROZEN)") : "");
        } else {
            snprintf(hint, sizeof(hint), "%s", e->move_timer < 0 ? "RECOVERING: ATTACK NOW" : "PREPARING NEXT ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){e->move_timer < 0 ? 139 : 247, 207, 164, 255}, r->font_tiny);
        int controls = drowned_queen_controls(g);
        if (controls) {
            snprintf(hint, sizeof(hint), width < 280 ? "CONTROLS: %d" : "CONTROLS: %d  INTERACT TO DRAIN", controls);
        } else {
            snprintf(hint, sizeof(hint), "%s", width < 280 ? "PATHS DRAINED" : "PATHS DRAINED: USE DRY STONE");
        }
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){186, 178, 211, 255}, r->font_tiny);
    }
}

