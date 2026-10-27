#include "stormwatch_renderer.h"
#include "../game/stormwatch.h"
#include "../game/stormbell.h"
#include "controls_renderer.h"
#include "world_props.h"
#include <stdio.h>

static void box(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void stormwatch_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    unsigned int seed = (unsigned int)x * 7349u + (unsigned int)y * 3929u;
    int water = tile == TILE_STORM_WATER;
    int stone = tile == TILE_STORM_STONE || tile == TILE_STORM_ENTRANCE || tile == TILE_STORM_EXIT;
    box(r, px, py, 24, 24, water ? (SDL_Color){11, 55, 67, 255} : stone ? (SDL_Color){97, 105, 98, 255} : (SDL_Color){116, 115, 76, 255});
    for (int n = 0; n < 9; n++) {
        seed = seed * 1664525u + 1013904223u;
        int xx = (seed >> 12) % 22;
        int yy = (seed >> 21) % 23;
        if (water) {
            int ebb = (SDL_GetTicks() / 180 + x + n) % 5;
            box(r, px + xx, py + yy, 2 + ebb, 1, (SDL_Color){29 + ebb * 3, 83 + ebb * 4, 96 + ebb * 4, 255});
        } else {
            box(r, px + xx, py + yy, 2 + n % 3, 1, stone ? (SDL_Color){124, 132, 119, 255} : (SDL_Color){145, 143, 99, 255});
            if (!stone && n % 3 == 0) {
                int sway = (SDL_GetTicks() / 400 + x + y) % 2;
                box(r, px + xx + sway, py + yy - 2, 1, 3, (SDL_Color){77, 96, 57, 255});
            }
        }
    }
    if (!water && !stone && y + 1 < STORM_H && g->map.tiles[y + 1][x] == TILE_STORM_WATER) {
        box(r, px, py + 19, 24, 5, (SDL_Color){65, 76, 73, 255});
        box(r, px + 2, py + 19, 19, 1, (SDL_Color){161, 165, 138, 255});
        box(r, px + 7, py + 21, 1, 3, (SDL_Color){41, 60, 62, 255});
        box(r, px + 18, py + 20, 1, 4, (SDL_Color){41, 60, 62, 255});
    }
    if (stone) {
        box(r, px, py + 11, 24, 1, (SDL_Color){73, 84, 83, 255});
        box(r, px + 12, py, 1, 11, (SDL_Color){73, 84, 83, 255});
    }
    if (water) {
        // Coast-facing foam stays within the water; no blue outline on land.
        if (y > 0 && map_is_walkable(&g->map, x, y - 1)) {
            box(r, px + 3, py + 1, 16, 1, (SDL_Color){128, 163, 151, 255});
        }
        return;
    }
    if (tile == TILE_STORM_WALL) {
        box(r, px, py, 24, 24, (SDL_Color){47, 63, 70, 255});
        for (int row = 0; row < 4; row++) {
            for (int col = -1; col < 3; col++) {
                int left = col * 12 + (row % 2) * 6;
                int from = left < 0 ? 0 : left;
                int to = left + 11 > 24 ? 24 : left + 11;
                if (to > from) {
                    box(r, px + from, py + row * 6, to - from, 5, (SDL_Color){87 + (row + col + 2) % 3 * 8, 104 + row * 2, 107 + row * 2, 255});
                    box(r, px + from, py + row * 6, to - from, 1, (SDL_Color){141, 152, 140, 255});
                }
            }
        }
    } else if (tile == TILE_STORM_SHUTTER_UP || tile == TILE_STORM_SHUTTER_DOWN) {
        int raised = tile == TILE_STORM_SHUTTER_UP;
        box(r, px, py, 24, 24, (SDL_Color){48, 40, 29, 255});
        for (int n = 0; n < 5; n++) {
            box(r, px + 1 + n * 5, py + 1, 3, 22, (SDL_Color){raised ? 109 : 150, raised ? 87 : 123, raised ? 55 : 79, 255});
        }
        box(r, px, py + (raised ? 5 : 21), 24, 3, (SDL_Color){48, 62, 67, 255});
        box(r, px, py + (raised ? 17 : 1), 24, 2, (SDL_Color){137, 149, 139, 255});
    } else if (tile == TILE_STORM_CRANK) {
        draw_world_prop(r, sx, sy, PROP_LEVER, (SDL_Color){211, 175, 84, 255});
    } else if (tile == TILE_STORM_SUPPLIES) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){198, 166, 93, 255});
    } else if (tile == TILE_STORM_EMPTY) {
        box(r, px + 5, py + 12, 14, 8, (SDL_Color){74, 60, 39, 255});
        box(r, px + 7, py + 13, 10, 4, (SDL_Color){41, 37, 30, 255});
    } else if (tile == TILE_STORM_ENTRANCE || tile == TILE_STORM_EXIT) {
        // An open masonry threshold and chevrons, integrated with the boundary.
        for (int n = 0; n < 2; n++) {
            int dy = tile == TILE_STORM_EXIT ? -1 : 1;
            SDL_SetRenderDrawColor(r->sdl, 226, 195, 115, 255);
            SDL_RenderDrawLine(r->sdl, px + 7, py + 8 + n * 7, px + 12, py + 8 + n * 7 + dy * 3);
            SDL_RenderDrawLine(r->sdl, px + 12, py + 8 + n * 7 + dy * 3, px + 17, py + 8 + n * 7);
        }
    }
}

