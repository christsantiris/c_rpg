#include "giants_crown_renderer.h"
#include "../game/giants_crown.h"
#include "dungeon_enemy_art.h"
#include "sprites.h"
#include "world_props.h"
#include "info_panel.h"
#include "controls_renderer.h"

static void box(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &rect);
}

void giants_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    unsigned int seed = x * 73856093u ^ y * 19349663u;
    SDL_Color grass = {63, 75, 48, 255};
    SDL_Color stone = {103, 105, 93, 255};
    SDL_Color dark = {48, 54, 51, 255};
    SDL_Color light = {151, 153, 131, 255};
    box(r, px, py, 24, 24, tile == TILE_GIANTS_SNOW ? (SDL_Color){173, 184, 178, 255} : grass);
    for (int n = 0; n < 6; n++) {
        seed = seed * 1664525u + 1013904223u;
        box(r, px + seed % 21, py + (seed >> 8) % 23, 2, 1,
            tile == TILE_GIANTS_SNOW ? (SDL_Color){204, 211, 197, 255} : (SDL_Color){89, 98, 62, 255});
    }
    if (tile == TILE_GIANTS_WALL || tile == TILE_GIANTS_CRACK) {
        if (tile == TILE_GIANTS_WALL && (g->level == 1 || g->level == 3) &&
            !(g->level == 3 && y == 22 && x >= 22 && x <= 42)) {
            // Outdoor terraces have natural rock faces, distinct from ruined halls.
            box(r, px, py, 24, 24, (SDL_Color){34, 43, 38, 255});
            for (int n = 0; n < 12; n++) {
                seed = seed * 1664525u + 1013904223u;
                box(r, px + seed % 22, py + (seed >> 8) % 22, 2, 1, (SDL_Color){49, 60, 49, 255});
            }
            if (y + 1 < GIANTS_H && map_is_walkable(&g->map, x, y + 1)) {
                for (int row = 6; row < 22; row++) {
                    box(r, px, py + row, 24, 1, (SDL_Color){91 - row, 99 - row, 81 - row, 255});
                }
                box(r, px, py + 5, 24, 2, (SDL_Color){109, 120, 87, 255});
                box(r, px, py + 22, 24, 2, dark);
                SDL_SetRenderDrawColor(r->sdl, 40, 48, 40, 255);
                SDL_Point fracture[] = {{px + 8,py + 8},{px + 6,py + 15},{px + 9,py + 21}};
                SDL_RenderDrawLines(r->sdl, fracture, 3);
            }
            return;
        }
        // Large weathered blocks; connected walls leave a shaded southern face.
        stone = (SDL_Color){61, 70, 64, 255};
        light = (SDL_Color){101, 113, 96, 255};
        box(r, px, py, 24, 24, dark);
        for (int row = 0; row < 3; row++) {
            int offset = (row + y) % 2 * 7;
            for (int col = -1; col < 3; col++) {
                int bx = col * 15 + offset;
                int left = bx < 0 ? 0 : bx;
                int right = bx + 14 > 24 ? 24 : bx + 14;
                if (right > left) {
                    box(r, px + left, py + row * 8, right - left, 7, stone);
                    box(r, px + left, py + row * 8, right - left, 1, light);
                }
            }
        }
        if (y + 1 < GIANTS_H && map_is_walkable(&g->map, x, y + 1)) {
            box(r, px, py + 19, 24, 5, dark);
            box(r, px + 1, py + 18, 22, 2, light);
        }
        if ((x + y * 7) % 5 == 0) {
            box(r, px + 3, py + 14, 8, 2, grass);
            box(r, px + 5, py + 16, 3, 4, grass);
        }
        if (tile == TILE_GIANTS_CRACK) {
            SDL_SetRenderDrawColor(r->sdl, 224, 177, 86, 255);
            SDL_Point points[] = {{px + 14,py + 1},{px + 8,py + 10},{px + 15,py + 14},{px + 9,py + 23}};
            SDL_RenderDrawLines(r->sdl, points, 4);
        }
        return;
    }
    if (tile == TILE_GIANTS_PAVING || tile == TILE_GIANTS_ENTRANCE || tile == TILE_GIANTS_EXIT) {
        for (int row = 0; row < 2; row++) {
            box(r, px + (row ? 2 : 0), py + row * 12, 21, 10, (SDL_Color){99, 103, 86, 255});
            box(r, px + (row ? 3 : 1), py + row * 12, 19, 1, (SDL_Color){128, 133, 109, 255});
        }
    }
    if (tile == TILE_GIANTS_COLUMN) {
        box(r, px + 3, py + 19, 20, 4, dark);
        box(r, px + 5, py + 3, 14, 18, stone);
        box(r, px + 6, py + 4, 3, 15, light);
        box(r, px + 17, py + 5, 3, 16, dark);
        box(r, px + 3, py + 1, 18, 4, light);
        box(r, px + 3, py + 19, 18, 3, stone);
    } else if (tile == TILE_GIANTS_CACHE || tile == TILE_GIANTS_EMPTY) {
        box(r, px + 4, py + 10, 17, 11, dark);
        box(r, px + 5, py + 8, 15, 11, stone);
        box(r, px + 5, py + 8, 15, 2, light);
        box(r, px + 11, py + 12, 3, 4, tile == TILE_GIANTS_CACHE ? (SDL_Color){224, 177, 86, 255} : dark);
    } else if (tile == TILE_GIANTS_ENTRANCE || tile == TILE_GIANTS_EXIT) {
        int sign = tile == TILE_GIANTS_EXIT ? -1 : 1;
        SDL_SetRenderDrawColor(r->sdl, 227, 218, 171, 255);
        SDL_RenderDrawLine(r->sdl, px + 6, py + 12, px + 12, py + 12 + sign * 5);
        SDL_RenderDrawLine(r->sdl, px + 18, py + 12, px + 12, py + 12 + sign * 5);
    }
}

