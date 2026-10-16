#include "cinder_lord_renderer.h"
#include "../game/cinder_lord.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_cinder_lord_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    draw_ashen_floor(r, sx, sy, mx, my);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_CINDER_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){225, 157, 84, 255});
        renderer_draw_text(r, "CINDER LORD REWARDS", px - 38, py - 12, (SDL_Color){255, 213, 155, 255}, r->font_tiny);
        return;
    }
    int hot = cinder_lord_hot_turns(tile);
    int cool = cinder_lord_cool_turns(tile);
    int timer = cinder_lord_sluice_turns(tile);
    if (hot) {
        draw_world_prop(r, sx, sy, PROP_LOW_FLAME_1, (SDL_Color){255, 161, 69, 255});
        for (int n = 0; n < 3; n++) {
            SDL_SetRenderDrawColor(r->sdl, 219, 99, 35, 255);
            SDL_RenderDrawLine(r->sdl, px + 3, py + 5 + n * 6, px + 10, py + 8 + n * 5);
            SDL_RenderDrawLine(r->sdl, px + 10, py + 8 + n * 5, px + 20, py + 4 + n * 6);
        }
    } else if (tile == TILE_CINDER_CHANNEL || cool) {
        draw_world_prop(r, sx, sy, PROP_CONDUIT, cool ? (SDL_Color){110, 214, 224, 255} : (SDL_Color){164, 132, 93, 255});
        SDL_SetRenderDrawColor(r->sdl, cool ? 117 : 106, cool ? 219 : 78, cool ? 235 : 58, 255);
        SDL_RenderDrawLine(r->sdl, px + 1, py + 14, px + 22, py + 14);
        SDL_RenderDrawLine(r->sdl, px + 1, py + 16, px + 22, py + 16);
    } else {
        draw_world_prop(r, sx, sy, PROP_LEVER, timer ? (SDL_Color){121, 173, 195, 255} : (SDL_Color){215, 148, 80, 255});
        renderer_draw_text(r, timer ? "COOLING SLUICE" : "A: OPEN SLUICE", px - 116, py + 6, (SDL_Color){245, 207, 153, 255}, r->font_tiny);
    }
    char label[8];
    if (hot || cool || timer) {
        snprintf(label, sizeof(label), hot ? "H%d" : cool ? "C%d" : "S%d", hot ? hot : cool ? cool : timer);
        renderer_draw_text(r, label, px + 4, py + 3, (SDL_Color){cool || timer ? 176 : 255, 225, 179, 255}, r->font_tiny);
    }
}

void draw_cinder_lord_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_ASHEN || g->level != ASHEN_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_CINDER_LORD || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                int turns = cinder_lord_attack_turns(g, e, x, y);
                if (!map_is_explored(&g->map, x, y) || !turns) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 208, turns == 1 ? 65 : 110, 32, 120);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 255, turns == 1 ? 172 : 220, 117, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                char label[8];
                snprintf(label, sizeof(label), "%d", turns);
                renderer_draw_text(r, label, px + 9, py + 4, (SDL_Color){255, 238, 191, 255}, r->font_tiny);
                SDL_RenderDrawLine(r->sdl, px + 4, py + 20, px + 12, py + 17);
                SDL_RenderDrawLine(r->sdl, px + 12, py + 17, px + 20, py + 20);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}
void draw_cinder_lord_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_ASHEN || g->level != ASHEN_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_CINDER_LORD || e->max_hp <= 0 ||
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
        SDL_SetRenderDrawColor(r->sdl, 44, 31, 29, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 201, 140, 79, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, width < 365 ? (e->attack_phase ? "CINDER: PHASE 2" : "CINDER: PHASE 1") :
            (e->attack_phase ? "CINDER LORD: BROKEN CRUCIBLE" : "CINDER LORD: FURNACE HEART"),
            x + 10, top + 8, (SDL_Color){255, 218, 163, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 72, 42, 37, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 224, 132, 68, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->boss_attack == CINDER_RING && e->move_timer > 0) {
            const char *format = width < 280 ? (e->move_timer == 2 ? "RING: I1 / O2%s" : "RING: OUTER 1%s") :
                (e->move_timer == 2 ? "RING: INNER 1 / OUTER 2%s" : "RING: OUTER 1%s");
            snprintf(hint, sizeof(hint), format, e->frozen_turns > 0 ? " *" : "");
        } else if (e->move_timer > 0) {
            const char *attack = e->boss_attack == CINDER_LANCE ? "CINDER LANCE" : e->boss_attack == CINDER_ERUPTION ? "LAVA ERUPTION" : "EMBER RING";
            snprintf(hint, sizeof(hint), width < 280 ? "%s: %d%s" : "%s: %d ACTION%s", attack, e->move_timer,
                width < 280 ? (e->frozen_turns > 0 ? " *" : "") : (e->frozen_turns > 0 ? " (FROZEN)" : e->move_timer == 1 ? "" : "S"));
        } else if (e->move_timer < 0) {
            snprintf(hint, sizeof(hint), width < 280 ? "RECOVERY: %d" : "RECOVERY: %d ACTION TO ATTACK", -e->move_timer);
        } else {
            snprintf(hint, sizeof(hint), "PREPARING ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){255, 205, 148, 255}, r->font_tiny);
        snprintf(hint, sizeof(hint), width < 280 ? "A: OPEN FORGE SLUICE" : "A: OPEN A SLUICE FEEDING WARNED LAVA");
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){158, 226, 229, 255}, r->font_tiny);
    }
}
