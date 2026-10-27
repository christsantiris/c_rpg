#include "ironfang_renderer.h"
#include "../game/ironfang.h"
#include "../game/war_machine.h"
#include "../game/combat_feedback.h"
#include "game_renderer.h"
#include "info_panel.h"
#include "world_props.h"
#include "controls_renderer.h"

static void box(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &rect);
}

void ironfang_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    int lit = ironfang_lit(g, x, y);
    int shade = lit ? 12 : 0;
    box(r, px, py, 24, 24, (SDL_Color){38 + shade, 37 + shade, 33 + shade, 255});
    for (int n = 0; n < 7; n++) {
        int xx = (x * 7 + y * 3 + n * 13) % 22;
        int yy = (x * 11 + y * 5 + n * 7) % 22;
        box(r, px + xx, py + yy, 2 + n % 2, 1, (SDL_Color){57 + shade, 53 + shade, 43 + shade, 255});
    }
    if (tile == TILE_IRON_WALL || tile == TILE_IRON_CRACK) {
        // Connected slate, iron seams, and a lit lower face above excavated floor.
        int edge = y + 1 < IRON_H && g->map.tiles[y + 1][x] != TILE_IRON_WALL;
        box(r, px, py, 24, 24, (SDL_Color){22 + shade / 2, 26 + shade / 2, 29 + shade / 2, 255});
        unsigned int grain = (unsigned int)(x * 73856093u) ^ (unsigned int)(y * 19349663u);
        for (int n = 0; n < 9; n++) {
            grain = grain * 1664525u + 1013904223u;
            int xx = grain % 23;
            int yy = (grain >> 9) % 23;
            box(r, px + xx, py + yy, 2, 1, (SDL_Color){37 + shade / 2, 43 + shade / 2, 46 + shade / 2, 255});
        }
        if (edge) {
            for (int row = 10; row < 21; row++) {
                box(r, px + 1, py + row, 22, 1, (SDL_Color){61 + (20 - row) / 2 + shade, 67 + (20 - row) / 2 + shade, 68 + (20 - row) / 2 + shade, 255});
            }
            SDL_SetRenderDrawColor(r->sdl, 35, 40, 43, 255);
            SDL_RenderDrawLine(r->sdl, px + 3 + x % 5, py + 12, px + 5 + x % 5, py + 20);
            SDL_RenderDrawLine(r->sdl, px + 17, py + 10, px + 15, py + 15);
        }
        if (g->level == IRON_DEPTH && (x * 11 + y * 7) % 23 == 0) {
            for (int n = 0; n < 3; n++) {
                for (int row = 0; row < 8 + n * 2; row++) {
                    box(r, px + 6 + n * 5, py + 19 - row, 3, 1, (SDL_Color){70 + n * 10, 125 + n * 12, 142 + n * 9, 255});
                }
                box(r, px + 6 + n * 5, py + 11 - n * 2, 1, 9 + n * 2, (SDL_Color){155, 197, 192, 255});
            }
        }
        if ((x * 7 + y * 3) % 11 == 0) {
            for (int n = 0; n < 12; n++) {
                box(r, px + 5 + n, py + 17 - n / 2, 2, 1, (SDL_Color){115, 79, 49, 255});
            }
        }
        if (edge) {
            box(r, px + 1, py + 18, 22, 2, (SDL_Color){81 + shade, 82 + shade, 75 + shade, 255});
            box(r, px + 1, py + 21, 22, 3, (SDL_Color){16, 20, 23, 255});
        }
        if (tile == TILE_IRON_CRACK) {
            SDL_SetRenderDrawColor(r->sdl, 192, 150, 83, 255);
            SDL_RenderDrawLine(r->sdl, px + 13, py + 3, px + 8, py + 12);
            SDL_RenderDrawLine(r->sdl, px + 8, py + 12, px + 14, py + 15);
            SDL_RenderDrawLine(r->sdl, px + 14, py + 15, px + 10, py + 22);
        }
        return;
    }
    if (tile == TILE_IRON_WATER || tile == TILE_IRON_TRESTLE) {
        box(r, px, py, 24, 24, (SDL_Color){14, 37, 44, 255});
        int flow = (SDL_GetTicks() / 250 + x + y) % 5;
        box(r, px + 3 + flow, py + 7, 7, 1, (SDL_Color){38, 77, 82, 255});
        box(r, px + 11 - flow, py + 18, 8, 1, (SDL_Color){32, 67, 75, 255});
    }
    if (tile == TILE_IRON_RAIL || tile == TILE_IRON_TRESTLE || tile == TILE_IRON_BROKEN) {
        int vertical = x == 30 && y < 25;
        for (int n = 2; n < 24; n += 6) {
            box(r, px + (vertical ? 4 : n), py + (vertical ? n : 4), vertical ? 16 : 3, vertical ? 3 : 16, (SDL_Color){91, 67, 42, 255});
        }
        if (tile != TILE_IRON_BROKEN) {
            for (int n = 0; n < 2; n++) {
                box(r, px + (vertical ? 6 + n * 11 : 0), py + (vertical ? 0 : 6 + n * 11), vertical ? 2 : 24, vertical ? 24 : 2, (SDL_Color){145, 151, 140, 255});
            }
        } else {
            box(r, px + 2, py + 5, 7, 2, (SDL_Color){142, 120, 80, 255});
            box(r, px + 16, py + 17, 7, 2, (SDL_Color){142, 120, 80, 255});
            renderer_draw_text(r, "!", px + 9, py + 6, (SDL_Color){237, 182, 91, 255}, r->font_tiny);
        }
    } else if (tile == TILE_IRON_LAMP || tile == TILE_IRON_LAMP_LIT) {
        box(r, px + 10, py + 9, 4, 14, (SDL_Color){67, 49, 34, 255});
        box(r, px + 6, py + 5, 12, 14, (SDL_Color){20, 25, 27, 255});
        box(r, px + 8, py + 7, 8, 9, (SDL_Color){tile == TILE_IRON_LAMP_LIT ? 225 : 76, tile == TILE_IRON_LAMP_LIT ? 167 : 76, 64, 255});
        box(r, px + 11, py + 7, 1, 11, (SDL_Color){76, 63, 41, 255});
        box(r, px + 7, py + 17, 10, 2, (SDL_Color){126, 107, 72, 255});
    } else if (tile == TILE_IRON_BUNK) {
        box(r, px + 3, py + 2, 18, 21, (SDL_Color){77, 52, 32, 255});
        box(r, px + 5, py + 4, 14, 17, (SDL_Color){109, 70, 42, 255});
        box(r, px + 5, py + 4, 14, 4, (SDL_Color){175, 157, 108, 255});
        box(r, px + 6, py + 10, 12, 10, (SDL_Color){100, 116, 57, 255});
        box(r, px + 7, py + 10, 1, 9, (SDL_Color){146, 151, 84, 255});
    } else if (tile == TILE_IRON_FORGE) {
        box(r, px + 3, py + 4, 18, 19, (SDL_Color){82, 85, 78, 255});
        box(r, px + 5, py + 9, 14, 12, (SDL_Color){25, 28, 27, 255});
        int pulse = (SDL_GetTicks() / 170 + x) % 3;
        box(r, px + 7, py + 15 - pulse, 10, 5 + pulse, (SDL_Color){227, 129, 46, 255});
        box(r, px + 10, py + 14, 4, 6, (SDL_Color){255, 213, 104, 255});
        box(r, px + 2, py + 3, 20, 3, (SDL_Color){142, 139, 115, 255});
    } else if (tile == TILE_IRON_SWITCH || tile == TILE_IRON_WINCH) {
        draw_world_prop(r, sx, sy, PROP_LEVER, (SDL_Color){203, 155, 76, 255});
    } else if (tile == TILE_IRON_CACHE) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){164, 122, 63, 255});
    } else if (tile == TILE_IRON_EMPTY) {
        box(r, px + 5, py + 13, 14, 8, (SDL_Color){75, 54, 35, 255});
        box(r, px + 7, py + 14, 10, 4, (SDL_Color){24, 25, 23, 255});
    } else if (tile == TILE_IRON_ENTRANCE || tile == TILE_IRON_EXIT) {
        // Boundary stairs recessed into the mine, with the same footprint as the passage.
        for (int n = 0; n < 5; n++) {
            box(r, px + 1, py + n * 5, 22, 4, (SDL_Color){105 + n * 5, 104 + n * 4, 87 + n * 3, 255});
            box(r, px + 1, py + n * 5 + 3, 22, 1, (SDL_Color){34, 36, 33, 255});
        }
        renderer_draw_text(r, tile == TILE_IRON_EXIT ? ">" : "<", px + 8, py + 8, (SDL_Color){239, 201, 116, 255}, r->font_tiny);
    }
    // Timber sets stand against the excavated wall, never across a walking tile.
    if (tile == TILE_IRON_FLOOR && x % 8 == 0 && y > 0 && g->map.tiles[y - 1][x] == TILE_IRON_WALL) {
        box(r, px + 1, py, 3, 13, (SDL_Color){101, 72, 43, 255});
        box(r, px + 20, py, 3, 13, (SDL_Color){78, 55, 36, 255});
        box(r, px, py, 24, 4, (SDL_Color){132, 99, 58, 255});
        box(r, px, py + 4, 24, 1, (SDL_Color){37, 30, 25, 255});
    }
}

