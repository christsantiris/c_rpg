#include "prism_sovereign_renderer.h"
#include "../game/prism_sovereign.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>
#include <stdlib.h>

void draw_prism_sovereign_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    draw_glassdeep_ruin(r, sx, sy, mx, my);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_PRISM_RELIQUARY || tile == TILE_PRISM_PAVED_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){159, 196, 237, 255});
        renderer_draw_text(r, "SOVEREIGN REWARDS", px - 36, py - 12, (SDL_Color){214, 207, 251, 255}, r->font_tiny);
        return;
    }
    draw_world_prop(r, sx, sy, PROP_CRYSTAL, tile == TILE_PRISM_RETURN ? (SDL_Color){135, 231, 193, 255} :
        tile == TILE_PRISM_RIGHT ? (SDL_Color){200, 155, 238, 255} : (SDL_Color){134, 198, 237, 255});
    SDL_SetRenderDrawColor(r->sdl, 229, 243, 255, 255);
    SDL_RenderDrawLine(r->sdl, px + 8, py + 7, px + 12, py + 3);
    SDL_RenderDrawLine(r->sdl, px + 12, py + 3, px + 17, py + 12);
    const char *label = tile == TILE_PRISM_RETURN ? "A: RETURN" : tile == TILE_PRISM_RIGHT ? "A: RIGHT" : "A: LEFT";
    renderer_draw_text(r, label, px - 84, py + 8, (SDL_Color){204, 227, 249, 255}, r->font_tiny);
}

void draw_prism_sovereign_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_GLASSDEEP || g->level != GLASSDEEP_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_PRISM_SOVEREIGN || e->move_timer <= 0) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !prism_sovereign_attack_marks(g, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_Rect tile = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, 102, 91, 177, 105);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 191, 177, 249, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                char label[8];
                snprintf(label, sizeof(label), "%d", e->move_timer);
                renderer_draw_text(r, label, px + 9, py + 3, (SDL_Color){233, 237, 255, 255}, r->font_tiny);
            }
        }
        PrismSegment segments[4];
        int count = prism_sovereign_segments(g, e, segments);
        for (int n = 0; n < count; n++) {
            const PrismSegment *s = &segments[n];
            int dx = (s->x2 > s->x1) - (s->x2 < s->x1);
            int dy = (s->y2 > s->y1) - (s->y2 < s->y1);
            int length = abs(s->x2 - s->x1) + abs(s->y2 - s->y1);
            for (int step = 1; step <= length; step++) {
                int x = s->x1 + dx * step;
                int y = s->y1 + dy * step;
                if (!viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE + TILE_SIZE / 2;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE + 18;
                SDL_SetRenderDrawColor(r->sdl, n % 2 ? 131 : 216, 219, 247, 255);
                SDL_RenderDrawLine(r->sdl, px - dx * 10, py - dy * 7, px + dx * 10, py + dy * 5);
                if (step == length) {
                    SDL_Rect endpoint = {px - 3, py - 3, 6, 6};
                    SDL_RenderDrawRect(r->sdl, &endpoint);
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}
void draw_prism_sovereign_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_GLASSDEEP || g->level != GLASSDEEP_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_PRISM_SOVEREIGN || e->max_hp <= 0 ||
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
        SDL_SetRenderDrawColor(r->sdl, 27, 31, 45, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 154, 166, 226, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, width < 365 ? (e->attack_phase ? "PRISM: PHASE 2" : "PRISM: PHASE 1") :
            (e->attack_phase ? "PRISM SOVEREIGN: FRACTURED CROWN" : "PRISM SOVEREIGN: FOCUSED LIGHT"),
            x + 10, top + 8, (SDL_Color){210, 218, 255, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 49, 44, 72, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 143, 184, 222, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == PRISM_SPLIT ? "SPLIT BEAMS" : "PRISM BEAM";
            snprintf(hint, sizeof(hint), width < 280 ? "%s: %d%s" : "%s: %d ACTION%s", attack, e->move_timer,
                width < 280 ? (e->frozen_turns > 0 ? " *" : "") : (e->frozen_turns > 0 ? " (FROZEN)" : e->move_timer == 1 ? "" : "S"));
        } else if (e->move_timer < 0) {
            snprintf(hint, sizeof(hint), "RECOVERY: %d%s", -e->move_timer, width < 280 ? "" : e->move_timer == -1 ? " ACTION" : " ACTIONS");
        } else {
            snprintf(hint, sizeof(hint), "PREPARING ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){255, 205, 148, 255}, r->font_tiny);
        snprintf(hint, sizeof(hint), width < 280 ? "A: ROTATE / LOCKED" : "A: ROTATE BETWEEN ATTACKS; WARNINGS LOCK FACES");
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){158, 226, 229, 255}, r->font_tiny);
    }
}
