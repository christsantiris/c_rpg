#include "castle_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include "../game/royal_arcanist.h"
#include "../game/lord_veyr.h"

static void rect(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_Rect box = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &box);
}

static int carpet(const Map *m, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return 0;
    }
    TileType tile = m->tiles[y][x];
    return tile == TILE_CASTLE_CARPET || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN ||
        tile == TILE_CASTLE_TRAP_OPEN || tile == TILE_CASTLE_GATE || tile == TILE_CASTLE_GATE_OPEN || tile == TILE_CASTLE_FIRE_RUNE;
}

void castle_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_CASTLE_TRAP_HIDDEN) {
        tile = TILE_CASTLE_FLOOR;
    }
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    int shade = (x * 17 + y * 11) % 9;
    SDL_Color stone = {54 + shade, 49 + shade, 65 + shade, 255};
    if (tile == TILE_CASTLE_WALL) {
        stone = (SDL_Color){30 + shade, 27 + shade, 39 + shade, 255};
    }
    rect(r, px, py, 24, 24, (SDL_Color){24, 23, 32, 255});
    rect(r, px + 1, py + 1, 22, 22, stone);
    rect(r, px + 2, py + 2, 20, 1, tile == TILE_CASTLE_WALL ? (SDL_Color){47, 42, 56, 255} : (SDL_Color){79, 72, 86, 255});
    if (tile == TILE_CASTLE_FLOOR || tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT || tile == TILE_CASTLE_MUSTER) {
        rect(r, px + 2, py + 11, 20, 1, (SDL_Color){37, 34, 45, 255});
        rect(r, px + 7 + (x % 2) * 5, py + 2, 1, 9, (SDL_Color){37, 34, 45, 255});
        rect(r, px + 12 - (x % 2) * 5, py + 12, 1, 10, (SDL_Color){37, 34, 45, 255});
        rect(r, px + 3 + (x * 7 + y) % 15, py + 4 + (y * 3 + x) % 15, 2, 1, (SDL_Color){89, 80, 96, 255});
    }
    if (tile == TILE_CASTLE_WALL) {
        rect(r, px, py + 11, 24, 2, (SDL_Color){22, 20, 28, 255});
        rect(r, px + 11, py, 2, 11, (SDL_Color){22, 20, 28, 255});
        rect(r, px + 4, py + 13, 2, 11, (SDL_Color){22, 20, 28, 255});
        // Stained-glass accents and gold masonry break up long walls.
        if ((x + y * 3) % 13 == 0 && y > 0 && map_is_walkable(&g->map, x, y + 1)) {
            rect(r, px + 6, py + 3, 12, 18, (SDL_Color){126, 99, 48, 255});
            rect(r, px + 8, py + 5, 8, 14, (SDL_Color){70, 43, 110, 255});
            rect(r, px + 11, py + 6, 2, 12, (SDL_Color){156, 104, 205, 255});
        } else if ((x + y) % 7 == 0 && map_is_walkable(&g->map, x, y + 1)) {
            int flicker = (SDL_GetTicks() / AMBIENT_FRAME_MS + x) % 2;
            rect(r, px + 10, py + 10, 4, 11, (SDL_Color){88, 61, 44, 255});
            rect(r, px + 8, py + 6, 8, 7, (SDL_Color){222, 103 + flicker * 20, 36, 255});
            rect(r, px + 10, py + 4 + flicker, 4, 7, (SDL_Color){255, 212, 91, 255});
        }
        return;
    }
    if (tile == TILE_CASTLE_CARPET || tile == TILE_CASTLE_FIRE_RUNE) {
        rect(r, px, py, 24, 24, (SDL_Color){89 + shade, 27, 41, 255});
        if (!carpet(&g->map, x - 1, y)) {
            rect(r, px + 1, py, 2, 24, (SDL_Color){155, 111, 54, 255});
        }
        if (!carpet(&g->map, x + 1, y)) {
            rect(r, px + 21, py, 2, 24, (SDL_Color){155, 111, 54, 255});
        }
        if (!carpet(&g->map, x, y - 1)) {
            rect(r, px, py + 1, 24, 2, (SDL_Color){155, 111, 54, 255});
        }
        if (!carpet(&g->map, x, y + 1)) {
            rect(r, px, py + 21, 24, 2, (SDL_Color){155, 111, 54, 255});
        }
        if ((x + y) % 5 == 0) {
            rect(r, px + 10, py + 10, 4, 4, (SDL_Color){128, 61, 53, 255});
        }
    }
    if (tile == TILE_CASTLE_FIRE_RUNE) {
        if (g->level == 6 && g->castle_fire_phase[5] == 3) {
            draw_world_prop(r, sx, sy, PROP_LOW_FLAME_1, (SDL_Color){255, 161, 57, 255});
        }
        SDL_SetRenderDrawColor(r->sdl, 202, 137, 68, 255);
        SDL_RenderDrawLine(r->sdl, px + 6, py + 16, px + 12, py + 6);
        SDL_RenderDrawLine(r->sdl, px + 12, py + 6, px + 18, py + 16);
        SDL_RenderDrawLine(r->sdl, px + 6, py + 16, px + 18, py + 16);
        rect(r, px + 11, py + 10, 2, 4, (SDL_Color){240, 188, 91, 255});
    } else if (tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT) {
        SDL_Color orb = tile == TILE_CASTLE_WARD ? (SDL_Color){101, 218, 239, 255} : (SDL_Color){77, 78, 94, 255};
        draw_world_prop(r, sx, sy, PROP_CRYSTAL, orb);
    } else if (tile == TILE_CASTLE_MUSTER) {
        draw_world_prop(r, sx, sy, PROP_MUSTER, (SDL_Color){222, 183, 104, 255});
    } else if (tile == TILE_CASTLE_TABLE) {
        draw_world_prop(r, sx, sy, PROP_BANQUET_TABLE, (SDL_Color){179, 150, 105, 255});
    } else if (tile == TILE_CASTLE_BOOKCASE) {
        draw_world_prop(r, sx, sy, PROP_BOOKCASE, (SDL_Color){170, 116, 58, 255});
    } else if (tile == TILE_CASTLE_THRONE) {
        draw_world_prop(r, sx, sy, PROP_THRONE, (SDL_Color){224, 177, 82, 255});
    } else if (tile == TILE_CASTLE_PILLAR) {
        draw_world_prop(r, sx, sy, PROP_PILLAR, (SDL_Color){136, 111, 63, 255});
    } else if (tile == TILE_CASTLE_BANNER) {
        draw_world_prop(r, sx, sy, PROP_BANNER, (SDL_Color){198, 159, 75, 255});
    } else if (tile == TILE_CASTLE_GATE || tile == TILE_CASTLE_GATE_OPEN) {
        draw_world_prop(r, sx, sy, tile == TILE_CASTLE_GATE ? PROP_GATE : PROP_GATE_OPEN, (SDL_Color){156, 119, 65, 255});
    } else if (tile == TILE_CASTLE_LEVER) {
        draw_world_prop(r, sx, sy, PROP_LEVER, (SDL_Color){159, 45, 55, 255});
    } else if (tile == TILE_CASTLE_TRAP_OPEN) {
        draw_world_prop(r, sx, sy, PROP_HATCH, (SDL_Color){199, 143, 60, 255});
    } else if (tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN) {
        draw_world_prop(r, sx, sy, tile == TILE_STAIRS_UP ? PROP_STAIRS_UP : PROP_STAIRS_DOWN, (SDL_Color){181, 146, 91, 255});
        renderer_draw_text(r, tile == TILE_STAIRS_UP ? "^" : "v", px + 8, py + 6, (SDL_Color){245, 206, 109, 255}, r->font_tiny);
    }
    if (g->level == 2 && x == 53 && y == 55 && g->castellan_reward_state == 1) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){218, 180, 96, 255});
        renderer_draw_text(r, "BULWARK: A", px - 16, py - 12, (SDL_Color){255, 224, 160, 255}, r->font_tiny);
    }
    if (g->level == 2 && tile == TILE_CASTLE_LEVER && y == 51) {
        renderer_draw_text(r, x == 37 ? "WEST: A" : "EAST: A", px - 10, py - 12, (SDL_Color){255, 224, 160, 255}, r->font_tiny);
    }
    if (g->level == 4 && y == 54 && (x == 37 || x == 53) && (tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT)) {
        char text[16];
        snprintf(text, sizeof(text), "%s: %s", x == 37 ? "WEST" : "EAST", tile == TILE_CASTLE_WARD ? "A" : "OFF");
        renderer_draw_text(r, text, px - 16, py - 12, (SDL_Color){174, 236, 245, 255}, r->font_tiny);
    }
    if (g->level == 6 && y == 54 && (x == 37 || x == 53) && (tile == TILE_CASTLE_WARD || tile == TILE_CASTLE_WARD_SPENT)) {
        renderer_draw_text(r, tile == TILE_CASTLE_WARD ? (x == 37 ? "WEST: A" : "EAST: A") : "WARD: OFF", px - 16, py - 12, (SDL_Color){241, 216, 145, 255}, r->font_tiny);
    }
    if (g->level == 4 && x == 53 && y == 56 && g->arcanist_reward_state == 1) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){163, 140, 214, 255});
        renderer_draw_text(r, "VESTMENTS: A", px - 28, py - 12, (SDL_Color){234, 218, 255, 255}, r->font_tiny);
    }
}

