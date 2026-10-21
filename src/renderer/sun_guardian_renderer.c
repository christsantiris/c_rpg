#include "sun_guardian_renderer.h"
#include "../game/sun_guardian.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_sun_guardian_tile(Renderer *r, int sx, int sy, int mx, int my, TileType tile) {
    draw_temple_floor(r, sx, sy, mx, my);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_GUARDIAN_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){244, 207, 112, 255});
        renderer_draw_text(r, "SUNWARDEN MANTLE", px - 36, py - 12, (SDL_Color){251, 226, 155, 255}, r->font_tiny);
        return;
    }
    int moon = tile == TILE_GUARDIAN_MOON_SANCTUARY;
    draw_world_prop(r, sx, sy, PROP_SUN_PLATE, moon ? (SDL_Color){115, 212, 233, 255} : (SDL_Color){244, 197, 91, 255});
    SDL_SetRenderDrawColor(r->sdl, moon ? 147 : 255, moon ? 231 : 222, moon ? 247 : 138, 255);
    SDL_Rect border = {px + 2, py + 2, 20, 20};
    SDL_RenderDrawRect(r->sdl, &border);
    renderer_draw_text(r, moon ? "M" : "S", px + 9, py + 8, (SDL_Color){255, 247, 207, 255}, r->font_tiny);
}

void draw_sun_guardian_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_TEMPLE || g->level != TEMPLE_DEPTH) {
        return;
    }
    SDL_BlendMode blend;
    SDL_GetRenderDrawBlendMode(r->sdl, &blend);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_FALLEN_SUN_GUARDIAN) {
            continue;
        }
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (!map_is_explored(&g->map, x, y) || !sun_guardian_attack_marks(g, e, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                int lunar = e->boss_attack == GUARDIAN_LUNAR;
                SDL_Rect cell = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
                SDL_SetRenderDrawColor(r->sdl, lunar ? 85 : 214, lunar ? 146 : 129, lunar ? 197 : 44, 120);
                SDL_RenderFillRect(r->sdl, &cell);
                SDL_SetRenderDrawColor(r->sdl, lunar ? 148 : 255, lunar ? 229 : 211, lunar ? 255 : 119, 255);
                SDL_RenderDrawRect(r->sdl, &cell);
                char label[8];
                snprintf(label, sizeof(label), "%d", e->move_timer);
                renderer_draw_text(r, label, px + 9, py + 4, (SDL_Color){255, 248, 219, 255}, r->font_tiny);
            }
        }
    }
    for (int n = 0; n < 2; n++) {
        int x = n ? 35 : 29;
        if (viewport_is_visible(v, x, 9) && map_is_explored(&g->map, x, 9) && g->map.tiles[9][x] == TILE_TEMPLE_ALTAR) {
            renderer_draw_text(r, "A", viewport_to_screen_x(v, x) * TILE_SIZE + 9,
                viewport_to_screen_y(v, 9) * TILE_SIZE + 16, (SDL_Color){251, 230, 172, 255}, r->font_tiny);
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

int sun_guardian_compact_status(const Renderer *r, const GameState *g, const Viewport *v) {
    int available = r->screen_w - INFO_PANEL_W;
    if (g->location != LOCATION_TEMPLE || g->level != TEMPLE_DEPTH || available < 176 || available >= 1004) {
        return 0;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (e->active && e->type == ENEMY_FALLEN_SUN_GUARDIAN && e->max_hp > 0 &&
            viewport_is_visible(v, e->x, e->y) && map_is_explored(&g->map, e->x, e->y)) {
            return 1;
        }
    }
    return 0;
}

void draw_sun_guardian_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_TEMPLE || g->level != TEMPLE_DEPTH) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_FALLEN_SUN_GUARDIAN || e->max_hp <= 0 ||
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
        int top = x < MAP_W / 2 + 12 ? MAP_H / 2 + 16 : 8;
        SDL_Rect panel = {x, top, width, 95};
        SDL_SetRenderDrawColor(r->sdl, 39, 31, 25, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 222, 179, 100, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        static const char *phases[] = {"SUNBOUND ARMOR", "CRACKED CORE", "ECLIPSE"};
        char text[80];
        int phase = e->attack_phase >= 0 && e->attack_phase <= 2 ? e->attack_phase : 0;
        if (width < 300) {
            snprintf(text, sizeof(text), "GUARDIAN: PHASE %d", phase + 1);
        } else {
            snprintf(text, sizeof(text), "SUN GUARDIAN: %s", phases[phase]);
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){255, 229, 168, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 74, 57, 45, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 236, 179, 76, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        static const char *attacks[] = {"PREPARING", "SOLAR LANCE", "SUNBURST RING", "SOLAR LANES", "LUNAR BURST"};
        if (e->move_timer > 0) {
            int attack = e->boss_attack >= GUARDIAN_LANCE && e->boss_attack <= GUARDIAN_LUNAR ? e->boss_attack : 0;
            snprintf(text, sizeof(text), "%s: %d%s", attacks[attack], e->move_timer, e->frozen_turns > 0 ? " *" : "");
        } else {
            snprintf(text, sizeof(text), e->move_timer < 0 ? "RECOVERY: 1 ACTION" : "PREPARING ATTACK");
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){255, 216, 141, 255}, r->font_tiny);
        int lunar = e->boss_attack == GUARDIAN_LUNAR;
        if (e->move_timer > 0) {
            snprintf(text, sizeof(text), width < 300 ? "%s GUARDS NEXT" : "%s SANCTUARIES SAFE; %s GUARD HALVES HIT",
                lunar ? "SUN" : "MOON", lunar ? "SUN" : "MOON");
        } else {
            snprintf(text, sizeof(text), width < 300 ? "A: SUN/MOON ALTARS" : "ALTARS CHOOSE ARMOR OR EXPOSED CORE");
        }
        renderer_draw_text(r, text, x + 10, top + 55, (SDL_Color){156, 225, 238, 255}, r->font_tiny);
        if (sun_guardian_compact_status(r, g, v)) {
            snprintf(text, sizeof(text), width < 300 ? "%s; %d FOES SEALED" : "A: ALTARS | %s; %d FOES SEALED",
                g->temple_alignment ? "MOON" : "SUN", game_temple_remaining_enemies(g));
        } else {
            snprintf(text, sizeof(text), "A: ALTARS | %s: %s | %d LEFT", g->temple_alignment ? "MOON" : "SUN",
                g->temple_alignment ? "CORE EXPOSED" : "BOSS GUARDED", game_temple_remaining_enemies(g));
        }
        renderer_draw_text(r, text, x + 10, top + 72, (SDL_Color){246, 226, 179, 255}, r->font_tiny);
    }
}
