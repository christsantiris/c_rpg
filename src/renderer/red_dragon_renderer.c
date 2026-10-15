#include "red_dragon_renderer.h"
#include "../game/red_dragon.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_red_dragon_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    TileType base = red_dragon_base_tile(tile);
    int terrain = base == TILE_DRAGON_FLOOR ? 0 : base == TILE_DRAGON_HOARD ? 2 : 1;
    draw_dragonspine_floor(r, sx, sy, mx, my, terrain);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile >= TILE_DRAGON_RELIQUARY_FLOOR && tile <= TILE_DRAGON_RELIQUARY_HOARD) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){228, 170, 87, 255});
        renderer_draw_text(r, "DRAGON SCALE MANTLE", px - 48, py - 12, (SDL_Color){255, 225, 164, 255}, r->font_tiny);
    } else if (tile == TILE_DRAGON_COLUMN_RUBBLE) {
        draw_world_prop(r, sx, sy, PROP_RUBBLE, (SDL_Color){102, 101, 108, 255});
    } else if (tile >= TILE_DRAGON_BASALT && tile <= TILE_DRAGON_CRACKED_COLUMN) {
        draw_world_prop(r, sx, sy, PROP_PILLAR, (SDL_Color){106, 104, 115, 255});
        SDL_SetRenderDrawColor(r->sdl, tile == TILE_DRAGON_BASALT ? 141 : 243, tile == TILE_DRAGON_BASALT ? 203 : 169, 127, 255);
        SDL_RenderDrawLine(r->sdl, px + 5, py + 3, px + 18, py + 3);
        if (tile == TILE_DRAGON_CRACKED_COLUMN) {
            SDL_SetRenderDrawColor(r->sdl, 251, 140, 72, 255);
            SDL_RenderDrawLine(r->sdl, px + 12, py + 4, px + 9, py + 10);
            SDL_RenderDrawLine(r->sdl, px + 9, py + 10, px + 14, py + 14);
            SDL_RenderDrawLine(r->sdl, px + 14, py + 14, px + 10, py + 21);
        }
        renderer_draw_text(r, tile == TILE_DRAGON_BASALT ? "S" : tile == TILE_DRAGON_CRACKED_COLUMN ? "!" : "W",
            px + 2, py + 15, (SDL_Color){255, 230, 185, 255}, r->font_tiny);
    } else if (red_dragon_hot_turns(tile)) {
        draw_world_prop(r, sx, sy, PROP_LOW_FLAME_1, (SDL_Color){255, 142, 52, 255});
        SDL_SetRenderDrawColor(r->sdl, 241, 112, 43, 255);
        SDL_RenderDrawLine(r->sdl, px + 2, py + 21, px + 10, py + 18);
        SDL_RenderDrawLine(r->sdl, px + 10, py + 18, px + 21, py + 22);
    }
}

void draw_red_dragon_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_DRAGONSPINE || g->level != DRAGONSPINE_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
            if (!map_is_explored(&g->map, x, y)) {
                continue;
            }
            int px = viewport_to_screen_x(v, x) * TILE_SIZE;
            int py = viewport_to_screen_y(v, y) * TILE_SIZE;
            TileType tile = g->map.tiles[y][x];
            if (tile == TILE_ITEM) {
                for (int i = 0; i < g->floor_item_count; i++) {
                    const FloorItem *f = &g->floor_items[i];
                    if (f->active && f->x == x && f->y == y) {
                        tile = (TileType)f->underlying_tile;
                        break;
                    }
                }
            }
            char text[8];
            if (red_dragon_hot_turns(tile)) {
                snprintf(text, sizeof(text), "H%d", red_dragon_hot_turns(tile));
                renderer_draw_text(r, text, px + 3, py + 3, (SDL_Color){255, 232, 176, 255}, r->font_tiny);
            }
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *e = &g->enemies[i];
                if (!red_dragon_attack_marks(g, e, x, y)) {
                    continue;
                }
                int landing = e->boss_attack == DRAGON_WINGBEAT;
                int stone = tile >= TILE_DRAGON_BASALT && tile <= TILE_DRAGON_CRACKED_COLUMN;
                SDL_Rect cell = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, landing ? 122 : 227, landing ? 115 : 79, 51, 115);
                SDL_RenderFillRect(r->sdl, &cell);
                SDL_SetRenderDrawColor(r->sdl, 255, landing ? 227 : 185, 124, 255);
                SDL_RenderDrawRect(r->sdl, &cell);
                snprintf(text, sizeof(text), stone ? "C%d" : "%d", e->move_timer);
                renderer_draw_text(r, text, px + (stone ? 3 : 9), py + 4, (SDL_Color){255, 244, 204, 255}, r->font_tiny);
                SDL_RenderDrawLine(r->sdl, px + 4, py + 20, px + 12, py + 16);
                SDL_RenderDrawLine(r->sdl, px + 12, py + 16, px + 20, py + 20);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_red_dragon_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_DRAGONSPINE || g->level != DRAGONSPINE_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_RED_DRAGON || e->max_hp <= 0 ||
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
        int top = x < SWAMP_MAP_W / 2 + 12 ? SWAMP_MAP_H / 2 + 16 : 8;
        SDL_Rect panel = {x, top, width, 78};
        SDL_SetRenderDrawColor(r->sdl, 43, 27, 26, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 209, 130, 87, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        static const char *phases[] = {"HOARD DEFENDER", "SCORCHED WINGS", "LAST INFERNO"};
        int phase = e->attack_phase >= 0 && e->attack_phase <= 2 ? e->attack_phase : 0;
        char text[80];
        if (width < 300) {
            snprintf(text, sizeof(text), "DRAGON: PHASE %d", phase + 1);
        } else {
            snprintf(text, sizeof(text), "RED DRAGON: %s", phases[phase]);
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){255, 220, 171, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 75, 42, 39, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 230, 119, 65, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        if (e->move_timer > 0) {
            static const char *attacks[] = {"PREPARING", "BREATH", "CLAW/TAIL", "WINGBEAT", "INFERNO I", "INFERNO II"};
            int attack = e->boss_attack >= DRAGON_BREATH && e->boss_attack <= DRAGON_INFERNO_SECOND ? e->boss_attack : 0;
            snprintf(text, sizeof(text), "%s: %d%s", attacks[attack], e->move_timer, e->frozen_turns > 0 ? " *" : "");
        } else {
            snprintf(text, sizeof(text), e->move_timer < 0 ? "RECOVERY: %d" : "PREPARING ATTACK", -e->move_timer);
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){255, 203, 143, 255}, r->font_tiny);
        renderer_draw_text(r, width < 300 ? "S: COVER  W/!: WEAK" : "BASALT BLOCKS FIRE; W CRACKS, ! BREAKS NEXT", x + 10, top + 55,
            (SDL_Color){177, 218, 221, 255}, r->font_tiny);
    }
}
