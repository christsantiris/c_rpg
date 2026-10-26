#include "elderwood_renderer.h"
#include "sprites.h"
#include "../game/elderwood.h"

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
