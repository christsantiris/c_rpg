#include "jail_renderer.h"
#include "sprites.h"
#include "../game/jail.h"

static void rect(Renderer *r, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_Rect box = {x, y, w, h};
    SDL_RenderFillRect(r->sdl, &box);
}

void jail_draw_person(Renderer *r, int sx, int sy, int prisoner) {
    int x = sx * TILE_SIZE;
    int y = sy * TILE_SIZE;
    SDL_Color skin = {208, 165, 126, 255};
    SDL_Color coat = prisoner ? (SDL_Color){112, 128, 143, 255} : (SDL_Color){130, 96, 65, 255};
    rect(r, x + 6, y + 3, 13, 9, (SDL_Color){67, 53, 44, 255});
    rect(r, x + 8, y + 6, 9, 7, skin);
    rect(r, x + 5, y + 13, 15, 8, coat);
    rect(r, x + 3, y + 14, 3, 6, skin);
    rect(r, x + 19, y + 14, 3, 6, skin);
    rect(r, x + 6, y + 21, 5, 3, (SDL_Color){49, 41, 37, 255});
    rect(r, x + 14, y + 21, 5, 3, (SDL_Color){49, 41, 37, 255});
    rect(r, x + 9, y + 8, 2, 1, (SDL_Color){31, 29, 28, 255});
    rect(r, x + 14, y + 8, 2, 1, (SDL_Color){31, 29, 28, 255});
    if (prisoner) {
        rect(r, x + 6, y + 16, 13, 2, (SDL_Color){76, 87, 102, 255});
        rect(r, x + 14, y + 19, 3, 2, (SDL_Color){183, 163, 127, 255});
    }
}

void jail_draw_building(Renderer *r, const Viewport *v) {
    int x = viewport_to_screen_x(v, JAIL_X) * TILE_SIZE;
    int y = viewport_to_screen_y(v, JAIL_Y) * TILE_SIZE;
    int w = JAIL_W * TILE_SIZE;
    int h = JAIL_H * TILE_SIZE;
    rect(r, x + 2, y + 30, w - 4, h - 30, (SDL_Color){30, 30, 38, 255});
    for (int row = 0; row < 7; row++) {
        for (int column = 0; column < 9; column++) {
            int bx = x + 6 + column * 18 - (row % 2) * 8;
            if (bx < x + 5 || bx + 16 > x + w - 5) {
                continue;
            }
            rect(r, bx, y + 39 + row * 11, 16, 9, (SDL_Color){92 + (row % 2) * 9, 93, 105, 255});
            rect(r, bx + 1, y + 39 + row * 11, 14, 1, (SDL_Color){130, 128, 137, 255});
        }
    }
    for (int row = 0; row < 8; row++) {
        int inset = 22 - row * 3;
        rect(r, x + inset, y + row * 5, w - inset * 2, 5, (SDL_Color){43 + row * 3, 55 + row * 2, 65 + row * 2, 255});
        for (int column = inset + 2; column < w - inset; column += 16) {
            rect(r, x + column, y + row * 5, 1, 4, (SDL_Color){27, 33, 43, 255});
        }
    }
    for (int window = 0; window < 2; window++) {
        int wx = x + 25 + window * 98;
        rect(r, wx - 3, y + 59, 23, 27, (SDL_Color){145, 139, 128, 255});
        rect(r, wx, y + 62, 17, 21, (SDL_Color){20, 22, 32, 255});
        for (int bar = 0; bar < 3; bar++) {
            rect(r, wx + 2 + bar * 5, y + 62, 2, 21, (SDL_Color){120, 126, 138, 255});
        }
    }
    int door = x + JAIL_W / 2 * TILE_SIZE;
    rect(r, door - 3, y + h - 36, 30, 36, (SDL_Color){153, 145, 130, 255});
    rect(r, door, y + h - 32, 24, 32, (SDL_Color){36, 40, 48, 255});
    for (int bar = 0; bar < 4; bar++) {
        rect(r, door + 3 + bar * 5, y + h - 31, 2, 30, (SDL_Color){121, 129, 144, 255});
    }
    rect(r, door + 1, y + h - 18, 22, 2, (SDL_Color){155, 151, 140, 255});
    rect(r, x + w / 2 - 27, y + 43, 54, 14, (SDL_Color){48, 39, 34, 255});
    renderer_draw_text(r, "JAIL", x + w / 2 - 20, y + 42, (SDL_Color){223, 204, 157, 255}, r->font_tiny);
}

void jail_draw_tile(Renderer *r, const GameState *g, int sx, int sy, int x, int y, TileType tile) {
    if (tile == TILE_CASTLE_WALL) {
        draw_dungeon_wall(r, sx, sy, x, y);
        return;
    }
    draw_dungeon_floor(r, sx, sy, x, y);
    int px = sx * TILE_SIZE;
    int py = sy * TILE_SIZE;
    if (tile == TILE_JAIL_BARS) {
        rect(r, px, py + 2, 24, 3, (SDL_Color){122, 126, 139, 255});
        rect(r, px, py + 20, 24, 3, (SDL_Color){87, 93, 110, 255});
        for (int bar = 0; bar < 4; bar++) {
            rect(r, px + 2 + bar * 6, py + 3, 2, 20, (SDL_Color){151, 155, 168, 255});
        }
    } else if (tile == TILE_JAIL_HATCH) {
        rect(r, px + 2, py + 3, 20, 19, (SDL_Color){20, 23, 30, 255});
        for (int rung = 0; rung < 3; rung++) {
            rect(r, px + 5, py + 6 + rung * 5, 13, 2, (SDL_Color){165, 135, 90, 255});
        }
    } else if (tile == TILE_TUNNEL_EXIT) {
        rect(r, px + 3, py + 2, 18, 22, (SDL_Color){138, 190, 149, 255});
        rect(r, px + 6, py + 6, 12, 18, (SDL_Color){217, 223, 172, 255});
    } else if (tile == TILE_NPC_PRISONER) {
        jail_draw_person(r, sx, sy, 1);
    } else if (tile == TILE_NPC_ROYAL_GUARD) {
        draw_royal_guard_overlay(r, sx, sy, 0);
    } else if (g->location == LOCATION_JAIL && x == 6 && y == 6) {
        rect(r, px + 2, py + 7, 20, 15, (SDL_Color){97, 79, 48, 255});
        for (int straw = 0; straw < 5; straw++) {
            rect(r, px + 4 + straw * 3, py + 9 + straw % 3, 1, 10, (SDL_Color){171, 146, 87, 255});
        }
    }
}