void stormwatch_draw_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_STORMWATCH) {
        return;
    }
    SDL_BlendMode old;
    SDL_GetRenderDrawBlendMode(r->sdl, &old);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};
    const StormwatchStage *state = &g->stormwatch[g->level - 1];
    for (int y = v->cam_y; y < v->cam_y + v->tiles_y && y < STORM_H; y++) {
        for (int x = v->cam_x; x < v->cam_x + v->tiles_x && x < STORM_W; x++) {
            if (!map_is_explored(&g->map, x, y)) {
                continue;
            }
            int px = viewport_to_screen_x(v, x) * TILE_SIZE;
            int py = viewport_to_screen_y(v, y) * TILE_SIZE;
            for (int lane = 0; lane < (g->level >= 3 ? 2 : 1); lane++) {
                if (stormwatch_wind_marks(g, lane, x, y)) {
                    int dir = state->direction[lane];
                    box(r, px + 1, py + 1, 22, 22, (SDL_Color){156, 196, 202, 35});
                    SDL_SetRenderDrawColor(r->sdl, 182, 213, 204, 180);
                    int ex = px + 12 + dx[dir] * 5;
                    int ey = py + 12 + dy[dir] * 5;
                    SDL_RenderDrawLine(r->sdl, px + 12 - dx[dir] * 4, py + 12 - dy[dir] * 4, ex, ey);
                    SDL_RenderDrawLine(r->sdl, ex, ey, ex - dx[dir] * 3 - dy[dir] * 3, ey - dy[dir] * 3 + dx[dir] * 3);
                    SDL_RenderDrawLine(r->sdl, ex, ey, ex - dx[dir] * 3 + dy[dir] * 3, ey - dy[dir] * 3 - dx[dir] * 3);
                }
            }
            for (int n = 0; n < g->enemy_count; n++) {
                const Enemy *enemy = &g->enemies[n];
                if (stormwatch_attack_marks(g, enemy, x, y)) {
                    box(r, px + 2, py + 2, 20, 20, (SDL_Color){221, 144, 63, 90});
                    SDL_Rect cell = {px + 1, py + 1, 22, 22};
                    SDL_SetRenderDrawColor(r->sdl, 244, 197, 103, 230);
                    SDL_RenderDrawRect(r->sdl, &cell);
                    char turns[4];
                    snprintf(turns, sizeof(turns), "%d", enemy->move_timer > 1 ? 2 : 1);
                    renderer_draw_text(r, turns, px + 8, py + 5, (SDL_Color){255, 230, 167, 255}, r->font_tiny);
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, old);
}

void stormwatch_draw_hint(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_STORMWATCH) {
        return;
    }
    const char *hint = stormwatch_hint(g);
    char action_hint[100];
    if (hint && hint == stormbell_hint(g)) {
        snprintf(action_hint, sizeof(action_hint), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), hint);
        hint = action_hint;
    }
    const StormwatchStage *state = &g->stormwatch[g->level - 1];
    const char *directions[] = {"EAST", "SOUTH", "WEST", "NORTH"};
    char text[180];
    int lane = state->phase[0] >= 2 && state->phase[0] <= 4 ? 0 : 1;
    int warning = (lane == 0 || g->level >= 3) && (state->phase[lane] == 2 || state->phase[lane] == 3);
    char wind[48];
    int marked = stormwatch_wind_marks(g, 0, g->player.x, g->player.y) || stormwatch_wind_marks(g, 1, g->player.x, g->player.y);
    if (warning) {
        int remaining = (4 - state->phase[lane]) * STORM_WIND_STEP_MS - state->wind_elapsed_ms;
        snprintf(wind, sizeof(wind), "GUST %s: %.1fs", directions[state->direction[lane]], remaining / 1000.0);
    } else if ((lane == 0 || g->level >= 3) && state->phase[lane] == 4) {
        snprintf(wind, sizeof(wind), "GUST %s!", directions[state->direction[lane]]);
    } else {
        snprintf(wind, sizeof(wind), "WIND CALM");
    }
    snprintf(text, sizeof(text), "%s  |  %s%s%s%s", stormwatch_stage_name(g->level), wind, warning && !marked ? " / SHELTERED" : "", hint ? "  |  " : "", hint ? hint : "");
    box(r, 52, 8, r->screen_w - 260, 24, (SDL_Color){21, 32, 36, 230});
    renderer_draw_text(r, text, 60, 12, (SDL_Color){225, 210, 158, 255}, r->font_tiny);
    if (g->level == 4 && g->stormbell_phase > 0 && g->stormbell_phase < 3) {
        Enemy guards[3];
        stormbell_capture(g, guards);
        int left = g->stormbell_phase == 1 ? guards[0].active + guards[1].active : guards[2].active;
        snprintf(text, sizeof(text), "STORMBELL | WAVE %d / 2 | %s REMAINING: %d | RETREAT IS SAFE",
            g->stormbell_phase, g->stormbell_phase == 1 ? "RAIDERS" : "STORMCALLERS", left);
        box(r, 52, 32, r->screen_w - 260, 20, (SDL_Color){21, 32, 36, 230});
        renderer_draw_text(r, text, 60, 35, (SDL_Color){225, 210, 158, 255}, r->font_tiny);
    }
}