static void quest_label(Renderer *r, const Viewport *v, const char *text, int x, int y) {
    int width = 0;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE - 14;
    int right = v->tiles_x * TILE_SIZE - width - 4;
    if (px > right) {
        px = right;
    }
    if (px < 4) {
        px = 4;
    }
    renderer_draw_text(r, text, px + 1, py + 1, (SDL_Color){20, 23, 26, 255}, r->font_tiny);
    renderer_draw_text(r, text, px, py, (SDL_Color){226, 198, 137, 255}, r->font_tiny);
}

static void quest_prop(Renderer *r, const GameState *g, const Viewport *v, int x, int y, int sx, int sy) {
    // Dropped gear and a return portal keep their visible tile and underlay.
    if (!map_is_explored(&g->map, x, y) || !viewport_is_visible(v, x, y) ||
        g->map.tiles[y][x] == TILE_ITEM || g->map.tiles[y][x] == TILE_PORTAL) {
        return;
    }
    SDL_Rect source = {sx, sy, 24, 24};
    SDL_Rect dest = {viewport_to_screen_x(v, x) * TILE_SIZE, viewport_to_screen_y(v, y) * TILE_SIZE, 24, 24};
    SDL_RenderCopy(r->sdl, r->war_machine_texture, &source, &dest);
}

