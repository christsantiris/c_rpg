#include "elderwood_renderer.h"
#include "sprites.h"
#include "../game/elderwood.h"
#include "../game/heartseed.h"
#include <stdio.h>
#include <stdlib.h>

void draw_elderwood_tile(Renderer *r, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_ELDERWOOD_WALL) {
        draw_elderwood_wall(r, sx, sy, x, y);
        return;
    }
    draw_elderwood_floor(r, sx, sy, x, y);
    SDL_Rect cell = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    if (tile == TILE_ELDERWOOD_LOG) {
        SDL_Color shades[] = {{35, 31, 23, 255}, {70, 57, 31, 255}, {116, 91, 44, 255}, {163, 129, 68, 255}};
        for (int n = 0; n < 4; n++) {
            SDL_SetRenderDrawColor(r->sdl, shades[n].r, shades[n].g, shades[n].b, 255);
            SDL_Rect log = {cell.x + 2 + n, cell.y + 7 + n, 21 - n * 2, 11 - n * 2};
            SDL_RenderFillRect(r->sdl, &log);
        }
        SDL_SetRenderDrawColor(r->sdl, 43, 36, 24, 255);
        SDL_RenderDrawLine(r->sdl, cell.x + 5, cell.y + 13, cell.x + 19, cell.y + 13);
    } else if (tile == TILE_ELDERWOOD_BRAMBLE) {
        for (int n = 0; n < 5; n++) {
            SDL_SetRenderDrawColor(r->sdl, 82 + n * 8, 102 + n * 6, 40, 255);
            SDL_RenderDrawLine(r->sdl, cell.x + 2, cell.y + n * 4 + 2, cell.x + 21, cell.y + 21 - n * 3);
            SDL_RenderDrawLine(r->sdl, cell.x + n * 4 + 2, cell.y + 3, cell.x + 21 - n * 3, cell.y + 21);
        }
    } else if (tile == TILE_ELDERWOOD_ENTRANCE || tile == TILE_ELDERWOOD_EXIT) {
        SDL_SetRenderDrawColor(r->sdl, 208, 181, 91, 255);
        int dy = tile == TILE_ELDERWOOD_EXIT ? -1 : 1;
        for (int n = 0; n < 2; n++) {
            int cy = cell.y + 9 + n * 6;
            SDL_RenderDrawLine(r->sdl, cell.x + 6, cy - dy * 3, cell.x + 12, cy + dy * 3);
            SDL_RenderDrawLine(r->sdl, cell.x + 12, cy + dy * 3, cell.x + 18, cy - dy * 3);
        }
    }
}

static void label(Renderer *r, const Viewport *v, const char *name, int x, int y) {
    int width = 0;
    TTF_SizeText(r->font_tiny, name, &width, NULL);
    renderer_draw_text(r, name, viewport_to_screen_x(v, x) * TILE_SIZE + (TILE_SIZE - width) / 2,
        viewport_to_screen_y(v, y) * TILE_SIZE - 16, (SDL_Color){226, 203, 116, 255}, r->font_tiny);
}

void draw_northern_road(Renderer *r, const GameState *g, const Viewport *v) {
    if (!northern_approach(g)) {
        return;
    }
    SDL_Rect elder = {(viewport_to_screen_x(v, NORTH_ELDER_X) - 3) * TILE_SIZE,
        (viewport_to_screen_y(v, NORTH_ELDER_Y) - 5) * TILE_SIZE, 7 * TILE_SIZE, 6 * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->elderwood_entrance_texture, NULL, &elder);
    draw_area_entrance(r, TOWN_EXIT_CATACOMBS, viewport_to_screen_x(v, NORTH_CAT_X), viewport_to_screen_y(v, NORTH_CAT_Y), 6);
    label(r, v, "ELDERWOOD FOREST", NORTH_ELDER_X, NORTH_ELDER_Y - 5);
    label(r, v, "ROYAL CATACOMBS", NORTH_CAT_X, NORTH_CAT_Y - 5);
    draw_town_road_sign(r, viewport_to_screen_x(v, NORTH_ROAD_X + 2), viewport_to_screen_y(v, 44), "S: OAKHAVEN");
    draw_town_road_sign(r, viewport_to_screen_x(v, NORTH_ROAD_X + 2), viewport_to_screen_y(v, 4), "N: CROWN ROAD NORTH");
    if (!g->elderwood_defeated && !g->debug_passages_unlocked) {
        draw_town_road_sign(r, viewport_to_screen_x(v, NORTH_ROAD_X - 8), viewport_to_screen_y(v, 34), "ELDERTHORN'S ROOTS\nBLOCK THE NORTH ROAD");
    } else if (!(g->defeated_bosses & (1 << LOCATION_CATACOMBS)) && !g->debug_passages_unlocked) {
        draw_town_road_sign(r, viewport_to_screen_x(v, NORTH_ROAD_X + 2), viewport_to_screen_y(v, 8), "CLEAR THE CATACOMBS\nTO REACH THE CASTLE");
    }
}

