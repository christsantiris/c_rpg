#include "minotaur_renderer.h"
#include "../game/minotaur.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_minotaur_tile(Renderer *r, const GameState *g, int sx, int sy, TileType tile) {
    draw_labyrinth_floor(r, sx, sy, sx, sy);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_MINOTAUR_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){228, 177, 94, 255});
        renderer_draw_text(r, g->minotaur_reward_state == MINOTAUR_REWARD_LEGACY ? "MAGIC SHIELD" : "LABYRINTH AEGIS",
            px - 28, py - 12, (SDL_Color){242, 216, 150, 255}, r->font_tiny);
        return;
    }
    draw_world_prop(r, sx, sy, tile == TILE_MINOTAUR_BROKEN ? PROP_RUBBLE : PROP_PILLAR,
        (SDL_Color){188, 167, 117, 255});
    if (tile == TILE_MINOTAUR_BROKEN) {
        return;
    }
    SDL_SetRenderDrawColor(r->sdl, 225, 192, 116, 255);
    SDL_RenderDrawLine(r->sdl, px + 7, py + 6, px + 17, py + 6);
    SDL_RenderDrawLine(r->sdl, px + 7, py + 8, px + 17, py + 8);
    if (tile == TILE_MINOTAUR_CRACKED) {
        SDL_SetRenderDrawColor(r->sdl, 48, 35, 33, 255);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 5, px + 10, py + 10);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 10, px + 14, py + 13);
        SDL_RenderDrawLine(r->sdl, px + 14, py + 13, px + 11, py + 19);
    }
    renderer_draw_text(r, tile == TILE_MINOTAUR_CRACKED ? "CRACK" : "PILLAR", px - 6, py - 12,
        (SDL_Color){241, 207, 150, 255}, r->font_tiny);
}

void draw_minotaur_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_LABYRINTH || g->level != LABYRINTH_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_MINOTAUR || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !minotaur_attack_marks(g, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 161, 77, 35, 100);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 255, 203, 117, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                if (e->boss_attack == MINOTAUR_CHARGE) {
                    int dx = e->facing_dx;
                    int dy = e->facing_dy;
                    SDL_RenderDrawLine(r->sdl, px + 12 - dx * 6, py + 17 - dy * 3, px + 12 + dx * 6, py + 17 + dy * 3);
                    SDL_RenderDrawLine(r->sdl, px + 12 + dx * 6, py + 17 + dy * 3, px + 12 + dx * 2 - dy * 3, py + 17 + dy - dx * 3);
                } else {
                    SDL_RenderDrawLine(r->sdl, px + 4, py + 20, px + 20, py + 16);
                    SDL_RenderDrawLine(r->sdl, px + 5, py + 21, px + 21, py + 17);
                }
                char label[8];
                snprintf(label, sizeof(label), "%d", e->move_timer);
                renderer_draw_text(r, label, px + 9, py + 4, (SDL_Color){245, 241, 197, 255}, r->font_tiny);
            }
        }
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_MINOTAUR || e->move_timer <= 0 ||
            e->boss_aux_x < 0 || !viewport_is_visible(v, e->boss_aux_x, e->boss_aux_y) ||
            !map_is_explored(&g->map, e->boss_aux_x, e->boss_aux_y)) {
            continue;
        }
        int px = viewport_to_screen_x(v, e->boss_aux_x) * TILE_SIZE;
        int py = viewport_to_screen_y(v, e->boss_aux_y) * TILE_SIZE;
        SDL_Rect outline = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
        SDL_SetRenderDrawColor(r->sdl, 254, 166, 101, 255);
        SDL_RenderDrawRect(r->sdl, &outline);
        TileType tile = g->map.tiles[e->boss_aux_y][e->boss_aux_x];
        const char *label = tile == TILE_MINOTAUR_PILLAR ? "C1" : tile == TILE_MINOTAUR_CRACKED && e->attack_phase ? "B1" : "H1";
        renderer_draw_text(r, label, px + 3, py + 3, (SDL_Color){255, 233, 173, 255}, r->font_tiny);
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_minotaur_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_LABYRINTH || g->level != LABYRINTH_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_MINOTAUR || e->max_hp <= 0 ||
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
        if (x < 116 && r->screen_w - INFO_PANEL_W >= width + 124) {
            x = 116;
        }
        int top = x < 116 ? 66 : 8;
        SDL_Rect panel = {x, top, width, 78};
        SDL_SetRenderDrawColor(r->sdl, 39, 28, 26, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 184, 137, 71, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        const char *name = width < 300 ? (e->attack_phase ? "MINOTAUR: PHASE 2" : "MINOTAUR: PHASE 1") :
            (e->attack_phase ? "MINOTAUR: BLOODIED BULL" : "MINOTAUR: VAULT KEEPER");
        renderer_draw_text(r, name, x + 10, top + 8, (SDL_Color){233, 216, 250, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 52, 39, 70, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 207, 113, 66, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == MINOTAUR_AXE ? "AXE FRONT" : e->boss_attack == MINOTAUR_CHARGE ? "CHARGE" : "WIDE AXE";
            if (width < 280) {
                snprintf(hint, sizeof(hint), "%s: %d%s", attack, e->move_timer, e->frozen_turns > 0 ? " *" : "");
            } else {
                snprintf(hint, sizeof(hint), "%s: %d TURN%s%s", attack, e->move_timer, e->move_timer == 1 ? "" : "S", e->frozen_turns > 0 ? " (FROZEN)" : "");
            }
        } else if (e->move_timer < 0) {
            if (width < 280) {
                snprintf(hint, sizeof(hint), "RECOVERY: %d", -e->move_timer);
            } else {
                snprintf(hint, sizeof(hint), "RECOVERY: %d ACTION%s TO ATTACK", -e->move_timer, e->move_timer == -1 ? "" : "S");
            }
        } else {
            snprintf(hint, sizeof(hint), "%s", width < 280 ? "PREPARING ATTACK" : "PREPARING NEXT ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){e->move_timer < 0 ? 139 : 247, 207, 164, 255}, r->font_tiny);
        snprintf(hint, sizeof(hint), "%s", width < 280 ? "BAIT STONE: STAGGER" : "BAIT STONE: 2 ACTIONS OF STAGGER");
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){186, 178, 211, 255}, r->font_tiny);
    }
}