void castle_draw_enemy(Renderer *r, int sx, int sy, const Enemy *e) {
    // Native frames are bottom-centered in 32-pixel regular / 40-pixel boss cells.
    static const SDL_Rect regular[5] = {{4, 6, 24, 26}, {36, 6, 24, 26}, {68, 6, 24, 26}, {4, 38, 24, 26}, {36, 38, 24, 26}};
    static const SDL_Rect bosses[5] = {{2, 4, 36, 36}, {42, 4, 36, 36}, {82, 4, 36, 36}, {2, 44, 36, 36}, {42, 44, 36, 36}};
    int index = e->type - ENEMY_OATHBOUND_SOLDIER;
    SDL_Texture *texture = index < 5 ? r->castle_enemy_texture : r->castle_boss_texture;
    SDL_Rect source;
    if (index < 5) {
        source = regular[index];
    } else {
        int phase = e->attack_phase >= 3 ? 2 : e->attack_phase == 2 ? 1 : 0;
        source = bosses[index == 7 ? 2 + phase : index - 5];
    }
    SDL_Rect destination = {sx * 24 - (e->is_boss ? 6 : 0), sy * 24 - (e->is_boss ? 12 : 2), e->is_boss ? 36 : 24, e->is_boss ? 36 : 26};
    if (texture) {
        if (e->type == ENEMY_CASTELLAN && e->boss_attack == 2) {
            SDL_SetTextureColorMod(texture, 255, 203, 185);
        } else if (e->type == ENEMY_ROYAL_ARCANIST && e->attack_phase == 2) {
            SDL_SetTextureColorMod(texture, 190, 224, 255);
        }
        SDL_RenderCopy(r->sdl, texture, &source, &destination);
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    } else {
        draw_orc(r, sx, sy);
    }
    if (e->type == ENEMY_IRON_WARDEN || e->type == ENEMY_CASTELLAN ||
        (e->type == ENEMY_LORD_VEYR && e->attack_phase <= 1)) {
        int cx = sx * 24 + 12 + e->facing_dx * 10;
        int cy = sy * 24 + 12 + e->facing_dy * 10;
        rect(r, cx - 2, cy - 2, 4, 4, e->move_timer == 2 ? (SDL_Color){94, 88, 107, 255} : (SDL_Color){242, 195, 84, 255});
    }
}

