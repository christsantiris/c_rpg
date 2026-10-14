#include "goblin_king_renderer.h"
#include "../game/goblin_king.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include <stdio.h>

void draw_goblin_king_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    draw_mountain_fortress_floor(r, sx, sy, x, y);
    if (tile == TILE_GOBLIN_KING_RELIQUARY) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){224, 173, 80, 255});
        renderer_draw_text(r, "GOBLIN KING'S SHIELD", sx * TILE_SIZE - 48, sy * TILE_SIZE - 14,
            (SDL_Color){255, 208, 135, 255}, r->font_tiny);
        return;
    }
    if (tile == TILE_GOBLIN_KEG_SPENT) {
        draw_world_prop(r, sx, sy, PROP_RUBBLE, (SDL_Color){109, 81, 58, 255});
        return;
    }
    // Native pixel-art staves, rounded silhouette, iron hoops and a powder mark.
    static const char *art[24] = {
        "........................", "........................",
        "...........oo...........", "..........offo..........",
        ".......oooooooooo.......", "......ommmmmmmmmmo......",
        ".....ommhhhhhhhmmmo.....", ".....ommhhhhhhhmmmo.....",
        "....oWwhwhwhwhwdddo.....", "....oWwhwhwhwhwdddo.....",
        "....ommmmmmmmmmmmmo.....", "....omhhhhhhhhhhmmo.....",
        "....oWwhwhrrwhwdddo.....", "....oWwhwrrrrhwdddo.....",
        "....oWwhwrrrrhwdddo.....", "....oWwhwhrrwhwdddo.....",
        "....ommmmmmmmmmmmmo.....", "....omhhhhhhhhhhmmo.....",
        ".....oWwhwhwhwdddo......", ".....oWwhwhwhwdddo......",
        "......ommmmmmmmmo.......", ".......ooooooooo........",
        "........................", "........................"
    };
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24; col++) {
            SDL_Color c;
            switch (art[row][col]) {
                case 'o': c = (SDL_Color){29, 23, 24, 255}; break;
                case 'm': c = (SDL_Color){81, 79, 85, 255}; break;
                case 'h': c = (SDL_Color){157, 141, 112, 255}; break;
                case 'W': c = (SDL_Color){191, 141, 78, 255}; break;
                case 'w': c = (SDL_Color){139, 92, 49, 255}; break;
                case 'd': c = (SDL_Color){89, 52, 34, 255}; break;
                case 'r': c = (SDL_Color){176, 50, 38, 255}; break;
                case 'f': c = (SDL_Color){tile == TILE_GOBLIN_KEG ? 189 : 255, 189, 76, 255}; break;
                default: continue;
            }
            SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
            SDL_Rect pixel = {sx * TILE_SIZE + col, sy * TILE_SIZE + row, 1, 1};
            SDL_RenderFillRect(r->sdl, &pixel);
        }
    }
    if (tile != TILE_GOBLIN_KEG) {
        renderer_draw_text(r, "*", sx * TILE_SIZE + 9, sy * TILE_SIZE - 5,
            (SDL_Color){255, 206, 102, 255}, r->font_tiny);
    }
}

void draw_goblin_king_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MOUNTAINS || g->level != MOUNTAIN_BOSS_LEVEL) {
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
            int fuse = goblin_king_keg_marks(g, x, y);
            int turns = 0;
            for (int i = 0; i < g->enemy_count; i++) {
                if (goblin_king_attack_marks(g, &g->enemies[i], x, y)) {
                    turns = g->enemies[i].move_timer;
                }
            }
            if (!turns && !fuse) {
                continue;
            }
            int px = viewport_to_screen_x(v, x) * TILE_SIZE;
            int py = viewport_to_screen_y(v, y) * TILE_SIZE;
            SDL_Rect rect = {px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2};
            SDL_SetRenderDrawColor(r->sdl, fuse ? 205 : 155, fuse ? 103 : 42, 34, 85);
            SDL_RenderFillRect(r->sdl, &rect);
            SDL_SetRenderDrawColor(r->sdl, 255, fuse ? 207 : 145, 103, 255);
            SDL_RenderDrawRect(r->sdl, &rect);
            char label[12];
            if (fuse && turns) {
                snprintf(label, sizeof(label), "%d/%d", turns, fuse);
            } else {
                snprintf(label, sizeof(label), "%d", fuse ? fuse : turns);
            }
            renderer_draw_text(r, label, px + (fuse && turns ? 2 : 9), py + 4, (SDL_Color){255, 238, 198, 255}, r->font_tiny);
            if (fuse) {
                SDL_RenderDrawLine(r->sdl, px + 5, py + 18, px + 18, py + 18);
                SDL_RenderDrawLine(r->sdl, px + 11, py + 14, px + 11, py + 21);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, blend);
}

void draw_goblin_king_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_MOUNTAINS || g->level != MOUNTAIN_BOSS_LEVEL) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_MOUNTAIN_GOBLIN_KING || e->max_hp <= 0 ||
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
        SDL_SetRenderDrawColor(r->sdl, 34, 24, 24, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 178, 105, 62, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        const char *name = width < 365 ? (e->attack_phase ? "GOBLIN KING: PHASE 2" : "GOBLIN KING: PHASE 1") :
            (e->attack_phase ? "GOBLIN KING: POWDER KING" : "GOBLIN KING: FORTRESS CHIEF");
        renderer_draw_text(r, name, x + 10, top + 8, (SDL_Color){233, 216, 250, 255}, r->font_tiny);
        SDL_Rect bar = {x + 10, top + 23, width - 20, 8};
        SDL_SetRenderDrawColor(r->sdl, 52, 39, 70, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = bar.w * e->hp / e->max_hp;
        SDL_SetRenderDrawColor(r->sdl, 208, 111, 60, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char hint[80];
        if (e->move_timer > 0) {
            const char *attack = e->boss_attack == GOBLIN_KING_BOMBS ? (width < 280 ? "BOMBS" : "POWDER BOMBARDMENT") : "THROWN AXE";
            snprintf(hint, sizeof(hint), "%s: %d TURN%s%s", attack, e->move_timer, e->move_timer == 1 ? "" : "S", e->frozen_turns > 0 ? (width < 280 ? " *" : " (FROZEN)") : "");
        } else {
            snprintf(hint, sizeof(hint), "%s", e->move_timer < 0 ? "RECOVERING: ATTACK NOW" : "PREPARING NEXT ATTACK");
        }
        renderer_draw_text(r, hint, x + 10, top + 38, (SDL_Color){e->move_timer < 0 ? 139 : 247, 207, 164, 255}, r->font_tiny);
        int kegs = goblin_king_remaining_kegs(g);
        snprintf(hint, sizeof(hint), width < 280 ? "KEGS: %d" : "KEGS: %d  INTERACT TO PRIME", kegs);
        renderer_draw_text(r, hint, x + 10, top + 55, (SDL_Color){186, 178, 211, 255}, r->font_tiny);
    }
}