static void draw_weapons_fall(Renderer *r, const GameState *g, const Viewport *v) {
    for (int n = 0; n < combat_feedback_count(); n++) {
        const CombatFeedbackEvent *e = combat_feedback_get(n);
        Uint32 age = SDL_GetTicks() - e->created_at;
        if (e->kind != FEEDBACK_WEAPONS_DUMP || e->location != g->location || e->level != g->level ||
            age >= COMBAT_FEEDBACK_MS || !viewport_is_visible(v, e->x, e->y)) {
            continue;
        }
        int px = viewport_to_screen_x(v, e->x) * TILE_SIZE;
        int py = viewport_to_screen_y(v, e->y) * TILE_SIZE;
        SDL_Rect source = {120, 24, 24, 24};
        if (age < 480) {
            for (int i = 0; i < 2; i++) {
                int fall = (int)age - i * 70;
                if (fall < 0) {
                    continue;
                }
                SDL_Rect dest = {px + i * 2 - 3, py - 1 + fall * fall / 4500, 24, 24};
                SDL_RenderCopyEx(r->sdl, r->war_machine_texture, &source, &dest, i ? fall / 5.0 : -fall / 6.0, NULL, SDL_FLIP_NONE);
            }
        } else {
            int spread = (int)(age - 480) / 25;
            box(r, px - spread, py + 50, 24 + spread * 2, 2, (SDL_Color){128, 190, 185, 255});
            box(r, px + 3, py + 45 - spread / 2, 2, 4, (SDL_Color){178, 211, 192, 255});
            box(r, px + 19, py + 46 - spread, 2, 3, (SDL_Color){178, 211, 192, 255});
        }
    }
}