void castle_draw_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR) {
        return;
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
            if (castle_fire_warning(g, x, y) && map_is_explored(&g->map, x, y)) {
                SDL_Rect tile = {(x - v->cam_x) * 24 + 1, (y - v->cam_y) * 24 + 1, 22, 22};
                SDL_SetRenderDrawColor(r->sdl, 246, 135, 39, 90);
                SDL_RenderFillRect(r->sdl, &tile);
                SDL_SetRenderDrawColor(r->sdl, 255, 204, 94, 255);
                SDL_RenderDrawRect(r->sdl, &tile);
                renderer_draw_text(r, g->castle_fire_phase[g->level - 1] == 1 ? "2" : "1", tile.x + 8, tile.y + 4, (SDL_Color){255, 236, 181, 255}, r->font_tiny);
            }
            for (int i = 0; i < g->enemy_count; i++) {
                const Enemy *e = &g->enemies[i];
                int rally = (e->attack_phase == CASTELLAN_RALLY && castle_castellan_marks(e, x, y)) || lord_veyr_escort_marks(e, x, y);
                int barrier = royal_arcanist_barrier_change(e, x, y) + lord_veyr_barrier_change(e, x, y);
                int attack_marks = castle_attack_marks(&g->map, e, x, y);
                if (attack_marks || rally || barrier) {
                    if (!map_is_explored(&g->map, x, y)) {
                        continue;
                    }
                    SDL_Rect tile = {(x - v->cam_x) * 24 + 1, (y - v->cam_y) * 24 + 1, 22, 22};
                    SDL_SetRenderDrawColor(r->sdl, 204, 108, 239, 55);
                    SDL_RenderFillRect(r->sdl, &tile);
                    SDL_SetRenderDrawColor(r->sdl, 236, 159, 252, 255);
                    SDL_RenderDrawRect(r->sdl, &tile);
                    if (e->type == ENEMY_CASTELLAN) {
                        char text[8];
                        snprintf(text, sizeof(text), "%s%d", rally ? "G" : "", e->move_timer >= 3 ? e->move_timer - 1 : 1);
                        renderer_draw_text(r, text, tile.x + (rally ? 3 : 8), tile.y + 4, (SDL_Color){255, 231, 174, 255}, r->font_tiny);
                    } else if (e->type == ENEMY_ROYAL_ARCANIST || e->type == ENEMY_LORD_VEYR) {
                        char text[8];
                        int turns = e->move_timer >= 3 ? e->move_timer - 1 : 1;
                        int west = e->type == ENEMY_LORD_VEYR ? e->boss_attack == VEYR_WEST_LANE : e->boss_attack == ARCANIST_WEST_LANE;
                        int east = e->type == ENEMY_LORD_VEYR ? e->boss_attack == VEYR_EAST_LANE : e->boss_attack == ARCANIST_EAST_LANE;
                        const char *label = rally ? "G" : barrier && attack_marks ? "X" : barrier ? (barrier < 0 ? "C" : "O") :
                            west ? "W" : east ? "E" : e->type == ENEMY_LORD_VEYR && e->boss_attack == VEYR_FIRE ? "F" : "";
                        snprintf(text, sizeof(text), "%s%d", label, turns);
                        SDL_SetRenderDrawColor(r->sdl, barrier ? 248 : 134, barrier ? 199 : 226, barrier ? 104 : 247, 255);
                        SDL_RenderDrawRect(r->sdl, &tile);
                        int inset = *label ? 3 : 8;
                        rect(r, tile.x + inset - 1, tile.y + 3, *label ? 18 : 10, 10, (SDL_Color){32, 27, 47, 255});
                        renderer_draw_text(r, text, tile.x + inset, tile.y + 4, (SDL_Color){255, 241, 196, 255}, r->font_tiny);
                    }
                    break;
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);
}

void castle_draw_arcanist_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 4) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_ROYAL_ARCANIST || e->max_hp <= 0 || !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
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
        rect(r, x, top, width, 78, (SDL_Color){27, 26, 47, 255});
        char text[80];
        if (width < 300) {
            snprintf(text, sizeof(text), "ARCANIST: PHASE %d", e->attack_phase == 2 ? 2 : 1);
        } else {
            snprintf(text, sizeof(text), "ROYAL ARCANIST: %s", e->attack_phase == 2 ? "SHATTERED LITURGY" : "WARDED SCHOLAR");
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){220, 225, 255, 255}, r->font_tiny);
        rect(r, x + 10, top + 23, width - 20, 8, (SDL_Color){59, 43, 81, 255});
        rect(r, x + 10, top + 23, (width - 20) * e->hp / e->max_hp, 8, (SDL_Color){153, 173, 228, 255});
        if (e->attack_target_x >= 0) {
            static const char *attacks[] = {"PREPARING", "WARDED BLAST", "CHAPEL CROSS", "WEST WARD LANE", "EAST WARD LANE"};
            int attack = e->boss_attack >= ARCANIST_BLAST && e->boss_attack <= ARCANIST_EAST_LANE ? e->boss_attack : 0;
            snprintf(text, sizeof(text), "%s: %d%s", !e->boss_aux_x && !e->boss_aux_y ? "BLOCKED ATTACK" : attacks[attack], e->move_timer >= 3 ? e->move_timer - 1 : 1, e->frozen_turns > 0 ? " FROZEN" : "");
        } else if (e->move_timer == 2) {
            snprintf(text, sizeof(text), "RECOVERY: %d%s", 1 + e->revive_timer, e->frozen_turns > 0 ? " FROZEN" : "");
        } else {
            snprintf(text, sizeof(text), "PREPARING ATTACK");
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){178, 232, 245, 255}, r->font_tiny);
        renderer_draw_text(r, width < 300 ? "W/E PEDESTALS: A" : "DISABLE THE LINKED W/E PEDESTAL: A", x + 10, top + 55, (SDL_Color){218, 201, 240, 255}, r->font_tiny);
    }
}