void draw_elderwood_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_ELDERWOOD) {
        return;
    }
    SDL_BlendMode old;
    SDL_GetRenderDrawBlendMode(r->sdl, &old);
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (elderwood_marks(enemy, x, y)) {
                    SDL_Rect cell = {viewport_to_screen_x(v, x) * TILE_SIZE, viewport_to_screen_y(v, y) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    SDL_SetRenderDrawColor(r->sdl, 235, 137, 38, 110);
                    SDL_RenderFillRect(r->sdl, &cell);
                    SDL_SetRenderDrawColor(r->sdl, 249, 195, 74, 230);
                    SDL_RenderDrawRect(r->sdl, &cell);
                }
            }
        }
        if (enemy->active && enemy->type == ENEMY_ELDERTHORN && viewport_is_visible(v, enemy->x, enemy->y)) {
            int x = viewport_to_screen_x(v, enemy->x) * TILE_SIZE - 20;
            int y = viewport_to_screen_y(v, enemy->y) * TILE_SIZE - 46;
            SDL_Rect bar = {x, y, 64, 5};
            SDL_SetRenderDrawColor(r->sdl, 42, 32, 24, 255);
            SDL_RenderFillRect(r->sdl, &bar);
            bar.w = enemy->max_hp > 0 ? 64 * enemy->hp / enemy->max_hp : 0;
            SDL_SetRenderDrawColor(r->sdl, 209, 159, 58, 255);
            SDL_RenderFillRect(r->sdl, &bar);
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, old);
}

static void ink(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &rect);
}

static void binding(Renderer *r, int x, int y, int cut) {
    ink(r, x + 8, y + 18, 12, 4, (SDL_Color){28, 43, 22, 255});
    ink(r, x + 10, y + (cut ? 15 : 3), 6, cut ? 7 : 19, (SDL_Color){35, 26, 22, 255});
    ink(r, x + 11, y + (cut ? 15 : 4), 3, cut ? 5 : 17, (SDL_Color){132, 91, 46, 255});
    ink(r, x + 11, y + (cut ? 15 : 4), 1, cut ? 5 : 17, (SDL_Color){197, 153, 77, 255});
    if (!cut) {
        for (int n = 0; n < 3; n++) {
            ink(r, x + 7, y + 7 + n * 4, 11, 2, (SDL_Color){55, 31, 70, 255});
            ink(r, x + 9, y + 7 + n * 4, 7, 1, (SDL_Color){181, 107, 198, 255});
        }
    } else {
        ink(r, x + 5, y + 20, 4, 1, (SDL_Color){126, 81, 131, 255});
        ink(r, x + 17, y + 17, 3, 2, (SDL_Color){107, 70, 110, 255});
    }
}

static void root_arch(Renderer *r, int sx, int sy) {
    const int points[][2] = {{3, 60}, {7, 37}, {17, 16}, {32, 6}, {47, 16}, {57, 37}, {61, 60}};
    const SDL_Color bark[] = {{31, 39, 19, 255}, {85, 78, 31, 255}, {144, 120, 48, 255}, {189, 162, 73, 255}};
    int px = sx * TILE_SIZE - 20;
    int py = sy * TILE_SIZE - 67;
    for (int shade = 0; shade < 4; shade++) {
        SDL_SetRenderDrawColor(r->sdl, bark[shade].r, bark[shade].g, bark[shade].b, 255);
        for (int width = -3 + shade; width <= 3 - shade; width++) {
            for (int i = 0; i < 6; i++) {
                SDL_RenderDrawLine(r->sdl, px + points[i][0] + width, py + points[i][1],
                    px + points[i + 1][0] + width, py + points[i + 1][1]);
            }
        }
    }
    for (int n = 0; n < 7; n++) {
        ink(r, px + 11 + n * 6, py + 8 + (n % 3) * 2, 8, 4, (SDL_Color){58, 86, 29, 255});
        ink(r, px + 12 + n * 6, py + 8 + (n % 3) * 2, 5, 2, (SDL_Color){117, 143, 49, 255});
    }
}

