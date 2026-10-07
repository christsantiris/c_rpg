#include "castle_renderer.h"
#include "sprites.h"

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
        tile == TILE_CASTLE_TRAP_OPEN || tile == TILE_CASTLE_GATE || tile == TILE_CASTLE_GATE_OPEN;
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
    if (tile == TILE_CASTLE_FLOOR) {
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
    if (tile == TILE_CASTLE_CARPET) {
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
    if (tile == TILE_CASTLE_TABLE) {
        rect(r, px + 3, py + 3, 18, 19, (SDL_Color){58, 34, 31, 255});
        rect(r, px + 4, py + 3, 16, 15, (SDL_Color){120, 78, 50, 255});
        rect(r, px + 8, py + 5, 8, 8, (SDL_Color){179, 150, 105, 255});
        rect(r, px + 2, py + 9, 2, 5, (SDL_Color){177, 105, 62, 255});
        rect(r, px + 20, py + 9, 2, 5, (SDL_Color){177, 105, 62, 255});
    } else if (tile == TILE_CASTLE_BOOKCASE) {
        rect(r, px + 2, py + 1, 20, 22, (SDL_Color){69, 40, 32, 255});
        for (int row = 3; row < 21; row += 7) {
            for (int col = 4; col < 20; col += 4) {
                rect(r, px + col, py + row, 3, 5, (SDL_Color){90 + col * 3, 50 + row * 3, 101 + col, 255});
            }
            rect(r, px + 3, py + row + 5, 18, 1, (SDL_Color){170, 116, 58, 255});
        }
    } else if (tile == TILE_CASTLE_THRONE) {
        rect(r, px + 3, py, 18, 24, (SDL_Color){192, 143, 65, 255});
        rect(r, px + 6, py + 3, 12, 17, (SDL_Color){111, 29, 48, 255});
        rect(r, px + 2, py + 14, 20, 4, (SDL_Color){224, 177, 82, 255});
        rect(r, px + 6, py + 1, 3, 3, (SDL_Color){36, 26, 43, 255});
        rect(r, px + 15, py + 1, 3, 3, (SDL_Color){36, 26, 43, 255});
    } else if (tile == TILE_CASTLE_PILLAR) {
        rect(r, px + 3, py + 18, 18, 5, (SDL_Color){98, 87, 104, 255});
        rect(r, px + 6, py + 4, 12, 16, (SDL_Color){43, 39, 53, 255});
        rect(r, px + 6, py + 4, 3, 16, (SDL_Color){112, 99, 120, 255});
        rect(r, px + 3, py + 2, 18, 4, (SDL_Color){136, 111, 63, 255});
    } else if (tile == TILE_CASTLE_BANNER) {
        rect(r, px + 4, py + 2, 16, 3, (SDL_Color){182, 142, 66, 255});
        rect(r, px + 6, py + 5, 12, 16, (SDL_Color){119, 28, 48, 255});
        rect(r, px + 9, py + 8, 6, 6, (SDL_Color){198, 159, 75, 255});
        rect(r, px + 9, py + 8, 2, 3, (SDL_Color){119, 28, 48, 255});
        rect(r, px + 14, py + 8, 1, 3, (SDL_Color){119, 28, 48, 255});
    } else if (tile == TILE_CASTLE_GATE || tile == TILE_CASTLE_GATE_OPEN) {
        int height = tile == TILE_CASTLE_GATE ? 24 : 5;
        for (int bx = 3; bx < 24; bx += 5) {
            rect(r, px + bx, py, 2, height, (SDL_Color){145, 135, 153, 255});
        }
        rect(r, px, py + 2, 24, 3, (SDL_Color){156, 119, 65, 255});
        if (height == 24) {
            rect(r, px, py + 15, 24, 2, (SDL_Color){96, 82, 107, 255});
        }
    } else if (tile == TILE_CASTLE_LEVER) {
        rect(r, px + 4, py + 15, 16, 6, (SDL_Color){103, 86, 70, 255});
        rect(r, px + 11, py + 5, 3, 14, (SDL_Color){204, 162, 77, 255});
        rect(r, px + 9, py + 3, 7, 4, (SDL_Color){159, 45, 55, 255});
    } else if (tile == TILE_CASTLE_TRAP_OPEN) {
        rect(r, px + 2, py + 2, 20, 20, (SDL_Color){8, 6, 15, 255});
        rect(r, px + 3, py + 3, 18, 2, (SDL_Color){199, 143, 60, 255});
        rect(r, px + 4, py + 6, 2, 14, (SDL_Color){125, 70, 51, 255});
        for (int by = 9; by < 21; by += 4) {
            rect(r, px + 5, py + by, 7, 1, (SDL_Color){125, 70, 51, 255});
        }
    } else if (tile == TILE_CASTLE_SEAL) {
        rect(r, px + 5, py + 4, 14, 16, (SDL_Color){158, 111, 51, 255});
        rect(r, px + 7, py + 6, 10, 12, (SDL_Color){242, 199, 96, 255});
        rect(r, px + 10, py + 8, 4, 8, (SDL_Color){101, 51, 137, 255});
    } else if (tile == TILE_CASTLE_PASSAGE) {
        rect(r, px + 4, py + 3, 16, 20, (SDL_Color){151, 124, 77, 255});
        rect(r, px + 6, py + 5, 12, 18, (SDL_Color){15, 28, 26, 255});
        rect(r, px + 8, py + 8, 8, 12, (SDL_Color){76, 185, 137, 255});
    } else if (tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN) {
        for (int i = 0; i < 4; i++) {
            rect(r, px + 4, py + 5 + i * 4, 16, 3, (SDL_Color){109 + i * 12, 94 + i * 10, 119 + i * 10, 255});
        }
        renderer_draw_text(r, tile == TILE_STAIRS_UP ? "^" : "v", px + 8, py + 6, (SDL_Color){245, 206, 109, 255}, r->font_tiny);
    }
}

void castle_draw_enemy(Renderer *r, int sx, int sy, const Enemy *e) {
    // Source rectangles select the unchanged review sprites from the BMP sheets.
    static const SDL_Rect regular[5] = {{55, 20, 420, 465}, {515, 20, 510, 465}, {1110, 25, 400, 460}, {275, 500, 420, 510}, {805, 500, 405, 500}};
    static const SDL_Rect bosses[5] = {{115, 5, 690, 505}, {820, 5, 475, 505}, {10, 495, 510, 520}, {500, 505, 535, 510}, {960, 470, 575, 545}};
    int index = e->type - ENEMY_OATHBOUND_SOLDIER;
    SDL_Texture *texture = index < 5 ? r->castle_enemy_texture : r->castle_boss_texture;
    SDL_Rect source;
    if (index < 5) {
        source = regular[index];
    } else {
        int phase = e->hp * 3 > e->max_hp * 2 ? 0 : e->hp * 3 > e->max_hp ? 1 : 2;
        source = bosses[index == 7 ? 2 + phase : index - 5];
    }
    SDL_Rect destination = {sx * 24 - (e->is_boss ? 6 : 0), sy * 24 - (e->is_boss ? 12 : 2), e->is_boss ? 36 : 24, e->is_boss ? 36 : 26};
    if (texture) {
        SDL_RenderCopy(r->sdl, texture, &source, &destination);
    } else {
        draw_orc(r, sx, sy);
    }
    if (e->type == ENEMY_IRON_WARDEN || e->type == ENEMY_CASTELLAN ||
        (e->type == ENEMY_LORD_VEYR && e->hp * 3 > e->max_hp * 2)) {
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
            for (int i = 0; i < g->enemy_count; i++) {
                if (castle_attack_marks(&g->map, &g->enemies[i], x, y) || castle_barrier_warning(g, x, y)) {
                    SDL_Rect tile = {(x - v->cam_x) * 24 + 1, (y - v->cam_y) * 24 + 1, 22, 22};
                    SDL_SetRenderDrawColor(r->sdl, 204, 108, 239, 55);
                    SDL_RenderFillRect(r->sdl, &tile);
                    SDL_SetRenderDrawColor(r->sdl, 236, 159, 252, 255);
                    SDL_RenderDrawRect(r->sdl, &tile);
                    break;
                }
            }
        }
    }
    SDL_SetRenderDrawBlendMode(r->sdl, SDL_BLENDMODE_NONE);
}