void castle_draw_castellan_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 2) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_CASTELLAN || e->max_hp <= 0 || !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
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
        rect(r, x, top, width, 78, (SDL_Color){35, 25, 39, 255});
        char text[80];
        if (width < 300) {
            snprintf(text, sizeof(text), "CASTELLAN: PHASE %d", e->boss_attack == 2 ? 2 : 1);
        } else {
            snprintf(text, sizeof(text), "CASTELLAN: %s", e->boss_attack == 2 ? "BROKEN FORMATION" : "SHIELD COMMANDER");
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){255, 220, 171, 255}, r->font_tiny);
        rect(r, x + 10, top + 23, width - 20, 8, (SDL_Color){75, 42, 59, 255});
        rect(r, x + 10, top + 23, (width - 20) * e->hp / e->max_hp, 8, (SDL_Color){211, 153, 89, 255});
        static const char *attacks[] = {"PREPARING", "CHARGE", "SHIELD SWING", "FORMATION SWEEP", "GUARD RALLY"};
        if (e->attack_target_x >= 0) {
            int attack = e->attack_phase >= CASTELLAN_CHARGE && e->attack_phase <= CASTELLAN_RALLY ? e->attack_phase : 0;
            snprintf(text, sizeof(text), "%s: %d%s", !e->boss_aux_x && attack != CASTELLAN_RALLY ? "BLOCKED ATTACK" : attacks[attack], e->move_timer >= 3 ? e->move_timer - 1 : 1, e->frozen_turns > 0 ? " FROZEN" : "");
        } else if (e->move_timer == 2) {
            snprintf(text, sizeof(text), "RECOVERY: %d%s", 1 + e->revive_timer, e->frozen_turns > 0 ? " FROZEN" : "");
        } else {
            snprintf(text, sizeof(text), "PREPARING ATTACK");
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){255, 203, 143, 255}, r->font_tiny);
        renderer_draw_text(r, width < 300 ? "SIDE LEVERS: A" : "SIDE LEVERS INTERRUPT CHARGES; A TO USE", x + 10, top + 55, (SDL_Color){196, 209, 215, 255}, r->font_tiny);
    }
}

