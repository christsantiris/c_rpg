#include "thorn_regent_renderer.h"
#include "../game/thorn_regent.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_thorn_regent_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    draw_moonveil_floor(r, sx, sy, mx, my);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_REGENT_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){155, 217, 168, 255});
        renderer_draw_text(r, "REGENT REWARDS", px - 25, py - 12, (SDL_Color){227, 207, 252, 255}, r->font_tiny);
        return;
    }
    int wall = thorn_regent_wall_turns(tile);
    int cooldown = thorn_regent_node_turns(tile);
    int roots = thorn_regent_root_turns(tile);
    if (wall) {
        draw_world_prop(r, sx, sy, PROP_PLANTING, (SDL_Color){89, 159, 126, 255});
        for (int row = 4; row <= 18; row += 4) {
            SDL_SetRenderDrawColor(r->sdl, 36, 68, 58, 255);
            SDL_RenderDrawLine(r->sdl, px + 2, py + row + 1, px + 21, py + row - 2);
            SDL_SetRenderDrawColor(r->sdl, 150, 181, 135, 255);
            SDL_RenderDrawLine(r->sdl, px + 2, py + row, px + 21, py + row - 3);
            SDL_RenderDrawLine(r->sdl, px + 6, py + row - 1, px + 7, py + row - 4);
            SDL_RenderDrawLine(r->sdl, px + 16, py + row - 2, px + 17, py + row + 1);
        }
    } else if (roots) {
        SDL_SetRenderDrawColor(r->sdl, 117, 151, 111, 255);
        for (int n = 0; n < 3; n++) {
            SDL_RenderDrawLine(r->sdl, px + 3, py + 17 - n * 4, px + 10, py + 14 - n * 4);
            SDL_RenderDrawLine(r->sdl, px + 10, py + 14 - n * 4, px + 20, py + 19 - n * 4);
            SDL_RenderDrawLine(r->sdl, px + 14, py + 17 - n * 4, px + 17, py + 13 - n * 4);
        }
    } else {
        draw_world_prop(r, sx, sy, PROP_FLOWER, cooldown ? (SDL_Color){124, 133, 155, 255} : (SDL_Color){212, 160, 243, 255});
        renderer_draw_text(r, cooldown ? "RESTING NODE" : "A: TEND NODE", px - 21, py - 12, (SDL_Color){203, 224, 215, 255}, r->font_tiny);
    }
    char label[8];
    if (wall || roots || cooldown) {
        snprintf(label, sizeof(label), wall ? "W%d" : roots ? "R%d" : "N%d", wall ? wall : roots ? roots : cooldown);
        renderer_draw_text(r, label, px + 4, py + 3, (SDL_Color){240, 214, 251, 255}, r->font_tiny);
    }
}

void draw_thorn_regent_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MOONVEIL || g->level != MOONVEIL_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_THORN_REGENT || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !thorn_regent_attack_marks(g, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                int growth = e->boss_attack == REGENT_REGROW;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, growth ? 46 : 128, growth ? 158 : 64, growth ? 97 : 167, 120);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, growth ? 165 : 223, growth ? 235 : 173, growth ? 183 : 251, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                char label[8];
                snprintf(label, sizeof(label), growth ? "G%d" : "%d", e->move_timer);
                renderer_draw_text(r, label, px + (growth ? 4 : 9), py + 4, (SDL_Color){246, 233, 252, 255}, r->font_tiny);
                SDL_RenderDrawLine(r->sdl, px + 4, py + 20, px + 20, py + 17);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}
void draw_thorn_regent_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MOONVEIL || g->level != MOONVEIL_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_THORN_REGENT || e->max_hp <= 0 ||
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
        int minimap_width = SWAMP_MAP_W / 2 + 12;
        int top = x < minimap_width ? SWAMP_MAP_H / 2 + 16 : 8;
        SDL_Rect panel = {x, top, width, 78};
        SDL_SetRenderDrawColor(r->sdl, 25, 36, 41, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 164, 142, 211, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, width < 365 ? (e->attack_phase ? "REGENT: PHASE 2" : "REGENT: PHASE 1") :
            (e->attack_phase ? "THORN REGENT: BRIAR CROWN" : "THORN REGENT: GARDEN REGENT"),
            x + 10, top + 8, (SDL_Color){233, 212, 252, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 52, 43, 71, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 128, 187, 153, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == REGENT_LANCE ? "THORN LANCE" : e->boss_attack == REGENT_ROOTS ? "CREEPING ROOTS" : "BRIAR REGROWTH";
            snprintf(hint, sizeof(hint), width < 280 ? "%s: %d%s" : "%s: %d ACTION%s", attack, e->move_timer,
                width < 280 ? (e->frozen_turns > 0 ? " *" : "") : (e->frozen_turns > 0 ? " (FROZEN)" : e->move_timer == 1 ? "" : "S"));
        } else if (e->move_timer < 0) {
            snprintf(hint, sizeof(hint), width < 280 ? "RECOVERY: %d" : "RECOVERY: %d ACTION TO ATTACK", -e->move_timer);
        } else {
            snprintf(hint, sizeof(hint), "PREPARING ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){255, 205, 148, 255}, r->font_tiny);
        snprintf(hint, sizeof(hint), width < 280 ? "A: TEND FLOWER NODES" : "A: TEND FLOWERING NODES TO OPEN A FLANK");
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){158, 226, 229, 255}, r->font_tiny);
    }
}