void giants_draw_details(Renderer *r, const GameState *g, const Viewport *v) {
    if (g->location != LOCATION_GIANTS_CROWN) {
        return;
    }
    for (int n = 0; n < g->enemy_count; n++) {
        const Enemy *enemy = &g->enemies[n];
        for (int y = enemy->attack_target_y - 2; y <= enemy->attack_target_y + 2; y++) {
            for (int x = enemy->attack_target_x - 2; x <= enemy->attack_target_x + 2; x++) {
                if (!viewport_is_visible(v, x, y) || !map_is_explored(&g->map, x, y) || !giants_attack_marks(enemy, x, y)) {
                    continue;
                }
                int px = viewport_to_screen_x(v, x) * TILE_SIZE;
                int py = viewport_to_screen_y(v, y) * TILE_SIZE;
                SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_BLEND);
                box(r, px + 1, py + 1, 22, 22, (SDL_Color){232, 155, 46, 95});
                SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);
                char count[4];
                SDL_snprintf(count, sizeof(count), "%d", enemy->move_timer);
                renderer_draw_text(r, count, px + 8, py + 6, (SDL_Color){255, 224, 143, 255}, r->font_tiny);
            }
        }
    }
}

void giants_draw_hint(Renderer *r, const GameState *g) {
    if (g->location != LOCATION_GIANTS_CROWN) {
        return;
    }
    const char *hint = giants_hint(g);
    char text[180];
    SDL_snprintf(text, sizeof(text), "%s%s%s", giants_stage_name(g->level), hint ? "  |  " : "", hint ? hint : "");
    char bound[100] = "";
    if (hint) {
        SDL_snprintf(bound, sizeof(bound), "%s: %s", controls_key_name(g->key_bindings[CONTROL_MOVE_LEFT]), hint + 3);
    }
    SDL_snprintf(text, sizeof(text), "%s%s%s", giants_stage_name(g->level), hint ? " | " : "", bound);
    box(r, 52, 8, r->screen_w - INFO_PANEL_W - 60, 24, (SDL_Color){31, 39, 32, 255});
    renderer_draw_text(r, text, 60, 13, (SDL_Color){224, 212, 162, 255}, r->font_tiny);
}

void giants_draw_enemy(Renderer *r, int sx, int sy, EnemyType type) {
    SDL_Color body = type == ENEMY_OGRE ? (SDL_Color){135, 143, 80, 255} :
        type == ENEMY_HILL_GIANT ? (SDL_Color){177, 137, 94, 255} : (SDL_Color){140, 151, 147, 255};
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){22, 23, 26, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + 35, body.g + 35, body.b + 35, 255};
    palette['e'] = (SDL_Color){25, 20, 19, 255};
    palette['y'] = (SDL_Color){239, 188, 63, 255};
    palette['c'] = (SDL_Color){56, 44, 34, 255};
    palette['C'] = (SDL_Color){92, 68, 44, 255};
    palette['v'] = (SDL_Color){132, 108, 70, 255};
    palette['k'] = (SDL_Color){43, 31, 27, 255};
    palette['l'] = palette['c'];
    palette['L'] = palette['C'];
    palette['K'] = palette['v'];
    palette['u'] = (SDL_Color){65, 64, 56, 255};
    palette['U'] = (SDL_Color){116, 115, 99, 255};
    palette['w'] = (SDL_Color){78, 57, 35, 255};
    palette['W'] = (SDL_Color){133, 104, 65, 255};
    palette['f'] = (SDL_Color){225, 217, 184, 255};
    palette['m'] = (SDL_Color){77, 84, 84, 255};
    palette['M'] = (SDL_Color){142, 151, 145, 255};
    palette['N'] = (SDL_Color){196, 204, 185, 255};
    palette['g'] = (SDL_Color){125, 92, 39, 255};
    palette['G'] = (SDL_Color){222, 182, 73, 255};
    // Derived from the existing native giant silhouette; terrain stays transparent.
    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 24; x++) {
            unsigned char ink = dungeon_enemy_art[DUNGEON_ART_GIANT][y][x];
            if (ink != '.' && palette[ink].a) {
                box(r, sx * 24 + x, sy * 24 + y, 1, 1, palette[ink]);
            }
        }
    }
    if (type == ENEMY_LAST_JARL) {
        for (int x = 9; x <= 17; x += 4) {
            box(r, sx * 24 + x, sy * 24, 2, 4, palette['G']);
        }
        box(r, sx * 24 + 9, sy * 24 + 3, 10, 2, palette['g']);
    }
}