static void bell_label(Renderer *r, const Viewport *v, const char *text, int x, int y, int above) {
    int width = 0;
    TTF_SizeText(r->font_tiny, text, &width, NULL);
    int px = viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2;
    int py = viewport_to_screen_y(v, y) * TILE_SIZE - above;
    renderer_draw_text(r, text, px + 1, py + 1, (SDL_Color){21, 32, 36, 255}, r->font_tiny);
    renderer_draw_text(r, text, px, py, (SDL_Color){225, 210, 158, 255}, r->font_tiny);
}

void stormbell_draw_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location == LOCATION_DAWNMERE && g->map.tiles[SEREN_Y][SEREN_X] == TILE_NPC_SEREN && viewport_is_visible(v, SEREN_X, SEREN_Y)) {
        bell_label(r, v, "WATCHKEEPER SEREN", SEREN_X, SEREN_Y, 16);
        return;
    }
    if (g->location != LOCATION_STORMWATCH || (g->level != 3 && g->level != 4)) {
        return;
    }
    int clapper = g->level == 3;
    int x = clapper ? STORMBELL_CLAPPER_X : STORMBELL_X;
    int y = clapper ? STORMBELL_CLAPPER_Y : STORMBELL_Y;
    if (!viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y) || (clapper && (g->stormbell_progress & STORMBELL_CLAPPER))) {
        return;
    }
    SDL_Rect src = {clapper ? 48 : g->stormbell_progress & STORMBELL_FITTED ? 24 : 0, 0, 24, 40};
    SDL_Rect dst = {viewport_to_screen_x(v, x) * TILE_SIZE, viewport_to_screen_y(v, y) * TILE_SIZE - 16, 24, 40};
    if (r->stormbell_texture) {
        SDL_RenderCopy(r->sdl, r->stormbell_texture, &src, &dst);
    }
    bell_label(r, v, clapper ? "BELL CLAPPER" : g->stormbell_phase == 3 ? "RESTORED STORMBELL" : "STORMBELL", x, y, clapper ? 14 : 31);
}