void draw_heartseed_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location == LOCATION_TOWN) {
        label(r, v, "FORESTER TAMSIN", TAMSIN_X - 2, TAMSIN_Y - 1);
        return;
    }
    if (g->location != LOCATION_ELDERWOOD) {
        return;
    }
    if (g->level == 2 && (g->heartseed_sites & 1)) {
        int x = viewport_to_screen_x(v, HEARTSEED_ORDERS_X) * TILE_SIZE;
        int y = viewport_to_screen_y(v, HEARTSEED_ORDERS_Y) * TILE_SIZE;
        ink(r, x + 3, y + 17, 19, 5, (SDL_Color){43, 42, 25, 255});
        ink(r, x + 5, y + 10, 15, 10, (SDL_Color){115, 81, 41, 255});
        ink(r, x + 5, y + 10, 15, 2, (SDL_Color){172, 130, 70, 255});
        ink(r, x + 8, y + 7, 9, 9, (SDL_Color){221, 202, 146, 255});
        for (int n = 0; n < 3; n++) {
            ink(r, x + 10, y + 9 + n * 2, 5, 1, (SDL_Color){102, 72, 47, 255});
        }
        label(r, v, "DARK ELF ORDERS", HEARTSEED_ORDERS_X, HEARTSEED_ORDERS_Y - 1);
    } else if (g->level == 4 && (g->heartseed_sites & 2)) {
        int sx = viewport_to_screen_x(v, HEARTSEED_X);
        int sy = viewport_to_screen_y(v, HEARTSEED_Y);
        root_arch(r, sx, sy);
        label(r, v, "GOLDEN-ROOT GROVE", HEARTSEED_X, HEARTSEED_Y - 5);
        for (int n = 0; n < g->enemy_count; n++) {
            const Enemy *e = &g->enemies[n];
            if (!e->active || !heartseed_is_guardian(g, e)) {
                continue;
            }
            label(r, v, "BOUND TREEFOLK", e->x, e->y - 1);
            int tx = viewport_to_screen_x(v, e->x) * TILE_SIZE + 12;
            int ty = viewport_to_screen_y(v, e->y) * TILE_SIZE + 20;
            for (int i = 0; i < 3; i++) {
                if (g->heartseed_bindings & (1 << i)) {
                    continue;
                }
                int x = viewport_to_screen_x(v, heartseed_stakes[i][0]) * TILE_SIZE + 12;
                int y = viewport_to_screen_y(v, heartseed_stakes[i][1]) * TILE_SIZE + 20;
                for (int step = 0; step < 6; step++) {
                    int ax = x + (tx - x) * step / 6;
                    int ay = y + (ty - y) * step / 6 + (step % 2) * 3;
                    int bx = x + (tx - x) * (step + 1) / 6;
                    int by = y + (ty - y) * (step + 1) / 6 + ((step + 1) % 2) * 3;
                    SDL_SetRenderDrawColor(r->sdl, 44, 29, 46, 255);
                    SDL_RenderDrawLine(r->sdl, ax, ay + 1, bx, by + 1);
                    SDL_SetRenderDrawColor(r->sdl, 147, 85, 161, 255);
                    SDL_RenderDrawLine(r->sdl, ax, ay, bx, by);
                }
            }
        }
        for (int i = 0; i < 3; i++) {
            int x = viewport_to_screen_x(v, heartseed_stakes[i][0]) * TILE_SIZE;
            int y = viewport_to_screen_y(v, heartseed_stakes[i][1]) * TILE_SIZE;
            int cut = !!(g->heartseed_bindings & (1 << i));
            binding(r, x, y, cut);
            if (!cut) {
                label(r, v, "BINDING", heartseed_stakes[i][0], heartseed_stakes[i][1] - 1);
            }
        }
        int x = sx * TILE_SIZE;
        int y = sy * TILE_SIZE;
        ink(r, x + 3, y + 17, 19, 5, (SDL_Color){38, 47, 29, 255});
        ink(r, x + 5, y + 14, 15, 6, (SDL_Color){95, 107, 62, 255});
        ink(r, x + 7, y + 13, 11, 2, (SDL_Color){162, 163, 106, 255});
        if (!(g->heartseed_progress & HEARTSEED_TAKEN)) {
            ink(r, x + 10, y + 7, 6, 8, (SDL_Color){104, 78, 29, 255});
            ink(r, x + 11, y + 8, 4, 5, (SDL_Color){233, 184, 66, 255});
            ink(r, x + 12, y + 8, 2, 3, (SDL_Color){255, 234, 140, 255});
            ink(r, x + 14, y + 5, 4, 3, (SDL_Color){147, 173, 67, 255});
        }
    }
    const char *hint = heartseed_hint(g);
    if (hint || (g->level == 4 && abs(g->player.x - HEARTSEED_X) + abs(g->player.y - HEARTSEED_Y) < 14)) {
        char text[110];
        if (g->level == 4) {
            int cut = !!(g->heartseed_bindings & 1) + !!(g->heartseed_bindings & 2) + !!(g->heartseed_bindings & 4);
            snprintf(text, sizeof(text), "BINDINGS: %d/3 | %s", cut, hint ? hint : "CUT THE VIOLET STAKES WITH A");
        } else {
            snprintf(text, sizeof(text), "%s", hint);
        }
        ink(r, 48, 8, 640, 28, (SDL_Color){24, 40, 26, 255});
        renderer_draw_text(r, text, 56, 16, (SDL_Color){226, 203, 116, 255}, r->font_tiny);
    }
}
