#include "pirate_renderer.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"

static unsigned int water_seed(int x, int y) {
    unsigned int seed = (unsigned int)x * 73856093u ^ (unsigned int)y * 19349663u;
    return seed ^ (seed >> 13);
}

static void draw_water(Renderer *r, int sx, int sy, int x, int y) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    SDL_Rect cell = {px, py, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(r->sdl, 5, 35, 50, 255);
    SDL_RenderFillRect(r->sdl, &cell);
    unsigned int seed = water_seed(x, y);
    unsigned int frame = SDL_GetTicks() / 120u;
    for (int i = 0; i < 2; i++) {
        unsigned int phase = (frame + (seed >> (i * 8))) % 16u;
        int swell = phase < 8 ? phase : 15 - phase;
        int wx = px + 2 + (seed >> (i * 5) & 7u) + swell / 2;
        int wy = py + 3 + i * 11 + (seed >> 16 & 3u) + swell / 4;
        int length = 4 + (seed >> (i * 3 + 4) & 3u);
        SDL_SetRenderDrawColor(r->sdl, 26 + swell * 3, 69 + swell * 5, 86 + swell * 5, 255);
        SDL_RenderDrawLine(r->sdl, wx, wy, wx + length, wy);
        SDL_RenderDrawLine(r->sdl, wx + 2, wy - 1, wx + length - 1, wy - 1);
        if (swell >= 5) {
            SDL_SetRenderDrawColor(r->sdl, 70, 137, 151, 255);
            SDL_RenderDrawPoint(r->sdl, wx + length / 2, wy - 1);
        }
    }
}

static int sea_at(const Map *map, int x, int y) {
    return x >= 0 && x < PIRATE_W && y >= 0 && y < PIRATE_H && map->tiles[y][x] == TILE_PIRATE_SEA;
}

static int sand_at(const Map *map, int x, int y) {
    if (x < 0 || x >= PIRATE_W || y < 0 || y >= PIRATE_H) {
        return 0;
    }
    TileType tile = map->tiles[y][x];
    return tile == TILE_PIRATE_SAND || tile == TILE_PIRATE_CHEST || tile == TILE_PIRATE_CRATE;
}

// Round both beach headlands and the inside corners of water inlets at pixel scale.
static void round_shore(Renderer *r, const Map *map, int sx, int sy, int x, int y, int sea) {
    const int curve[12] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0};
    const int dx[4] = {-1, 1, 1, -1};
    const int dy[4] = {-1, -1, 1, 1};
    for (int corner = 0; corner < 4; corner++) {
        int round = sea ? sand_at(map, x + dx[corner], y) && sand_at(map, x, y + dy[corner]) :
            sea_at(map, x + dx[corner], y) && sea_at(map, x, y + dy[corner]);
        if (!round) {
            continue;
        }
        int cx = sx * TILE_SIZE + (dx[corner] < 0 ? 0 : TILE_SIZE - 1);
        int cy = sy * TILE_SIZE + (dy[corner] < 0 ? 0 : TILE_SIZE - 1);
        for (int row = 0; row < 12; row++) {
            int width = curve[row];
            if (!width) {
                continue;
            }
            SDL_SetRenderDrawColor(r->sdl, sea ? 151 : 5, sea ? 144 : 35, sea ? 99 : 50, 255);
            SDL_RenderDrawLine(r->sdl, cx, cy - dy[corner] * row, cx - dx[corner] * (width - 1), cy - dy[corner] * row);
        }
    }
}