static void draw_war_machine(Renderer *r, const GameState *g, const Viewport *v) {
    if (!r->war_machine_texture) {
        return;
    }
    int x;
    int y;
    if (g->level == 2) {
        x = WAR_ORDERS_X;
        y = WAR_ORDERS_Y;
        quest_prop(r, g, v, x, y, 0, 0);
        if (viewport_is_visible(v, x, y) && map_is_explored(&g->map, x, y)) {
            quest_label(r, v, "SHIPMENT ORDERS", x, y);
        }
    } else if (g->level == 4 && viewport_is_visible(v, WAR_WINCH_X, WAR_WINCH_Y) && map_is_explored(&g->map, WAR_WINCH_X, WAR_WINCH_Y)) {
        SDL_Rect source = {g->war_machine_progress & WAR_SABOTAGE ? 72 : 24, 0, 48, 48};
        SDL_Rect dest = {viewport_to_screen_x(v, 61) * TILE_SIZE, viewport_to_screen_y(v, 22) * TILE_SIZE, 48, 48};
        SDL_RenderCopy(r->sdl, r->war_machine_texture, &source, &dest);
        quest_prop(r, g, v, WAR_WINCH_X, WAR_WINCH_Y, 120, 0);
        quest_label(r, v, g->war_machine_progress & WAR_SABOTAGE ? "SHIPMENT DESTROYED" : "WEAPONS WINCH", WAR_WINCH_X, 22);
        draw_weapons_fall(r, g, v);
    } else if (g->level == 5) {
        x = WAR_SEAL_X;
        y = WAR_SEAL_Y;
        quest_prop(r, g, v, x, y, 144, g->war_machine_progress & WAR_SEAL ? 24 : 0);
        if (viewport_is_visible(v, x, y) && map_is_explored(&g->map, x, y)) {
            quest_label(r, v, g->war_machine_progress & WAR_SEAL ? "EMPTY COMMAND COFFER" : "COMMAND COFFER", x, y);
        }
    }
}

void ironfang_draw_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_IRONFANG) {
        return;
    }
    draw_war_machine(r, g, v);
    const IronfangStage *state = &g->ironfang[g->level - 1];
    int px = viewport_to_screen_x(v, state->cart_x) * TILE_SIZE;
    int py = viewport_to_screen_y(v, state->cart_y) * TILE_SIZE;
    box(r, px + 2, py + 10, 20, 10, (SDL_Color){71, 58, 43, 255});
    box(r, px + 3, py + 9, 18, 3, (SDL_Color){169, 138, 87, 255});
    box(r, px + 5, py + 13, 14, 4, (SDL_Color){106, 85, 53, 255});
    for (int n = 0; n < 2; n++) {
        box(r, px + 4 + n * 12, py + 20, 5, 4, (SDL_Color){25, 27, 28, 255});
        box(r, px + 5 + n * 12, py + 21, 2, 2, (SDL_Color){151, 155, 140, 255});
    }
    SDL_BlendMode previous;
    SDL_GetRenderDrawBlendMode(r->sdl, &previous);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y && y < IRON_H; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x && x < IRON_W; x++) {
            int turns = ironfang_ore_marks(g, x, y) ? state->ore_fuse : 0;
            for (int n = 0; n < g->enemy_count; n++) {
                if (ironfang_attack_marks(g, &g->enemies[n], x, y)) {
                    turns = g->enemies[n].move_timer;
                }
            }
            if (turns) {
                int xx = viewport_to_screen_x(v, x) * TILE_SIZE;
                int yy = viewport_to_screen_y(v, y) * TILE_SIZE;
                box(r, xx + 1, yy + 1, 22, 22, (SDL_Color){221, 133, 48, 90});
                char number[4];
                snprintf(number, sizeof(number), "%d", turns > 2 ? 2 : turns);
                renderer_draw_text(r, number, xx + 8, yy + 6, (SDL_Color){255, 218, 137, 255}, r->font_tiny);
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, previous);
}

void ironfang_draw_hint(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_IRONFANG) {
        return;
    }
    const IronfangStage *state = &g->ironfang[g->level - 1];
    const char *hint = ironfang_hint(g);
    char text[160];
    char bound[80] = "";
    if (hint) {
        // Match configured controls, including remapped Interact and movement keys.
        snprintf(bound, sizeof(bound), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), hint + 3);
    }
    snprintf(text, sizeof(text), "%s | TRACK %s%s%s", ironfang_stage_name(g->level), state->route ? "NORTH" : "EAST", hint ? " | " : "", bound);
    box(r, 52, 8, r->screen_w - INFO_PANEL_W - 60, 24, (SDL_Color){25, 29, 29, 255});
    renderer_draw_text(r, text, 60, 13, (SDL_Color){226, 198, 137, 255}, r->font_tiny);
}
