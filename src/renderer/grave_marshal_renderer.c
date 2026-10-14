#include "grave_marshal_renderer.h"
#include "../game/grave_marshal.h"
#include "../game/catacombs.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_grave_marshal_tile(Renderer *r, int sx, int sy, TileType tile) {
    draw_world_prop(r, sx, sy, tile == TILE_MARSHAL_RELIQUARY ? PROP_CHEST : PROP_TOMB,
        (SDL_Color){164, 188, 195, 255});
    const char *label = tile == TILE_MARSHAL_RELIQUARY ? "GRAVEKEEPER'S MANTLE" :
        tile == TILE_MARSHAL_NICHE ? "TOMB NICHE" : "EMPTY NICHE";
    renderer_draw_text(r, label, sx * TILE_SIZE - (tile == TILE_MARSHAL_RELIQUARY ? 46 : 24), sy * TILE_SIZE - 12,
        (SDL_Color){177, 226, 241, 255}, r->font_tiny);
}

void draw_grave_marshal_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CATACOMBS || g->level != CATACOMBS_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_GRAVE_MARSHAL || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !grave_marshal_attack_marks(&g->map, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 51, 132, 171, 100);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 177, 232, 247, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                if (e->boss_attack == MARSHAL_GUARDS) {
                    SDL_RenderDrawLine(r->sdl, px + 6, py + 16, px + 18, py + 16);
                    SDL_RenderDrawLine(r->sdl, px + 12, py + 13, px + 12, py + 21);
                } else if (e->boss_attack == MARSHAL_SWEEP) {
                    SDL_RenderDrawLine(r->sdl, px + 4, py + 20, px + 20, py + 16);
                } else {
                    SDL_RenderDrawLine(r->sdl, px + 4, py + 18, px + 20, py + 18);
                    SDL_RenderDrawLine(r->sdl, px + 12, py + 14, px + 12, py + 22);
                }
                char label[8];
                snprintf(label, sizeof(label), e->boss_attack == MARSHAL_GUARDS ? "H%d" : "%d", e->move_timer);
                renderer_draw_text(r, label, px + (e->boss_attack == MARSHAL_GUARDS ? 3 : 9), py + 4, (SDL_Color){245, 241, 197, 255}, r->font_tiny);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_grave_marshal_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CATACOMBS || g->level != CATACOMBS_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_GRAVE_MARSHAL || e->max_hp <= 0 ||
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
        SDL_SetRenderDrawColor(r->sdl, 22, 29, 39, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 112, 169, 193, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        const char *name = width < 365 ? (e->attack_phase ? "MARSHAL: PHASE 2" : "MARSHAL: PHASE 1") :
            (e->attack_phase ? "GRAVE MARSHAL: FUNERAL MARCH" : "GRAVE MARSHAL: HONORED COMMANDER");
        renderer_draw_text(r, name, x + 10, top + 8, (SDL_Color){233, 216, 250, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 52, 39, 70, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 110, 183, 217, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == MARSHAL_SWEEP ? "POLEARM" : e->boss_attack == MARSHAL_CROSS ? "CROSS" :
                e->boss_attack == MARSHAL_GUARDS ? "HONOR GUARD" : "CHANNEL";
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
        int flames = catacombs_lit_braziers(&g->map, g->map.room_count - 1);
        snprintf(hint, sizeof(hint), width < 280 ? "BRAZIERS: %d" : "BRAZIERS: %d  INTERACT TO SILENCE CHANNEL", flames);
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){186, 178, 211, 255}, r->font_tiny);
    }
}