void draw_pirate_tile(Renderer *r, const Map *map, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_PIRATE_QUEST_CHEST) {
        draw_pirate_tile(r, map, sx, sy, x, y, TILE_PIRATE_DECK);
        draw_world_prop(r, sx, sy, PROP_SILVER_CHEST, (SDL_Color){177, 205, 218, 255});
        return;
    }
    if (tile == TILE_PIRATE_SEA) {
        draw_water(r, sx, sy, x, y);
        round_shore(r, map, sx, sy, x, y, 1);
        return;
    }
    if (tile == TILE_PIRATE_CLIFF) {
        draw_mountain_wall(r, sx, sy, x, y);
        return;
    }
    if (tile == TILE_PIRATE_ENTRANCE || tile == TILE_PIRATE_EXIT) {
        draw_coast_edge(r, sx, sy, x, y, tile == TILE_PIRATE_EXIT);
        return;
    }
    SDL_Rect cell = {sx * TILE_SIZE, sy * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    if (tile == TILE_PIRATE_DECK || tile == TILE_PIRATE_RAIL) {
        SDL_SetRenderDrawColor(r->sdl, 113, 76, 43, 255);
        SDL_RenderFillRect(r->sdl, &cell);
        for (int n = 0; n < 4; n++) {
            int yy = cell.y + n * 6;
            SDL_SetRenderDrawColor(r->sdl, 54, 39, 29, 255);
            SDL_RenderDrawLine(r->sdl, cell.x, yy, cell.x + 23, yy);
            int seam = cell.x + (n % 2 ? 6 : 18);
            SDL_RenderDrawLine(r->sdl, seam, yy, seam, yy + 5);
            SDL_SetRenderDrawColor(r->sdl, 165, 116, 62, 255);
            SDL_RenderDrawLine(r->sdl, cell.x + 2, yy + 2, cell.x + 20, yy + 2);
            SDL_SetRenderDrawColor(r->sdl, 35, 34, 29, 255);
            SDL_RenderDrawPoint(r->sdl, seam + 2, yy + 4);
        }
        if (tile == TILE_PIRATE_RAIL) {
            SDL_SetRenderDrawColor(r->sdl, 40, 31, 24, 255);
            SDL_Rect post = {cell.x + 2, cell.y, 4, 24};
            SDL_RenderFillRect(r->sdl, &post);
            post.x += 16;
            SDL_RenderFillRect(r->sdl, &post);
            SDL_Rect rail = {cell.x, cell.y + 5, 24, 5};
            SDL_RenderFillRect(r->sdl, &rail);
            SDL_SetRenderDrawColor(r->sdl, 191, 137, 69, 255);
            SDL_RenderDrawLine(r->sdl, cell.x, cell.y + 5, cell.x + 23, cell.y + 5);
        }
        return;
    }
    draw_coast_sand(r, sx, sy, x, y, 0);
    if (tile == TILE_PIRATE_SAND) {
        round_shore(r, map, sx, sy, x, y, 0);
    }
    if (tile == TILE_PIRATE_PALISADE) {
        for (int n = 0; n < 4; n++) {
            int xx = cell.x + n * 6;
            SDL_SetRenderDrawColor(r->sdl, 39, 31, 25, 255);
            SDL_Rect post = {xx, cell.y + 4, 6, 20};
            SDL_RenderFillRect(r->sdl, &post);
            SDL_SetRenderDrawColor(r->sdl, 112 + n % 2 * 12, 79, 44, 255);
            post.x++; post.w = 4; post.y += 2; post.h -= 2;
            SDL_RenderFillRect(r->sdl, &post);
            SDL_RenderDrawLine(r->sdl, xx + 1, cell.y + 4, xx + 3, cell.y);
            SDL_RenderDrawLine(r->sdl, xx + 3, cell.y, xx + 4, cell.y + 4);
        }
    } else if (tile == TILE_PIRATE_CRATE) {
        SDL_Rect crate = {cell.x + 2, cell.y + 2, 20, 20};
        SDL_SetRenderDrawColor(r->sdl, 51, 37, 26, 255);
        SDL_RenderFillRect(r->sdl, &crate);
        crate.x += 2; crate.y += 2; crate.w -= 4; crate.h -= 4;
        SDL_SetRenderDrawColor(r->sdl, 153, 103, 49, 255);
        SDL_RenderFillRect(r->sdl, &crate);
        SDL_SetRenderDrawColor(r->sdl, 82, 59, 35, 255);
        SDL_RenderDrawLine(r->sdl, crate.x, crate.y, crate.x + 15, crate.y + 15);
        SDL_RenderDrawLine(r->sdl, crate.x + 15, crate.y, crate.x, crate.y + 15);
    } else if (tile == TILE_PIRATE_CHEST) {
        draw_world_prop(r, sx, sy, PROP_CHEST, (SDL_Color){222, 177, 69, 255});
    }
}

