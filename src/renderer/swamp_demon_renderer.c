#include "swamp_demon_renderer.h"
#include "../game/swamp_demon.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_swamp_demon_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    draw_swamp_floor(r, sx, sy, x, y);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_DEMON_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){156, 190, 91, 255});
        renderer_draw_text(r, "DEMONIC SWORD", px - 28, py - 14, (SDL_Color){229, 211, 143, 255}, r->font_tiny);
        return;
    }
    int rot = swamp_demon_rot_turns(tile);
    if (rot) {
        SDL_SetRenderDrawColor(r->sdl, 84, 107, 39, 255);
        for (int row = 0; row < 4; row++) {
            SDL_Rect ripple = {px + 4 + row % 2, py + 6 + row * 3, 16 - row * 2, 2};
            SDL_RenderFillRect(r->sdl, &ripple);
        }
        SDL_SetRenderDrawColor(r->sdl, 157, 182, 66, 255);
        SDL_RenderDrawLine(r->sdl, px + 3, py + 9, px + 6, py + 6);
        SDL_RenderDrawLine(r->sdl, px + 17, py + 19, px + 21, py + 15);
        return;
    }
    int cooldown = swamp_demon_vent_turns(tile);
    draw_world_prop(r, sx, sy, PROP_INTAKE, cooldown ? (SDL_Color){134, 136, 110, 255} : (SDL_Color){163, 218, 127, 255});
    SDL_SetRenderDrawColor(r->sdl, 105, 133, 70, 255);
    SDL_RenderDrawLine(r->sdl, px + 1, py + 21, px + 7, py + 16);
    SDL_RenderDrawLine(r->sdl, px + 7, py + 16, px + 12, py + 19);
    SDL_RenderDrawLine(r->sdl, px + 16, py + 16, px + 22, py + 22);
    char label[12];
    if (cooldown) {
        snprintf(label, sizeof(label), "VENT %d", cooldown);
    } else {
        snprintf(label, sizeof(label), "VENT");
    }
    renderer_draw_text(r, label, px - 4, py - 12, (SDL_Color){190, 229, 151, 255}, r->font_tiny);
}

void draw_swamp_demon_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_SWAMP || g->level != SWAMP_BOSS_LEVEL) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_SWAMP_DEMON || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !swamp_demon_attack_marks(g, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 118, 124, 38, 100);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 224, 192, 94, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                if (e->boss_attack == DEMON_CLEAVE) {
                    SDL_RenderDrawLine(r->sdl, px + 5, py + 20, px + 18, py + 15);
                    SDL_RenderDrawLine(r->sdl, px + 7, py + 21, px + 20, py + 16);
                } else if (e->boss_attack == DEMON_POOL) {
                    SDL_Rect bubble = {px + 5, py + 16, 4, 4};
                    SDL_RenderDrawRect(r->sdl, &bubble);
                    bubble.x += 9;
                    bubble.y -= 2;
                    SDL_RenderDrawRect(r->sdl, &bubble);
                } else {
                    SDL_RenderDrawLine(r->sdl, px + 12, py + 21, px + 4, py + 15);
                    SDL_RenderDrawLine(r->sdl, px + 12, py + 21, px + 12, py + 15);
                    SDL_RenderDrawLine(r->sdl, px + 12, py + 21, px + 20, py + 15);
                }
                char label[8];
                snprintf(label, sizeof(label), "%d", e->move_timer);
                renderer_draw_text(r, label, px + 9, py + 4, (SDL_Color){245, 241, 197, 255}, r->font_tiny);
            }
        }
    }
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H || !map_is_explored(&g->map, x, y)) {
                continue;
            }
            TileType tile = g->map.tiles[y][x];
            if (tile == TILE_ITEM) {
                for (int i = 0; i < g->floor_item_count; i++) {
                    if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                        tile = g->floor_items[i].underlying_tile;
                        break;
                    }
                }
            }
            int turns = swamp_demon_rot_turns(tile);
            if (turns) {
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect border = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 152, 178, 61, 255);
                SDL_RenderDrawRect(r->sdl, &border);
                char label[8];
                snprintf(label, sizeof(label), "R%d", turns);
                renderer_draw_text(r, label, px + 3, py + 2, (SDL_Color){248, 224, 132, 255}, r->font_tiny);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_swamp_demon_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_SWAMP || g->level != SWAMP_BOSS_LEVEL) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_SWAMP_DEMON || e->max_hp <= 0 ||
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
        SDL_SetRenderDrawColor(r->sdl, 23, 33, 24, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 112, 162, 82, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        const char *name = width < 365 ? (e->attack_phase ? "DEMON: PHASE 2" : "DEMON: PHASE 1") :
            (e->attack_phase ? "SWAMP DEMON: UNSHACKLED" : "SWAMP DEMON: BOG TYRANT");
        renderer_draw_text(r, name, x + 10, top + 8, (SDL_Color){233, 216, 250, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 52, 39, 70, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 160, 179, 79, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == DEMON_POOL ? "ROT POOLS" : e->boss_attack == DEMON_CLEAVE ? "CLEAVE" : "MIASMA FAN";
            if (width < 280) {
                snprintf(hint, sizeof(hint), "%s: %d%s", attack, e->move_timer, e->frozen_turns > 0 ? " *" : "");
            } else {
                snprintf(hint, sizeof(hint), "%s: %d TURN%s%s", attack, e->move_timer, e->move_timer == 1 ? "" : "S", e->frozen_turns > 0 ? " (FROZEN)" : "");
            }
        } else {
            snprintf(hint, sizeof(hint), "%s", e->move_timer < 0 ? (width < 280 ? "RECOVERY: ATTACK" : "RECOVERING: ATTACK NOW") :
                (width < 280 ? "PREPARING ATTACK" : "PREPARING NEXT ATTACK"));
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){e->move_timer < 0 ? 139 : 247, 207, 164, 255}, r->font_tiny);
        snprintf(hint, sizeof(hint), "%s", width < 280 ? "VENTS DRAIN ROT" : "INTERACT: VENT DRAINS ROT / EXPOSES BOSS");
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){186, 178, 211, 255}, r->font_tiny);
    }
}