void castle_draw_veyr_status(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_CASTLE_INTERIOR || g->level != 6) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_LORD_VEYR || e->max_hp <= 0 || !viewport_is_visible(v, e->x, e->y) || !map_is_explored(&g->map, e->x, e->y)) {
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
        rect(r, x, top, width, 78, (SDL_Color){39, 25, 42, 255});
        static const char *phases[] = {"ARMORED RULER", "BROKEN THRONE", "UNBOUND CROWN"};
        int phase = e->attack_phase >= 3 ? 2 : e->attack_phase == 2 ? 1 : 0;
        char text[80];
        if (width < 300) {
            snprintf(text, sizeof(text), "VEYR: PHASE %d", phase + 1);
        } else {
            snprintf(text, sizeof(text), "LORD VEYR: %s", phases[phase]);
        }
        renderer_draw_text(r, text, x + 10, top + 8, (SDL_Color){251, 220, 156, 255}, r->font_tiny);
        rect(r, x + 10, top + 23, width - 20, 8, (SDL_Color){76, 41, 66, 255});
        rect(r, x + 10, top + 23, (width - 20) * e->hp / e->max_hp, 8, (SDL_Color){217, 153, 82, 255});
        if (e->attack_target_x >= 0) {
            static const char *attacks[] = {"PREPARING", "OATHBLADE", "THRONE ADVANCE", "ROYAL EDICT", "WEST WARD LANE", "EAST WARD LANE", "UNBOUND CROWN", "THRONE FIRE"};
            int attack = e->boss_attack >= VEYR_SWING && e->boss_attack <= VEYR_FIRE ? e->boss_attack : 0;
            snprintf(text, sizeof(text), "%s: %d%s", !e->boss_aux_x ? "BLOCKED ATTACK" : attacks[attack], e->move_timer >= 3 ? e->move_timer - 1 : 1, e->frozen_turns > 0 ? " FROZEN" : "");
        } else if (e->move_timer == 2) {
            snprintf(text, sizeof(text), "RECOVERY: %d%s", 1 + e->revive_timer, e->frozen_turns > 0 ? " FROZEN" : "");
        } else {
            snprintf(text, sizeof(text), "PREPARING ATTACK");
        }
        renderer_draw_text(r, text, x + 10, top + 38, (SDL_Color){242, 193, 222, 255}, r->font_tiny);
        renderer_draw_text(r, width < 300 ? "BOTH WARDS OFF: CROWN R2" : "BOTH WARDS OFF: CROWN RECOVERS 2 ACTIONS", x + 10, top + 55, (SDL_Color){193, 223, 222, 255}, r->font_tiny);
    }
}