void draw_pirate_status(Renderer *r, const GameState *g, const Viewport *v) {
    int width = r->screen_w - INFO_PANEL_W;
    if (g->location != LOCATION_PIRATE_COAST || width < 328) {
        return;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *e = &g->enemies[i];
        if (!e->active || e->type != ENEMY_CAPTAIN_VOSS || !viewport_is_visible(v, e->x, e->y)) {
            continue;
        }
        int x = width - 320;
        SDL_Rect panel = {x, 8, 312, 100};
        SDL_SetRenderDrawColor(r->sdl, 26, 27, 31, 255);
        SDL_RenderFillRect(r->sdl, &panel);
        SDL_SetRenderDrawColor(r->sdl, 196, 155, 72, 255);
        SDL_RenderDrawRect(r->sdl, &panel);
        renderer_draw_text(r, "CAPTAIN VOSS", x + 12, 17, (SDL_Color){240, 205, 122, 255}, r->font_small);
        SDL_Rect bar = {x + 12, 39, 288, 8};
        SDL_SetRenderDrawColor(r->sdl, 67, 40, 35, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        bar.w = e->max_hp ? 288 * e->hp / e->max_hp : 0;
        SDL_SetRenderDrawColor(r->sdl, 191, 66, 47, 255);
        SDL_RenderFillRect(r->sdl, &bar);
        char status[64];
        if (e->frozen_turns) {
            SDL_snprintf(status, sizeof(status), "FROZEN: %d TURNS", e->frozen_turns);
        } else if (e->attack_phase) {
            SDL_snprintf(status, sizeof(status), "%s: %d TURNS", e->boss_attack == PIRATE_SABER ? "SABER SWEEP" : "BLADE WAVE", e->move_timer);
        } else {
            SDL_snprintf(status, sizeof(status), "%s", e->move_timer < 0 ? "RECOVERING: STRIKE NOW" : "WATCH FOR HIS NEXT ORDER");
        }
        renderer_draw_text(r, status, x + 12, 57, (SDL_Color){236, 225, 200, 255}, r->font_tiny);
        renderer_draw_text(r, "CLEAR THE MARKED DECK BEFORE IMPACT", x + 12, 79, (SDL_Color){236, 225, 200, 255}, r->font_tiny);
        break;
    }
}

void draw_pirate_warnings(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_PIRATE_COAST) {
        return;
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < g->enemy_count; i++) {
        const Enemy *enemy = &g->enemies[i];
        for (int y = v->cam_y; y < v->cam_y + v->tiles_y; y++) {
            for (int x = v->cam_x; x < v->cam_x + v->tiles_x; x++) {
                if (pirate_marks(g, enemy, x, y)) {
                    SDL_Rect rect = {viewport_to_screen_x(v, x) * TILE_SIZE, viewport_to_screen_y(v, y) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    SDL_SetRenderDrawColor(r->sdl, 238, 78, 34, 110);
                    SDL_RenderFillRect(r->sdl, &rect);
                    SDL_SetRenderDrawColor(r->sdl, 255, 204, 90, 240);
                    SDL_RenderDrawRect(r->sdl, &rect);
                    char count[8];
                    SDL_snprintf(count, sizeof(count), "%d", enemy->move_timer);
                    renderer_draw_text(r, count, rect.x + 8, rect.y + 5, (SDL_Color){255, 239, 172, 255}, r->font_tiny);
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);
}
