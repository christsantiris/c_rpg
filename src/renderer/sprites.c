#include "giants_crown_renderer.h"
#include "stonewake_renderer.h"
#include "sprites.h"
#include "player_art.h"
#include "dungeon_enemy_art.h"
#include "dungeon_boss_art.h"
#include "forest_enemy_art.h"
#include "mountain_enemy_art.h"
#include "coast_enemy_art.h"
#include "swamp_enemy_art.h"
#include "dragonspine_road_art.h"
#include "frostfell_enemy_art.h"
#include "frost_kraken_art.h"
#include "desert_enemy_art.h"
#include "ashen_enemy_art.h"
#include "glassdeep_enemy_art.h"
#include "catacombs_enemy_art.h"
#include "npc_renderer.h"
#include "world_props.h"
#include "world_prop_art.h"
#include "../game/town_road.h"
#include <stdlib.h>
#include <string.h>

static void fill_rect(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
}

void draw_world_prop(Renderer *r, int tx, int ty, WorldPropArt art, SDL_Color accent) {
    if (art < 0 || art >= PROP_COUNT) {
        return;
    }
    SDL_Color light = {
        accent.r > 205 ? 255 : accent.r + 50,
        accent.g > 205 ? 255 : accent.g + 50,
        accent.b > 205 ? 255 : accent.b + 50,
        255
    };
    SDL_Color dark = {accent.r * 3 / 5, accent.g * 3 / 5, accent.b * 3 / 5, 255};
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char key = world_prop_art[art][row][col];
            int end = col + 1;
            while (end < 24 && world_prop_art[art][row][end] == key) {
                end++;
            }
            SDL_Color color;
            switch (key) {
                case '#': color = (SDL_Color){19, 21, 29, 255}; break;
                case 'd': color = (SDL_Color){48, 51, 63, 255}; break;
                case 's': color = (SDL_Color){88, 90, 101, 255}; break;
                case 'S': color = (SDL_Color){151, 151, 153, 255}; break;
                case 'b': color = (SDL_Color){57, 34, 27, 255}; break;
                case 'w': color = (SDL_Color){111, 66, 37, 255}; break;
                case 'W': color = (SDL_Color){168, 106, 56, 255}; break;
                case 'm': color = (SDL_Color){113, 129, 139, 255}; break;
                case 'M': color = (SDL_Color){205, 218, 216, 255}; break;
                case 'g': color = (SDL_Color){184, 131, 45, 255}; break;
                case 'G': color = (SDL_Color){248, 211, 103, 255}; break;
                case 'r': color = (SDL_Color){134, 37, 51, 255}; break;
                case 'R': color = (SDL_Color){203, 78, 82, 255}; break;
                case 'p': color = (SDL_Color){205, 188, 144, 255}; break;
                case 'P': color = (SDL_Color){246, 235, 198, 255}; break;
                case 'l': color = (SDL_Color){44, 91, 53, 255}; break;
                case 'L': color = (SDL_Color){103, 161, 99, 255}; break;
                case 'u': color = (SDL_Color){63, 112, 170, 255}; break;
                case 'U': color = (SDL_Color){131, 189, 232, 255}; break;
                case 'a': color = accent; color.a = 255; break;
                case 'A': color = light; break;
                case 'v': color = dark; break;
                default: col = end; continue;
            }
            fill_rect(r, tx * TILE_SIZE + col, ty * TILE_SIZE + row, end - col, 1, color);
            col = end;
        }
    }
}

void draw_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = {18, 18, 35, 255};
    SDL_Color dot  = {30, 30, 55, 255};
    fill_rect(r, x,            y,            TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + TILE_SIZE/2, y + TILE_SIZE/2, 2, 2, dot);
}

void draw_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color face  = {50, 45, 80, 255};
    SDL_Color top   = {70, 65, 105, 255};
    SDL_Color seam  = {35, 30, 60, 255};
    fill_rect(r, x,     y,     TILE_SIZE,     TILE_SIZE,     face);
    fill_rect(r, x,     y,     TILE_SIZE,     3,             top);
    fill_rect(r, x,     y,     2,             TILE_SIZE,     top);
    fill_rect(r, x + 2, y + 8, TILE_SIZE - 4, 1,             seam);
}

void draw_dungeon_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color mortar = {13, 14, 25, 255};
    SDL_Color slabs[3] = {
        {29, 29, 45, 255},
        {34, 33, 50, 255},
        {25, 27, 42, 255}
    };
    SDL_Color edge = {47, 44, 62, 255};
    SDL_Color shadow = {19, 19, 32, 255};
    unsigned int seed = (unsigned int)map_x * 1103515245u +
        (unsigned int)map_y * 2654435761u +
        (unsigned int)(map_x * map_y) * 2246822519u;

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, mortar);
    for (int row = 0; row < 3; row++) {
        int slab_y = 1 + row * 8;
        int joint = ((map_x + map_y + row) & 1) == 0 ? 10 : 14;
        int color = (int)((seed >> (row * 3)) % 3u);
        fill_rect(r, x + 1, y + slab_y, joint - 2, 7, slabs[color]);
        fill_rect(r, x + joint, y + slab_y, 23 - joint, 7,
            slabs[(color + 1) % 3]);
        fill_rect(r, x + 2, y + slab_y, joint - 3, 1, edge);
        fill_rect(r, x + joint + 1, y + slab_y, 21 - joint, 1, edge);
        fill_rect(r, x + 2, y + slab_y + 6, joint - 3, 1, shadow);
        fill_rect(r, x + joint + 1, y + slab_y + 6, 21 - joint, 1, shadow);
    }

    int detail = (int)((seed >> 17) % 23u);
    if (detail == 2 || detail == 15) {
        int crack_x = 6 + (int)((seed >> 6) % 10u);
        fill_rect(r, x + crack_x, y + 5, 1, 5, shadow);
        fill_rect(r, x + crack_x - 2, y + 9, 3, 1, shadow);
        fill_rect(r, x + crack_x - 2, y + 9, 1, 4, shadow);
    } else if (detail == 7) {
        fill_rect(r, x + 3, y + 17, 8, 4,
            (SDL_Color){18, 31, 38, 255});
        fill_rect(r, x + 6, y + 16, 8, 4,
            (SDL_Color){21, 38, 43, 255});
        fill_rect(r, x + 7, y + 17, 4, 1,
            (SDL_Color){34, 54, 55, 255});
    } else if (detail == 20) {
        fill_rect(r, x + 16, y + 15, 4, 3,
            (SDL_Color){57, 53, 61, 255});
        fill_rect(r, x + 18, y + 13, 3, 2,
            (SDL_Color){43, 41, 51, 255});
    }
}

void draw_dungeon_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color mortar = {24, 21, 38, 255};
    SDL_Color stones[3] = {
        {55, 51, 75, 255},
        {63, 57, 84, 255},
        {47, 45, 68, 255}
    };
    SDL_Color edge = {79, 70, 98, 255};
    SDL_Color shadow = {34, 30, 51, 255};
    unsigned int seed = (unsigned int)map_x * 2246822519u +
        (unsigned int)map_y * 3266489917u +
        (unsigned int)(map_x * map_y) * 668265263u;

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, mortar);
    for (int row = 0; row < 3; row++) {
        int stone_y = row * 8;
        int joint = ((map_x + map_y + row) & 1) == 0 ? 9 : 15;
        int color = (int)((seed >> (row * 4)) % 3u);
        fill_rect(r, x, y + stone_y, joint - 1, 7, stones[color]);
        fill_rect(r, x + joint, y + stone_y, 24 - joint, 7,
            stones[(color + 1) % 3]);
        fill_rect(r, x + 1, y + stone_y, joint - 2, 2, edge);
        fill_rect(r, x + joint + 1, y + stone_y, 22 - joint, 2, edge);
        fill_rect(r, x + 1, y + stone_y + 6, joint - 2, 1, shadow);
        fill_rect(r, x + joint + 1, y + stone_y + 6, 22 - joint, 1, shadow);
    }

    int detail = (int)((seed >> 18) % 19u);
    if (detail == 3) {
        fill_rect(r, x + 5, y + 2, 2, 4,
            (SDL_Color){38, 55, 47, 255});
        fill_rect(r, x + 6, y + 5, 3, 5,
            (SDL_Color){32, 48, 42, 255});
    } else if (detail == 11) {
        fill_rect(r, x + 16, y + 4, 1, 6, shadow);
        fill_rect(r, x + 13, y + 9, 4, 1, shadow);
        fill_rect(r, x + 13, y + 9, 1, 5, shadow);
    }
}

void draw_dungeon_torch(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = (unsigned int)map_x * 2246822519u ^
        (unsigned int)map_y * 3266489917u;
    unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        ((seed >> 12) & 1u)) & 1u;
    SDL_Color iron = {91, 78, 72, 255};
    SDL_Color ember = {229, 77, 21, 255};
    SDL_Color flame = {255, 157, 35, 255};
    SDL_Color highlight = {255, 224, 104, 255};

    fill_rect(r, x + 8, y + 11, 9, 3, iron);
    fill_rect(r, x + 11, y + 13, 3, 6, iron);
    if (frame == 0u) {
        fill_rect(r, x + 9, y + 5, 7, 7, ember);
        fill_rect(r, x + 11, y + 2, 4, 8, flame);
        fill_rect(r, x + 12, y + 5, 2, 4, highlight);
    } else {
        fill_rect(r, x + 10, y + 4, 6, 8, ember);
        fill_rect(r, x + 10, y + 3, 4, 7, flame);
        fill_rect(r, x + 11, y + 5, 2, 3, highlight);
    }
}

void draw_dungeon_wall_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = (unsigned int)map_x * 2246822519u ^
        (unsigned int)map_y * 3266489917u;
    int chip = 4 + (int)((seed >> 9) % 12u);
    SDL_Color stone = {48, 44, 65, 255};
    SDL_Color shadow = {11, 12, 22, 255};
    SDL_Color highlight = {70, 63, 85, 255};

    if (edges & DUNGEON_EDGE_NORTH) {
        fill_rect(r, x, y, TILE_SIZE, 1, stone);
        fill_rect(r, x, y + 1, TILE_SIZE, 2, shadow);
        fill_rect(r, x + chip, y, 4, 1, highlight);
        fill_rect(r, x + chip + 1, y + 2, 2, 2, stone);
    }
    if (edges & DUNGEON_EDGE_EAST) {
        fill_rect(r, x + TILE_SIZE - 1, y, 1, TILE_SIZE, stone);
        fill_rect(r, x + TILE_SIZE - 3, y, 2, TILE_SIZE, shadow);
        fill_rect(r, x + TILE_SIZE - 1, y + chip, 1, 4, highlight);
        fill_rect(r, x + TILE_SIZE - 4, y + chip + 1, 2, 2, stone);
    }
    if (edges & DUNGEON_EDGE_SOUTH) {
        fill_rect(r, x, y + TILE_SIZE - 1, TILE_SIZE, 1, stone);
        fill_rect(r, x, y + TILE_SIZE - 3, TILE_SIZE, 2, shadow);
        fill_rect(r, x + chip, y + TILE_SIZE - 1, 4, 1, highlight);
        fill_rect(r, x + chip + 1, y + TILE_SIZE - 4, 2, 2, stone);
    }
    if (edges & DUNGEON_EDGE_WEST) {
        fill_rect(r, x, y, 1, TILE_SIZE, stone);
        fill_rect(r, x + 1, y, 2, TILE_SIZE, shadow);
        fill_rect(r, x, y + chip, 1, 4, highlight);
        fill_rect(r, x + 2, y + chip + 1, 2, 2, stone);
    }
}

static unsigned int forest_tile_seed(int map_x, int map_y) {
    unsigned int seed = (unsigned int)map_x * 73856093u ^
        (unsigned int)map_y * 19349663u;
    seed ^= seed >> 16;
    seed *= 2246822519u;
    return seed ^ (seed >> 13);
}

void draw_swamp_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    unsigned int phase = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        (unsigned int)(map_x / 4 + map_y / 6)) & 3u;
    static const int sway[4] = {0, 1, 0, -1};
    SDL_Color mud[3] = {
        {19, 32, 22, 255},
        {22, 35, 23, 255},
        {17, 30, 21, 255}
    };
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        mud[seed % 3u]);
    int pool_x = 2 + (int)((seed >> 5) % 7u);
    int pool_y = 5 + (int)((seed >> 12) % 8u);
    fill_rect(r, x + pool_x, y + pool_y, 13, 6,
        (SDL_Color){8, 28, 25, 255});
    fill_rect(r, x + pool_x + 2 + sway[phase], y + pool_y + 2, 6, 1,
        (SDL_Color){40, 76, 57, 255});
    fill_rect(r, x + 2 + (int)((seed >> 17) % 9u), y + 2, 8, 3,
        (SDL_Color){36, 59, 31, 255});
    if (seed % 3u == 0u) {
        int reed_x = x + 17 + (int)((seed >> 21) % 3u);
        int lean = sway[phase];
        fill_rect(r, reed_x, y + 15, 2, 8,
            (SDL_Color){48, 75, 35, 255});
        fill_rect(r, reed_x + lean, y + 12, 2, 6,
            (SDL_Color){75, 109, 45, 255});
        fill_rect(r, reed_x + 2 + lean, y + 14, 2, 4,
            (SDL_Color){55, 89, 39, 255});
    }
}

void draw_swamp_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    unsigned int phase = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        (unsigned int)(map_x / 4 + map_y / 6)) & 3u;
    static const int sway[4] = {0, 1, 0, -1};
    int lean = sway[phase];
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        (SDL_Color){5, 17, 16, 255});
    fill_rect(r, x + 2, y + 18, 19, 4,
        (SDL_Color){8, 31, 25, 255});
    fill_rect(r, x + 4 + lean, y + 20, 7, 1,
        (SDL_Color){33, 67, 47, 255});
    int shape = (int)((seed >> 9) % 5u);
    if (shape == 0) {
        for (int i = 0; i < 4; i++) {
            int reed_x = x + 3 + i * 5;
            int height = 10 + (int)((seed >> (i * 3)) % 7u);
            fill_rect(r, reed_x, y + 23 - height, 2, height,
                (SDL_Color){39, 67, 34, 255});
            fill_rect(r, reed_x + lean, y + 20 - height, 3, 5,
                (SDL_Color){60, 105, 45, 255});
        }
    } else {
        int trunk_x = x + 9 + (int)((seed >> 5) % 5u);
        fill_rect(r, trunk_x, y + 5, 5, 18,
            (SDL_Color){43, 42, 28, 255});
        fill_rect(r, trunk_x + 1, y + 7, 1, 14,
            (SDL_Color){70, 66, 39, 255});
        fill_rect(r, trunk_x - 4, y + 20, 6, 2,
            (SDL_Color){55, 52, 33, 255});
        fill_rect(r, trunk_x + 3, y + 19, 7, 2,
            (SDL_Color){55, 52, 33, 255});
        if (shape == 1) {
            fill_rect(r, trunk_x - 5, y + 8, 7, 2,
                (SDL_Color){60, 55, 32, 255});
            fill_rect(r, trunk_x + 3, y + 11, 8, 2,
                (SDL_Color){60, 55, 32, 255});
            fill_rect(r, trunk_x - 7 + lean, y + 5, 6, 5,
                (SDL_Color){24, 53, 29, 255});
            fill_rect(r, trunk_x + 4 + lean, y + 7, 5, 5,
                (SDL_Color){21, 49, 27, 255});
            fill_rect(r, trunk_x + 6 + lean, y + 11, 2, 8,
                (SDL_Color){43, 74, 39, 255});
        } else {
            int crown_x = shape == 2 ? 2 : (shape == 3 ? 5 : 1);
            int crown_y = shape == 3 ? 4 : 2;
            int crown_w = shape == 3 ? 17 : 20;
            fill_rect(r, x + crown_x + lean, y + crown_y,
                crown_w, 8, (SDL_Color){17, 47, 28, 255});
            fill_rect(r, x + crown_x + 3 + lean, y,
                crown_w - 5, 6, (SDL_Color){22, 57, 31, 255});
            fill_rect(r, x + crown_x - 1 + lean, y + 8, 10, 5,
                (SDL_Color){19, 50, 29, 255});
            fill_rect(r, x + crown_x + 6 + lean, y + 2, 7, 2,
                (SDL_Color){45, 91, 43, 255});
            fill_rect(r, x + crown_x + 12 + lean, y + 7, 5, 2,
                (SDL_Color){39, 83, 40, 255});
            fill_rect(r, x + crown_x + 2 + lean, y + 10, 2, 8,
                (SDL_Color){36, 65, 35, 255});
            fill_rect(r, x + crown_x + 15 + lean, y + 9, 2, 7,
                (SDL_Color){38, 69, 37, 255});
        }
    }
}

void draw_swamp_bank(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    unsigned int phase = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        (unsigned int)(map_x / 4 + map_y / 6)) & 3u;
    static const int sway[4] = {0, 1, 0, -1};
    int lean = sway[phase];
    SDL_Color bank = {12, 28, 20, 255};
    SDL_Color root = {57, 55, 34, 255};
    SDL_Color reed = {73, 111, 48, 255};
    int offset = 5 + (int)((seed >> 7) % 9u);
    if (edges & FOREST_EDGE_NORTH) {
        fill_rect(r, x, y, TILE_SIZE, 2, bank);
        fill_rect(r, x + offset, y + 2, 6, 2, root);
        fill_rect(r, x + offset + lean, y + 2, 2, 7, reed);
    }
    if (edges & FOREST_EDGE_EAST) {
        fill_rect(r, x + TILE_SIZE - 2, y, 2, TILE_SIZE, bank);
        fill_rect(r, x + TILE_SIZE - 5, y + offset, 4, 3, root);
        fill_rect(r, x + TILE_SIZE - 7 + lean,
            y + offset - 3, 2, 8, reed);
    }
    if (edges & FOREST_EDGE_SOUTH) {
        fill_rect(r, x, y + TILE_SIZE - 2, TILE_SIZE, 2, bank);
        fill_rect(r, x + offset, y + TILE_SIZE - 5, 7, 3, root);
        fill_rect(r, x + offset + 3 + lean,
            y + TILE_SIZE - 11, 2, 9, reed);
    }
    if (edges & FOREST_EDGE_WEST) {
        fill_rect(r, x, y, 2, TILE_SIZE, bank);
        fill_rect(r, x + 1, y + offset, 5, 2, root);
        fill_rect(r, x + 3 + lean, y + offset - 4, 2, 8, reed);
    }
}

void draw_swamp_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int exit) {
    draw_swamp_floor(r, tile_x, tile_y, map_x, map_y);
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color wood = {91, 75, 43, 255};
    SDL_Color grain = {141, 119, 67, 255};
    SDL_Color glow = exit ? (SDL_Color){202, 245, 111, 255} : (SDL_Color){115, 232, 202, 255};
    fill_rect(r, x + 1, y, 5, TILE_SIZE, (SDL_Color){23, 29, 19, 255});
    fill_rect(r, x + 18, y, 5, TILE_SIZE, (SDL_Color){23, 29, 19, 255});
    fill_rect(r, x + 2, y + 1, 3, 22, wood);
    fill_rect(r, x + 19, y + 1, 3, 22, wood);
    fill_rect(r, x + 3, y + 5, 1, 16, grain);
    fill_rect(r, x + 20, y + 5, 1, 16, grain);
    fill_rect(r, x + 2, y + 1, 20, 4, wood);
    fill_rect(r, x + 3, y + 1, 18, 1, grain);
    fill_rect(r, x + 2, y + 6, 3, 4, glow);
    fill_rect(r, x + 19, y + 6, 3, 4, glow);
    fill_rect(r, x + 10, y + 9, 4, 10, glow);
    fill_rect(r, x + 7, y + 12, 10, 4, glow);
}

static void draw_woodland_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int living) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    SDL_Color soil[3] = {{13, 25, 26, 255}, {17, 29, 28, 255}, {19, 28, 29, 255}};
    SDL_Color moss = living ? (SDL_Color){46, 73, 35, 255} : (SDL_Color){28, 43, 38, 255};
    SDL_Color moss_light = living ? (SDL_Color){84, 112, 47, 255} : (SDL_Color){44, 64, 51, 255};
    SDL_Color leaf = living ? (SDL_Color){144, 118, 55, 255} : (SDL_Color){66, 61, 52, 255};
    if (living) {
        soil[0] = (SDL_Color){30, 47, 27, 255};
        soil[1] = (SDL_Color){35, 51, 28, 255};
        soil[2] = (SDL_Color){39, 53, 29, 255};
    }

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, soil[seed % 3u]);
    int patch_x = 2 + (int)((seed >> 4) % 11u);
    int patch_y = 2 + (int)((seed >> 9) % 10u);
    fill_rect(r, x + patch_x, y + patch_y, 9, 6, moss);
    fill_rect(r, x + patch_x + 2, y + patch_y - 1, 5, 2, moss);
    fill_rect(r, x + patch_x + 3, y + patch_y + 2, 3, 1, moss_light);
    fill_rect(r, x + (int)((seed >> 18) % 19u),
        y + 17 + (int)((seed >> 23) % 4u), 4, 2, leaf);
    fill_rect(r, x + (int)((seed >> 12) % 19u), y + 3, 3, 1,
        (SDL_Color){41, 54, 29, 255});

    int detail = (int)((seed >> 25) % 19u);
    if (detail < 3) {
        int root_x = 5 + (int)((seed >> 6) % 8u);
        fill_rect(r, x + root_x, y + 15, 1, 7,
            (SDL_Color){62, 51, 32, 255});
        fill_rect(r, x + root_x + 1, y + 19, 5, 1,
            (SDL_Color){62, 51, 32, 255});
    } else if (detail < 6) {
        int grass_x = 4 + (int)((seed >> 7) % 13u);
        fill_rect(r, x + grass_x, y + 12, 1, 5, moss_light);
        fill_rect(r, x + grass_x - 2, y + 14, 1, 3, moss_light);
        fill_rect(r, x + grass_x + 2, y + 13, 1, 4, moss_light);
    } else if (detail == 11) {
        fill_rect(r, x + 18, y + 9, 1, 5, moss_light);
        fill_rect(r, x + 17, y + 8, 3, 2,
            (SDL_Color){149, 143, 77, 255});
    } else if (detail == 17) {
        fill_rect(r, x + 5, y + 14, 2, 3,
            (SDL_Color){148, 127, 91, 255});
        fill_rect(r, x + 3, y + 12, 6, 2,
            (SDL_Color){131, 63, 50, 255});
    }
}

void draw_forest_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_woodland_floor(r, tile_x, tile_y, map_x, map_y, 0);
}

void draw_elderwood_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_woodland_floor(r, tile_x, tile_y, map_x, map_y, 1);
}

static void draw_woodland_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int living) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    int trunk_x = 7 + (int)((seed >> 4) % 7u);
    unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        ((seed >> 16) & 1u)) & 1u;
    int canopy_shift = (seed >> 20) % 3u == 0u ? (int)frame : 0;
    SDL_Color shadow = living ? (SDL_Color){14, 31, 17, 255} : (SDL_Color){7, 18, 20, 255};
    SDL_Color foliage = living ? (SDL_Color){31, 69, 33, 255} : (SDL_Color){24, 42, 38, 255};
    SDL_Color foliage_light = living ? (SDL_Color){85, 119, 48, 255} : (SDL_Color){44, 64, 53, 255};
    SDL_Color bark = living ? (SDL_Color){91, 74, 41, 255} : (SDL_Color){68, 62, 54, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, shadow);
    fill_rect(r, x + 1, y + 5, 9, 10, foliage);
    fill_rect(r, x + 14, y + 9, 9, 10, foliage);
    fill_rect(r, x + 3 + canopy_shift, y + 15, 7, 5, foliage_light);
    fill_rect(r, x + 17 + canopy_shift, y + 4, 5, 4, foliage_light);
    int shape = (int)((seed >> 11) % 4u);
    if (!living && shape != 0) {
        // Bare crooked branches interrupt the canopy in the corrupted woods.
        fill_rect(r, x + trunk_x, y + 8, 5, 16, bark);
        fill_rect(r, x + trunk_x + 1, y + 5, 3, 14, (SDL_Color){94, 84, 66, 255});
        fill_rect(r, x + trunk_x - 2, y + 18, 8, 4, bark);
        SDL_SetRenderDrawColor(r->sdl, bark.r, bark.g, bark.b, 255);
        SDL_RenderDrawLine(r->sdl, x + trunk_x + 2, y + 13, x + 3, y + 5);
        SDL_RenderDrawLine(r->sdl, x + 3, y + 5, x + 4, y + 1);
        SDL_RenderDrawLine(r->sdl, x + trunk_x + 3, y + 16, x + 21, y + 8);
        SDL_RenderDrawLine(r->sdl, x + 21, y + 8, x + 19, y + 3);
        SDL_RenderDrawLine(r->sdl, x + trunk_x + 2, y + 7, x + 16, y + 1);
        return;
    }
    if (shape == 0) {
        fill_rect(r, x + 2, y + 7, 20, 13, foliage);
        fill_rect(r, x + 4, y + 4, 11, 9,
            living ? (SDL_Color){49, 88, 36, 255} : (SDL_Color){29, 48, 42, 255});
        fill_rect(r, x + 12, y + 11, 10, 10,
            living ? (SDL_Color){46, 82, 35, 255} : (SDL_Color){27, 46, 40, 255});
        fill_rect(r, x + 6 + canopy_shift, y + 6, 6, 2,
            foliage_light);
        fill_rect(r, x + 15 + canopy_shift, y + 13, 5, 2,
            foliage_light);
    } else {
        fill_rect(r, x + trunk_x - 1, y + 5, 8, 19,
            (SDL_Color){35, 35, 23, 255});
        fill_rect(r, x + trunk_x, y + 5, 6, 19, bark);
        fill_rect(r, x + trunk_x + 1, y + 7, 2, 14,
            (SDL_Color){91, 76, 44, 255});
        fill_rect(r, x + trunk_x + 4, y + 10, 1, 8,
            (SDL_Color){43, 39, 27, 255});
        fill_rect(r, x + trunk_x - 3, y + 21, 5, 2, bark);
        fill_rect(r, x + trunk_x + 5, y + 20, 5, 2, bark);
        if (shape == 1) {
            fill_rect(r, x + trunk_x - 5, y + 10, 6, 2, bark);
        } else if (shape == 2) {
            fill_rect(r, x + trunk_x + 5, y + 13, 7, 2, bark);
        }
        fill_rect(r, x + trunk_x - 4, y + 1, 14, 7, foliage);
        fill_rect(r, x + trunk_x - 2, y, 11, 5,
            (SDL_Color){29, 66, 33, 255});
        fill_rect(r, x + trunk_x + 1 + canopy_shift, y + 2, 5, 2,
            foliage_light);
    }

    if ((seed >> 20) % 5u == 0u) {
        fill_rect(r, x + 2, y + 8, 4, 3,
            (SDL_Color){49, 91, 42, 255});
    } else if ((seed >> 20) % 5u == 1u) {
        fill_rect(r, x + 18, y + 15, 3, 3,
            (SDL_Color){48, 84, 39, 255});
    }
}

void draw_forest_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_woodland_wall(r, tile_x, tile_y, map_x, map_y, 0);
}

void draw_elderwood_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_woodland_wall(r, tile_x, tile_y, map_x, map_y, 1);
}

void draw_forest_tree_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    int offset = 4 + (int)((seed >> 7) % 8u);
    SDL_Color shade = {8, 25, 16, 255};
    SDL_Color moss = {25, 55, 29, 255};
    SDL_Color root = {62, 51, 32, 255};

    if (edges & FOREST_EDGE_NORTH) {
        fill_rect(r, x, y, TILE_SIZE, 1, shade);
        fill_rect(r, x + offset, y + 1, 7, 2, moss);
        fill_rect(r, x + offset + 2, y + 3, 2, 2, root);
    }
    if (edges & FOREST_EDGE_EAST) {
        fill_rect(r, x + TILE_SIZE - 1, y, 1, TILE_SIZE, shade);
        fill_rect(r, x + TILE_SIZE - 3, y + offset, 2, 7, moss);
        fill_rect(r, x + TILE_SIZE - 5, y + offset + 2, 2, 2, root);
    }
    if (edges & FOREST_EDGE_SOUTH) {
        fill_rect(r, x, y + TILE_SIZE - 1, TILE_SIZE, 1, shade);
        fill_rect(r, x + offset, y + TILE_SIZE - 3, 7, 2, moss);
        fill_rect(r, x + offset + 2, y + TILE_SIZE - 5, 2, 2, root);
    }
    if (edges & FOREST_EDGE_WEST) {
        fill_rect(r, x, y, 1, TILE_SIZE, shade);
        fill_rect(r, x + 1, y + offset, 2, 7, moss);
        fill_rect(r, x + 3, y + offset + 2, 2, 2, root);
    }
}

void draw_forest_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward) {
    int x = tile_x * TILE_SIZE, y = tile_y * TILE_SIZE;
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    SDL_Color trunk = {60, 43, 25, 255};
    SDL_Color leaf = {25, 80, 38, 255};
    SDL_Color glow = forward ? (SDL_Color){112, 224, 92, 255} :
        (SDL_Color){76, 155, 120, 255};
    fill_rect(r, x+2, y, 4, TILE_SIZE, trunk);
    fill_rect(r, x+18, y, 4, TILE_SIZE, trunk);
    fill_rect(r, x+3, y+1, 18, 5, leaf);
    fill_rect(r, x+10, y+8, 4, 9, glow);
    fill_rect(r, x+8, y+11, 8, 3, glow);
}

void draw_forest_landmark(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_MARKER, (SDL_Color){94, 224, 126, 255});
}

void draw_forest_ruin(Renderer *r, int tile_x, int tile_y, int offset_x, int offset_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {76, 95, 82, 255};
    SDL_Color edge = {109, 132, 113, 255};
    SDL_Color shadow = {42, 58, 47, 255};
    SDL_Color moss = {35, 77, 39, 255};

    if (offset_y == -1 && offset_x == -1) {
        fill_rect(r, x + 15, y + 15, 9, 6, shadow);
        fill_rect(r, x + 16, y + 13, 8, 5, stone);
        fill_rect(r, x + 19, y + 13, 5, 1, edge);
        fill_rect(r, x + 15, y + 18, 4, 3, moss);
    } else if (offset_y == -1 && offset_x == 0) {
        fill_rect(r, x, y + 15, 10, 6, stone);
        fill_rect(r, x + 14, y + 15, 10, 6, stone);
        fill_rect(r, x, y + 14, 8, 2, edge);
        fill_rect(r, x + 16, y + 14, 8, 2, edge);
        fill_rect(r, x + 8, y + 18, 3, 3, shadow);
        fill_rect(r, x + 13, y + 18, 3, 3, shadow);
        fill_rect(r, x + 2, y + 15, 4, 2, moss);
    } else if (offset_y == -1 && offset_x == 1) {
        fill_rect(r, x, y + 15, 9, 6, shadow);
        fill_rect(r, x, y + 13, 8, 5, stone);
        fill_rect(r, x, y + 13, 5, 1, edge);
        fill_rect(r, x + 5, y + 18, 4, 3, moss);
    } else if (offset_y == 0 && offset_x == -1) {
        fill_rect(r, x + 15, y + 5, 8, 18, shadow);
        fill_rect(r, x + 17, y + 3, 6, 17, stone);
        fill_rect(r, x + 16, y + 3, 8, 2, edge);
        fill_rect(r, x + 18, y + 8, 3, 8, moss);
        fill_rect(r, x + 15, y + 20, 9, 2, edge);
    } else if (offset_y == 0 && offset_x == 0) {
        fill_rect(r, x + 5, y + 17, 15, 5, shadow);
        fill_rect(r, x + 8, y + 5, 9, 14, stone);
        fill_rect(r, x + 10, y + 2, 5, 5, edge);
        fill_rect(r, x + 11, y + 8, 3, 7, moss);
        fill_rect(r, x + 9, y + 10, 7, 3, moss);
    } else if (offset_y == 0 && offset_x == 1) {
        fill_rect(r, x + 1, y + 5, 8, 18, shadow);
        fill_rect(r, x + 1, y + 3, 6, 17, stone);
        fill_rect(r, x, y + 3, 8, 2, edge);
        fill_rect(r, x + 3, y + 8, 3, 8, moss);
        fill_rect(r, x, y + 20, 9, 2, edge);
    } else if (offset_y == 1 && offset_x == -1) {
        fill_rect(r, x + 16, y + 2, 8, 5, shadow);
        fill_rect(r, x + 18, y + 1, 6, 3, stone);
        fill_rect(r, x + 20, y + 1, 4, 1, edge);
    } else if (offset_y == 1 && offset_x == 0) {
        fill_rect(r, x + 4, y + 1, 16, 6, shadow);
        fill_rect(r, x + 6, y + 1, 12, 3, stone);
        fill_rect(r, x + 8, y + 1, 8, 1, edge);
        fill_rect(r, x + 4, y + 5, 5, 2, moss);
    } else if (offset_y == 1 && offset_x == 1) {
        fill_rect(r, x, y + 2, 8, 5, shadow);
        fill_rect(r, x, y + 1, 6, 3, stone);
        fill_rect(r, x, y + 1, 4, 1, edge);
    }
}

void draw_forest_false_marker(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_FALSE_MARKER, (SDL_Color){64, 124, 76, 255});
}

static unsigned int mountain_tile_seed(int map_x, int map_y) {
    unsigned int seed = (unsigned int)map_x * 2654435761u ^
        (unsigned int)map_y * 2246822519u;
    seed ^= seed >> 16;
    seed *= 3266489917u;
    return seed ^ (seed >> 15);
}

static void draw_mountain_particle(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int cave) {
    unsigned int seed = mountain_tile_seed(map_x, map_y);
    if ((seed >> 20) % 3u != 0u) {
        return;
    }
    unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        ((seed >> 17) & 1u)) & 1u;
    int x = tile_x * TILE_SIZE + 3 + (int)((seed >> 5) % 16u);
    int y = tile_y * TILE_SIZE + 6 + (int)((seed >> 11) % 12u);
    int ember = ((seed >> 23) & 1u) == 0u;
    SDL_Color color = ember ? (SDL_Color){224, 83, 31, 255} :
        (cave ? (SDL_Color){105, 96, 91, 255} :
        (SDL_Color){139, 119, 105, 255});

    fill_rect(r, x + (int)frame, y - (int)frame, ember ? 2 : 1, 1,
        color);
}

static void draw_mountain_ground(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int cave) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = mountain_tile_seed(map_x, map_y);
    SDL_Color open_ground[3] = {
        {28, 24, 29, 255}, {33, 27, 31, 255}, {30, 25, 27, 255}
    };
    SDL_Color cave_ground[3] = {
        {19, 18, 23, 255}, {23, 20, 25, 255}, {20, 19, 24, 255}
    };
    SDL_Color stone = cave ? (SDL_Color){53, 45, 50, 255} :
        (SDL_Color){68, 54, 51, 255};
    SDL_Color shadow = cave ? (SDL_Color){13, 13, 18, 255} :
        (SDL_Color){19, 18, 24, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        cave ? cave_ground[seed % 3u] : open_ground[seed % 3u]);
    for (int row = 0; row < 2; row++) {
        int shard_x = 2 + (int)((seed >> (row * 8 + 4)) % 10u);
        int shard_y = 3 + row * 11 + (int)((seed >> (row * 8 + 9)) % 3u);
        int shard_w = 6 + (int)((seed >> (row * 8 + 6)) % 5u);
        fill_rect(r, x + shard_x, y + shard_y, shard_w, 3, stone);
        fill_rect(r, x + shard_x + 2, y + shard_y + 3, shard_w - 2, 1,
            shadow);
    }
    for (int gravel = 0; gravel < 3; gravel++) {
        unsigned int bits = seed >> (gravel * 7);
        int gravel_x = 2 + (int)(bits % 19u);
        int gravel_y = 3 + (int)((bits >> 4) % 18u);
        fill_rect(r, x + gravel_x, y + gravel_y, 2, 2,
            cave ? (SDL_Color){79, 65, 58, 255} :
            (SDL_Color){97, 76, 61, 255});
    }

    int detail = (int)((seed >> 24) % 17u);
    if (detail == 2 || detail == 9) {
        fill_rect(r, x + 4, y + 17, 8, 3,
            (SDL_Color){56, 53, 52, 255});
        fill_rect(r, x + 7, y + 16, 5, 1,
            (SDL_Color){73, 68, 62, 255});
    } else if (detail == 6 && !cave) {
        fill_rect(r, x + 14, y + 7, 5, 3,
            (SDL_Color){56, 70, 49, 255});
        fill_rect(r, x + 16, y + 6, 4, 1,
            (SDL_Color){75, 88, 57, 255});
    } else if (detail == 13 && cave) {
        fill_rect(r, x + 7, y + 19, 3, 2,
            (SDL_Color){142, 58, 32, 255});
    }
    draw_mountain_particle(r, tile_x, tile_y, map_x, map_y, cave);
}

void draw_mountain_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_mountain_ground(r, tile_x, tile_y, map_x, map_y, 0);
}

void draw_mountain_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = mountain_tile_seed(map_x, map_y);
    SDL_Color rock[3] = {
        {52, 46, 50, 255}, {64, 51, 50, 255}, {45, 42, 49, 255}
    };
    SDL_Color rim = {83, 68, 65, 255};
    SDL_Color shade = {23, 21, 27, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        (SDL_Color){35, 31, 36, 255});
    for (int piece = 0; piece < 5; piece++) {
        unsigned int bits = seed + (unsigned int)piece * 2654435761u;
        bits ^= bits >> 16;
        int left = (int)(bits % 16u);
        int top = (int)((bits >> 5) % 19u);
        int width = 8 + (int)((bits >> 11) % 8u);
        int height = 5 + (int)((bits >> 18) % 6u);
        if (left + width > TILE_SIZE) {
            width = TILE_SIZE - left;
        }
        if (top + height > TILE_SIZE) {
            height = TILE_SIZE - top;
        }
        fill_rect(r, x + left, y + top, width, height,
            rock[(bits >> 23) % 3u]);
        fill_rect(r, x + left + 1, y + top, width - 2, 1, rim);
        fill_rect(r, x + left + width - 1, y + top + 1, 1, height - 1,
            shade);
        fill_rect(r, x + left + 1, y + top + height - 1,
            width - 1, 1, shade);
    }

    int detail = (int)((seed >> 25) % 17u);
    if (detail == 4) {
        fill_rect(r, x + 7, y + 3, 2, 5,
            (SDL_Color){46, 66, 47, 255});
        fill_rect(r, x + 9, y + 6, 4, 2,
            (SDL_Color){63, 82, 53, 255});
    } else if (detail == 11) {
        fill_rect(r, x + 15, y + 5, 1, 7, shade);
        fill_rect(r, x + 12, y + 11, 4, 1, shade);
        fill_rect(r, x + 12, y + 11, 1, 5, shade);
    } else if (detail == 15) {
        fill_rect(r, x + 5, y + 15, 7, 1,
            (SDL_Color){109, 51, 40, 255});
    }
}

void draw_mountain_gate(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_mountain_wall(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_DOOR, (SDL_Color){196, 91, 47, 255});
}

void draw_mountain_gate_support(Renderer *r, int tile_x, int tile_y, int above) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {87, 71, 64, 255};
    SDL_Color rim = {139, 103, 72, 255};
    SDL_Color shadow = {34, 29, 33, 255};

    if (above) {
        fill_rect(r, x + 2, y + 13, 20, 11, shadow);
        fill_rect(r, x + 3, y + 13, 18, 9, stone);
        fill_rect(r, x + 2, y + 13, 20, 2, rim);
        fill_rect(r, x + 9, y + 16, 6, 7, (SDL_Color){148, 50, 32, 255});
        fill_rect(r, x + 11, y + 16, 2, 5, (SDL_Color){218, 114, 52, 255});
    } else {
        fill_rect(r, x + 2, y, 20, 9, shadow);
        fill_rect(r, x + 3, y, 18, 7, stone);
        fill_rect(r, x + 2, y, 20, 2, rim);
        fill_rect(r, x + 6, y + 4, 3, 2, (SDL_Color){115, 111, 108, 255});
        fill_rect(r, x + 15, y + 4, 3, 2, (SDL_Color){115, 111, 108, 255});
    }
}

void draw_mountain_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward) {
    int x=tile_x*TILE_SIZE, y=tile_y*TILE_SIZE;
    draw_mountain_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r,x+2,y+1,5,22,(SDL_Color){40,31,32,255});
    fill_rect(r,x+18,y+1,5,22,(SDL_Color){40,31,32,255});
    fill_rect(r,x+2,y+1,21,4,(SDL_Color){91,32,28,255});
    fill_rect(r,x+10,y+8,5,9,forward ?
        (SDL_Color){239,65,25,255} : (SDL_Color){145,47,32,255});
}

void draw_mountain_bridge(Renderer *r, int tile_x, int tile_y, unsigned int paths) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){12, 8, 11, 255});
    SDL_Color deck = {81, 48, 31, 255};
    SDL_Color rim = {136, 76, 39, 255};
    SDL_Color shade = {48, 29, 24, 255};
    SDL_Color joint = {35, 25, 24, 255};
    fill_rect(r, x + 3, y + 3, 18, 18, deck);
    if (paths & MOUNTAIN_EDGE_NORTH) {
        fill_rect(r, x + 3, y, 18, 3, deck);
    } else {
        fill_rect(r, x + 3, y + 3, 18, 2, rim);
    }
    if (paths & MOUNTAIN_EDGE_EAST) {
        fill_rect(r, x + 21, y + 3, 3, 18, deck);
    } else {
        fill_rect(r, x + 19, y + 3, 2, 18, shade);
    }
    if (paths & MOUNTAIN_EDGE_SOUTH) {
        fill_rect(r, x + 3, y + 21, 18, 3, deck);
    } else {
        fill_rect(r, x + 3, y + 19, 18, 2, shade);
    }
    if (paths & MOUNTAIN_EDGE_WEST) {
        fill_rect(r, x, y + 3, 3, 18, deck);
    } else {
        fill_rect(r, x + 3, y + 3, 2, 18, rim);
    }
    int horizontal = (paths & MOUNTAIN_EDGE_EAST) != 0;
    horizontal += (paths & MOUNTAIN_EDGE_WEST) != 0;
    int vertical = (paths & MOUNTAIN_EDGE_NORTH) != 0;
    vertical += (paths & MOUNTAIN_EDGE_SOUTH) != 0;
    if (vertical > horizontal) {
        fill_rect(r, x + 5, y + 8, 14, 2, joint);
        fill_rect(r, x + 5, y + 16, 14, 2, joint);
    } else {
        fill_rect(r, x + 8, y + 5, 2, 14, joint);
        fill_rect(r, x + 16, y + 5, 2, 14, joint);
    }
}

void draw_mountain_rockfall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_mountain_wall(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 3, y + 14, 8, 7, (SDL_Color){142, 91, 64, 255});
    fill_rect(r, x + 13, y + 12, 9, 9, (SDL_Color){105, 72, 60, 255});
    fill_rect(r, x + 8, y + 7, 8, 8, (SDL_Color){176, 117, 70, 255});
    fill_rect(r, x + 10, y + 3, 3, 5, (SDL_Color){236, 181, 78, 255});
}

void draw_mountain_chasm(Renderer *r, int tile_x, int tile_y, unsigned int bridges) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){5, 3, 8, 255});
    SDL_Color rock = {53, 45, 50, 255};
    SDL_Color rim = {99, 69, 65, 255};
    SDL_Color wood = {81, 48, 31, 255};
    SDL_Color splinter = {136, 76, 39, 255};
    if (bridges & MOUNTAIN_EDGE_NORTH) {
        fill_rect(r, x + 5, y, 14, 6, wood);
        fill_rect(r, x + 7, y + 5, 4, 3, splinter);
    } else {
        fill_rect(r, x, y, TILE_SIZE, 4, rock);
        fill_rect(r, x + 2, y + 3, 20, 1, rim);
    }
    if (bridges & MOUNTAIN_EDGE_EAST) {
        fill_rect(r, x + 18, y + 5, 6, 14, wood);
        fill_rect(r, x + 16, y + 14, 3, 4, splinter);
    } else {
        fill_rect(r, x + 20, y, 4, TILE_SIZE, rock);
        fill_rect(r, x + 20, y + 2, 1, 20, rim);
    }
    if (bridges & MOUNTAIN_EDGE_SOUTH) {
        fill_rect(r, x + 5, y + 18, 14, 6, wood);
        fill_rect(r, x + 14, y + 16, 4, 3, splinter);
    } else {
        fill_rect(r, x, y + 20, TILE_SIZE, 4, rock);
        fill_rect(r, x + 2, y + 20, 20, 1, rim);
    }
    if (bridges & MOUNTAIN_EDGE_WEST) {
        fill_rect(r, x, y + 5, 6, 14, wood);
        fill_rect(r, x + 5, y + 7, 3, 4, splinter);
    } else {
        fill_rect(r, x, y, 4, TILE_SIZE, rock);
        fill_rect(r, x + 3, y + 2, 1, 20, rim);
    }
}

void draw_mountain_cave_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_mountain_ground(r, tile_x, tile_y, map_x, map_y, 1);
}

void draw_mountain_fortress_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = mountain_tile_seed(map_x, map_y);
    SDL_Color flags[3] = {
        {49, 42, 45, 255}, {57, 46, 47, 255}, {42, 38, 44, 255}
    };
    SDL_Color rim = {83, 67, 63, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        (SDL_Color){22, 19, 24, 255});
    for (int row = 0; row < 2; row++) {
        int flag_y = row * 12 + 1;
        int joint = ((map_x + map_y + row) & 1) == 0 ? 10 : 15;
        int color = (int)((seed >> (row * 7)) % 3u);
        fill_rect(r, x + 1, y + flag_y, joint - 2, 10, flags[color]);
        fill_rect(r, x + joint, y + flag_y, 23 - joint, 10,
            flags[(color + 1) % 3]);
        fill_rect(r, x + 2, y + flag_y, joint - 3, 1, rim);
        fill_rect(r, x + joint + 1, y + flag_y, 21 - joint, 1, rim);
    }
    if ((seed >> 20) % 9u == 2u) {
        fill_rect(r, x + 15, y + 17, 5, 2,
            (SDL_Color){30, 27, 30, 255});
    }
    draw_mountain_particle(r, tile_x, tile_y, map_x, map_y, 0);
}

static unsigned int coast_tile_seed(int map_x, int map_y) {
    unsigned int seed = (unsigned int)map_x * 2246822519u ^
        (unsigned int)map_y * 3266489917u;
    seed ^= seed >> 16;
    seed *= 668265263u;
    return seed ^ (seed >> 15);
}

void draw_coast_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = coast_tile_seed(map_x, map_y);
    SDL_Color stone[3] = {
        {24, 62, 69, 255}, {29, 70, 74, 255}, {20, 56, 66, 255}
    };
    SDL_Color wet_edge = {46, 105, 105, 255};
    SDL_Color seam = {9, 33, 44, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, seam);
    for (int row = 0; row < 2; row++) {
        int slab_y = 1 + row * 12;
        int joint = ((map_x + map_y + row) & 1) == 0 ? 10 : 15;
        int color = (int)((seed >> (row * 7)) % 3u);
        fill_rect(r, x + 1, y + slab_y, joint - 2, 10, stone[color]);
        fill_rect(r, x + joint, y + slab_y, 23 - joint, 10,
            stone[(color + 1) % 3]);
        fill_rect(r, x + 2, y + slab_y, joint - 3, 1, wet_edge);
        fill_rect(r, x + joint + 1, y + slab_y, 21 - joint, 1, wet_edge);
    }

    int detail = (int)((seed >> 23) % 23u);
    if (detail == 2 || detail == 16) {
        fill_rect(r, x + 3, y + 18, 7, 3,
            (SDL_Color){96, 91, 68, 255});
        fill_rect(r, x + 5, y + 17, 4, 1,
            (SDL_Color){124, 111, 75, 255});
    } else if (detail == 7) {
        fill_rect(r, x + 17, y + 8, 2, 7,
            (SDL_Color){24, 79, 69, 255});
        fill_rect(r, x + 15, y + 11, 3, 3,
            (SDL_Color){29, 92, 73, 255});
    } else if (detail == 12) {
        fill_rect(r, x + 5, y + 7, 4, 2,
            (SDL_Color){150, 150, 122, 255});
        fill_rect(r, x + 6, y + 6, 2, 1,
            (SDL_Color){192, 184, 146, 255});
    }
}

void draw_coast_sand(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int wet) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = coast_tile_seed(map_x, map_y);
    SDL_Color sand = wet ? (SDL_Color){73, 103, 99, 255} : (SDL_Color){141, 133, 99, 255};
    SDL_Color ridge = wet ? (SDL_Color){93, 128, 119, 255} : (SDL_Color){166, 155, 115, 255};
    SDL_Color shade = wet ? (SDL_Color){51, 83, 84, 255} : (SDL_Color){117, 113, 86, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, sand);
    // Continuous ripples cross tile boundaries without a paving-stone grid.
    for (int row = 0; row < 3; row++) {
        int ripple_y = y + row * 8 + (map_x + row * 2) % 4;
        fill_rect(r, x, ripple_y, 10, 1, ridge);
        fill_rect(r, x + 10, ripple_y + 1, 8, 1, ridge);
        fill_rect(r, x + 3, ripple_y + 2, 12, 1, shade);
    }
    int detail = (int)(seed % 19u);
    if (detail == 0) {
        // A small shell, with a dark lip and a pale, ridged crown.
        fill_rect(r, x + 9, y + 14, 7, 3, (SDL_Color){87, 96, 84, 255});
        fill_rect(r, x + 10, y + 12, 5, 4, (SDL_Color){194, 189, 153, 255});
        fill_rect(r, x + 12, y + 11, 1, 5, (SDL_Color){222, 215, 176, 255});
    } else if (detail == 4) {
        // Stranded kelp follows the outgoing water.
        fill_rect(r, x + 7, y + 12, 10, 2, (SDL_Color){48, 83, 65, 255});
        fill_rect(r, x + 9, y + 10, 4, 2, (SDL_Color){67, 103, 72, 255});
        fill_rect(r, x + 14, y + 14, 5, 2, (SDL_Color){55, 91, 68, 255});
    } else if (detail == 9) {
        // Salt-worn driftwood, kept below the scale of a walking sprite.
        fill_rect(r, x + 5, y + 15, 13, 3, (SDL_Color){81, 81, 67, 255});
        fill_rect(r, x + 7, y + 14, 9, 2, (SDL_Color){171, 164, 129, 255});
        fill_rect(r, x + 15, y + 12, 2, 4, (SDL_Color){121, 118, 93, 255});
    }
}

void draw_coast_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = coast_tile_seed(map_x, map_y);
    SDL_Color rock[3] = {
        {28, 65, 69, 255}, {34, 73, 73, 255}, {24, 56, 65, 255}
    };
    SDL_Color wet_edge = {62, 112, 105, 255};
    SDL_Color shadow = {9, 31, 41, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        (SDL_Color){14, 39, 48, 255});
    for (int piece = 0; piece < 4; piece++) {
        unsigned int bits = seed + (unsigned int)piece * 2654435761u;
        bits ^= bits >> 16;
        int left = (int)(bits % 15u);
        int top = (int)((bits >> 5) % 18u);
        int width = 9 + (int)((bits >> 11) % 7u);
        int height = 6 + (int)((bits >> 18) % 6u);
        if (left + width > TILE_SIZE) {
            width = TILE_SIZE - left;
        }
        if (top + height > TILE_SIZE) {
            height = TILE_SIZE - top;
        }
        fill_rect(r, x + left, y + top, width, height,
            rock[(bits >> 23) % 3u]);
        fill_rect(r, x + left + 1, y + top, width - 2, 1, wet_edge);
        fill_rect(r, x + left + width - 1, y + top + 1, 1, height - 1,
            shadow);
    }

    int detail = (int)((seed >> 24) % 17u);
    if (detail == 3) {
        fill_rect(r, x + 8, y + 4, 2, 7,
            (SDL_Color){29, 87, 70, 255});
        fill_rect(r, x + 10, y + 8, 4, 2,
            (SDL_Color){39, 103, 74, 255});
    } else if (detail == 11) {
        fill_rect(r, x + 15, y + 14, 3, 2,
            (SDL_Color){120, 124, 100, 255});
        fill_rect(r, x + 19, y + 17, 2, 2,
            (SDL_Color){98, 112, 93, 255});
    }
}

void draw_coast_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 2, y + 2, 5, 21, (SDL_Color){42, 84, 82, 255});
    fill_rect(r, x + 18, y + 2, 5, 21, (SDL_Color){42, 84, 82, 255});
    fill_rect(r, x + 2, y + 2, 21, 4, (SDL_Color){93, 139, 117, 255});
    if (forward == 2) {
        fill_rect(r, x + 7, y + 7, 11, 13,
            (SDL_Color){5, 28, 48, 255});
        fill_rect(r, x + 8, y + 9, 2, 11,
            (SDL_Color){93, 139, 117, 255});
        fill_rect(r, x + 14, y + 9, 2, 11,
            (SDL_Color){93, 139, 117, 255});
    } else {
        fill_rect(r, x + 9, y + 8, 7, 9, forward ?
            (SDL_Color){62, 220, 216, 255} :
            (SDL_Color){47, 130, 145, 255});
    }
}

static void draw_coast_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int deep) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = coast_tile_seed(map_x, map_y);
    unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        ((seed >> 20) & 1u)) & 1u;
    SDL_Color shallow[3] = {
        {19, 75, 90, 255}, {23, 81, 94, 255}, {17, 70, 87, 255}
    };
    SDL_Color deep_water[3] = {
        {6, 38, 60, 255}, {7, 44, 65, 255}, {5, 35, 57, 255}
    };
    SDL_Color ripple = deep == 2 ? (SDL_Color){17, 61, 77, 255} : deep ? (SDL_Color){29, 96, 122, 255} :
        (SDL_Color){68, 152, 154, 255};
    SDL_Color glint = deep == 2 ? (SDL_Color){28, 78, 95, 255} : deep ? (SDL_Color){51, 120, 138, 255} :
        (SDL_Color){111, 191, 180, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        deep == 2 ? (SDL_Color){6, 27, 45, 255} : deep ? deep_water[seed % 3u] : shallow[seed % 3u]);
    for (int wave = 0; wave < 2; wave++) {
        unsigned int bits = seed >> (wave * 9);
        int wave_x = 1 + (int)(bits % 11u) + (int)frame;
        int wave_y = 4 + wave * 10 + (int)((bits >> 5) % 4u);
        int width = 7 + (int)((bits >> 11) % 6u);
        fill_rect(r, x + wave_x, y + wave_y, width, 1, ripple);
        fill_rect(r, x + wave_x + 2, y + wave_y - 1, width - 4, 1,
            glint);
    }
    if (!deep && (seed >> 24) % 7u == 2u) {
        fill_rect(r, x + 3 + (int)frame, y + 20, 8, 1,
            (SDL_Color){153, 206, 189, 255});
        fill_rect(r, x + 5 + (int)frame, y + 19, 3, 1,
            (SDL_Color){196, 220, 203, 255});
    }
}

void draw_coast_shallow_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_water(r, tile_x, tile_y, map_x, map_y, 0);
}

void draw_coast_sea(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_water(r, tile_x, tile_y, map_x, map_y, 2);
}

void draw_coast_deep_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_water(r, tile_x, tile_y, map_x, map_y, 1);
}

void draw_coast_shore(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges, TileType water) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int shift = (int)((coast_tile_seed(map_x, map_y) >> 7) % 4u);
    int deep = water == TILE_COAST_SEA || water == TILE_COAST_DEEP_WATER ||
        water == TILE_COAST_CHANNEL_WATER;
    int channel = water == TILE_COAST_CHANNEL_WATER;
    SDL_Color wash = deep ? (SDL_Color){18, 62, 80, 255} :
        (SDL_Color){32, 94, 104, 255};
    SDL_Color foam = deep ? (SDL_Color){57, 125, 137, 255} :
        (SDL_Color){104, 173, 166, 255};

    if (edges & COAST_SHORE_NORTH) {
        fill_rect(r, x + (channel ? 3 : 0), y,
            TILE_SIZE - (channel ? 3 : 0), 2, wash);
        fill_rect(r, x + 4 + shift, y + 1, 8, 1, foam);
        fill_rect(r, x + 15, y, 5, 1, foam);
    }
    if (edges & COAST_SHORE_EAST) {
        fill_rect(r, x + TILE_SIZE - 2, y, 2,
            TILE_SIZE - (channel ? 3 : 0), wash);
        fill_rect(r, x + TILE_SIZE - 2, y + 2 + shift, 1, 8, foam);
        fill_rect(r, x + TILE_SIZE - 1, y + 16, 1, 5, foam);
    }
    if (edges & COAST_SHORE_SOUTH) {
        fill_rect(r, x, y + TILE_SIZE - 2,
            TILE_SIZE - (channel ? 3 : 0), 2, wash);
        fill_rect(r, x + 3 + shift, y + TILE_SIZE - 2, 8, 1, foam);
        fill_rect(r, x + 15, y + TILE_SIZE - 1, 5, 1, foam);
    }
    if (edges & COAST_SHORE_WEST) {
        fill_rect(r, x, y + (channel ? 3 : 0), 2,
            TILE_SIZE - (channel ? 3 : 0), wash);
        fill_rect(r, x + 1, y + 3 + shift, 1, 8, foam);
        fill_rect(r, x, y + 16, 1, 5, foam);
    }
}

void draw_coast_channel(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int amber, int flooded) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    if (flooded) {
        draw_coast_deep_water(r, tile_x, tile_y, map_x, map_y);
    } else {
        draw_coast_sand(r, tile_x, tile_y, map_x, map_y, 1);
    }
    SDL_Color rim = amber ? (SDL_Color){183, 127, 51, 255} : (SDL_Color){48, 157, 179, 255};
    fill_rect(r, x, y, 3, 3, rim);
    fill_rect(r, x + TILE_SIZE - 3, y + TILE_SIZE - 3, 3, 3, rim);
}

void draw_coast_sluice(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_LEVER, (SDL_Color){91, 151, 132, 255});
}

void draw_coast_sluice_intake(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_INTAKE, (SDL_Color){91, 151, 132, 255});
}

void draw_coast_sluice_conduit(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_CONDUIT, (SDL_Color){91, 151, 132, 255});
}

void draw_coast_cache(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_CHEST, (SDL_Color){213, 177, 66, 255});
    fill_rect(r, tile_x * TILE_SIZE + 3, tile_y * TILE_SIZE + 20, 7, 1, (SDL_Color){42, 136, 119, 255});
}

void draw_coast_tide_control(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_LEVER, (SDL_Color){90, 231, 207, 255});
}

void draw_coast_beacon(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int lit) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_shallow_water(r, tile_x, tile_y, map_x, map_y);
    draw_world_prop(r, tile_x, tile_y, PROP_BRAZIER, (SDL_Color){116, 126, 98, 255});
    if (lit) {
        unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
            ((coast_tile_seed(map_x, map_y) >> 20) & 1u)) & 1u;
        draw_world_prop(r, tile_x, tile_y, frame == 0u ? PROP_SMALL_FLAME_0 : PROP_SMALL_FLAME_1, (SDL_Color){47, 221, 212, 255});
    } else {
        fill_rect(r, x + 9, y + 8, 7, 2, (SDL_Color){34, 42, 45, 255});
    }
}

static void draw_weapon_line(Renderer *r, int bx, int by, int dx, int dy, int f1, int s1, int f2, int s2, SDL_Color color) {
    int px = -dy;
    int py = dx;
    SDL_SetRenderDrawColor(r->sdl, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(r->sdl,
        bx + dx * f1 + px * s1, by + dy * f1 + py * s1,
        bx + dx * f2 + px * s2, by + dy * f2 + py * s2);
}

static int weapon_is_magic(const Item *weapon) {
    switch (weapon->visual_id) {
        case ITEM_VISUAL_MAGIC_LONG_SWORD:
        case ITEM_VISUAL_MAGIC_BATTLE_AXE:
        case ITEM_VISUAL_MAGIC_STAFF:
        case ITEM_VISUAL_SANDSTORM_STAFF:
        case ITEM_VISUAL_MAGIC_LONGBOW:
        case ITEM_VISUAL_MAGIC_DAGGER:
        case ITEM_VISUAL_MAGIC_GREATSWORD:
        case ITEM_VISUAL_DEMONIC_SWORD:
            return 1;
        default:
            return 0;
    }
}

static void draw_player_sword(Renderer *r, const Item *weapon, int bx, int by, int dx, int dy) {
    int length = 10;
    int width = 1;
    if (weapon->visual_id == ITEM_VISUAL_SHORT_SWORD) {
        length = 8;
    } else if (weapon->weapon_family == WEAPON_FAMILY_DAGGER) {
        length = 6;
    } else if (weapon->visual_id == ITEM_VISUAL_GREATSWORD ||
        weapon->visual_id == ITEM_VISUAL_MAGIC_GREATSWORD) {
        length = 11;
        width = 2;
    }

    int magic = weapon_is_magic(weapon);
    SDL_Color glow = {78, 222, 244, 255};
    SDL_Color blade = magic ? (SDL_Color){124, 151, 224, 255}
        : (SDL_Color){181, 194, 204, 255};
    SDL_Color edge = magic ? (SDL_Color){221, 250, 255, 255}
        : (SDL_Color){235, 239, 241, 255};
    SDL_Color guard = magic ? (SDL_Color){171, 91, 211, 255}
        : (SDL_Color){205, 160, 60, 255};
    SDL_Color grip = {78, 46, 31, 255};
    if (weapon->visual_id == ITEM_VISUAL_DEMONIC_SWORD) {
        glow = (SDL_Color){89, 216, 69, 255};
        blade = (SDL_Color){33, 57, 42, 255};
        edge = (SDL_Color){137, 239, 106, 255};
        guard = (SDL_Color){112, 60, 85, 255};
    }
    if (weapon->visual_id == ITEM_VISUAL_RUSTY_SWORD) {
        blade = (SDL_Color){151, 77, 45, 255};
        edge = (SDL_Color){190, 105, 59, 255};
    }
    if (strcmp(weapon->name, "Cryptblade") == 0) {
        glow = (SDL_Color){184, 134, 242, 255};
        blade = (SDL_Color){142, 144, 180, 255};
    }

    SDL_Color outline = {16, 20, 29, 255};
    draw_weapon_line(r, bx, by, dx, dy, 1, -width - 1, length, -width - 1, outline);
    draw_weapon_line(r, bx, by, dx, dy, 1, width + 1, length, width + 1, outline);
    if (magic) {
        draw_weapon_line(r, bx, by, dx, dy, 1, -width - 1,
            length, -width - 1, glow);
        draw_weapon_line(r, bx, by, dx, dy, 1, width + 1,
            length, width + 1, glow);
    }
    for (int side = -width; side <= width; side++) {
        draw_weapon_line(r, bx, by, dx, dy, 1, side, length, side,
            side == -width ? edge : blade);
    }
    draw_weapon_line(r, bx, by, dx, dy, -4, 0, 0, 0, grip);
    for (int band = -3; band < 0; band += 2) {
        draw_weapon_line(r, bx, by, dx, dy, band, -1, band, 1, guard);
    }
    draw_weapon_line(r, bx, by, dx, dy, 3, 0, length - 2, 0,
        magic ? glow : (SDL_Color){110, 128, 148, 255});
    draw_weapon_line(r, bx, by, dx, dy, 0, -3, 0, 3, guard);
    draw_weapon_line(r, bx, by, dx, dy, length, -width,
        length + 1, 0, edge);
    draw_weapon_line(r, bx, by, dx, dy, length, width,
        length + 1, 0, edge);
}

static void draw_player_axe(Renderer *r, const Item *weapon, int bx, int by, int dx, int dy) {
    int magic = weapon_is_magic(weapon);
    SDL_Color glow = {78, 226, 246, 255};
    SDL_Color handle = magic ? (SDL_Color){91, 48, 111, 255}
        : (SDL_Color){115, 68, 36, 255};
    SDL_Color head = magic ? (SDL_Color){113, 132, 207, 255}
        : (SDL_Color){157, 169, 177, 255};
    SDL_Color edge = magic ? (SDL_Color){217, 249, 255, 255}
        : (SDL_Color){225, 231, 233, 255};
    if (weapon->visual_id == ITEM_VISUAL_EMBERCLEAVER) {
        handle = (SDL_Color){88, 51, 34, 255};
        head = (SDL_Color){98, 100, 109, 255};
        edge = (SDL_Color){255, 174, 77, 255};
    }
    draw_weapon_line(r, bx, by, dx, dy, -4, 0, 9, 0, handle);
    draw_weapon_line(r, bx, by, dx, dy, -4, -1, 7, -1, (SDL_Color){36, 28, 29, 255});
    for (int forward = 7; forward <= 10; forward++) {
        int half = forward < 9 ? 4 : 3;
        draw_weapon_line(r, bx, by, dx, dy, forward, -half, forward, half, head);
    }
    draw_weapon_line(r, bx, by, dx, dy, 6, -1, 6, 1, edge);
    if (magic) {
        draw_weapon_line(r, bx, by, dx, dy, 7, -5, 10, -3, glow);
        draw_weapon_line(r, bx, by, dx, dy, 7, 5, 10, 3, glow);
    }
    draw_weapon_line(r, bx, by, dx, dy, 7, -4, 10, -2, head);
    draw_weapon_line(r, bx, by, dx, dy, 7, 4, 10, 2, head);
    draw_weapon_line(r, bx, by, dx, dy, 10, -2, 10, 2, edge);
}

static void draw_player_staff(Renderer *r, const Item *weapon, int bx, int by, int dx, int dy) {
    int magic = weapon_is_magic(weapon);
    int runed = weapon->visual_id == ITEM_VISUAL_RUNED_STAFF;
    SDL_Color wood = magic ? (SDL_Color){99, 53, 122, 255} : runed
        ? (SDL_Color){127, 108, 53, 255}
        : (SDL_Color){119, 72, 38, 255};
    SDL_Color gem = magic ? (SDL_Color){77, 223, 242, 255} : runed
        ? (SDL_Color){104, 219, 132, 255}
        : (SDL_Color){105, 181, 214, 255};
    SDL_Color core = {219, 252, 255, 255};
    if (weapon->visual_id == ITEM_VISUAL_SANDSTORM_STAFF) {
        wood = (SDL_Color){227, 180, 71, 255};
        gem = (SDL_Color){244, 126, 35, 255};
        core = (SDL_Color){255, 238, 157, 255};
    }
    draw_weapon_line(r, bx, by, dx, dy, -5, -1, 8, -1, (SDL_Color){28, 23, 34, 255});
    draw_weapon_line(r, bx, by, dx, dy, -5, 0, 8, 0, wood);
    for (int band = -2; band <= 6; band += 4) {
        draw_weapon_line(r, bx, by, dx, dy, band, 0, band, 1,
            runed || magic ? gem : (SDL_Color){186, 141, 63, 255});
    }
    draw_weapon_line(r, bx, by, dx, dy, 8, -1, 10, -1, gem);
    draw_weapon_line(r, bx, by, dx, dy, 8, 0, 11, 0, gem);
    draw_weapon_line(r, bx, by, dx, dy, 8, 1, 10, 1, gem);
    draw_weapon_line(r, bx, by, dx, dy, 8, -2, 11, 0, gem);
    draw_weapon_line(r, bx, by, dx, dy, 8, 2, 11, 0, gem);
    draw_weapon_line(r, bx, by, dx, dy, 9, -1, 9, 1,
        magic || runed ? core : gem);
}

static void draw_player_bow(Renderer *r, const Item *weapon, int bx, int by, int dx, int dy) {
    int magic = weapon_is_magic(weapon);
    int length = weapon->visual_id == ITEM_VISUAL_BOW ? 8 : 10;
    if (weapon->visual_id == ITEM_VISUAL_SHORTBOW) {
        length = 6;
    }
    SDL_Color wood = magic ? (SDL_Color){151, 101, 215, 255}
        : (SDL_Color){151, 100, 48, 255};
    SDL_Color string = magic ? (SDL_Color){208, 248, 255, 255}
        : (SDL_Color){217, 214, 191, 255};
    SDL_Color arrow = magic ? (SDL_Color){80, 228, 245, 255}
        : (SDL_Color){190, 201, 204, 255};
    if (strcmp(weapon->name, "Krakenbone Bow") == 0) {
        wood = (SDL_Color){192, 193, 158, 255};
    }
    if (weapon->visual_id == ITEM_VISUAL_CROSSBOW) {
        draw_weapon_line(r, bx, by, dx, dy, -5, -1, 8, -1, (SDL_Color){35, 27, 24, 255});
        draw_weapon_line(r, bx, by, dx, dy, -5, 0, 8, 0, wood);
        draw_weapon_line(r, bx, by, dx, dy, -5, 1, 8, 1, wood);
        draw_weapon_line(r, bx, by, dx, dy, 5, -6, 8, -3, arrow);
        draw_weapon_line(r, bx, by, dx, dy, 8, -3, 8, 3, arrow);
        draw_weapon_line(r, bx, by, dx, dy, 8, 3, 5, 6, arrow);
        draw_weapon_line(r, bx, by, dx, dy, 5, -6, 5, 6, string);
        draw_weapon_line(r, bx, by, dx, dy, 1, 0, 11, 0, arrow);
        draw_weapon_line(r, bx, by, dx, dy, 9, -2, 11, 0, arrow);
        draw_weapon_line(r, bx, by, dx, dy, 9, 2, 11, 0, arrow);
        return;
    }
    draw_weapon_line(r, bx, by, dx, dy, -length / 2, 3, -length / 2 + 2, 0, wood);
    draw_weapon_line(r, bx, by, dx, dy, -length / 2 + 2, 0, 0, -2, wood);
    draw_weapon_line(r, bx, by, dx, dy, 0, -2, length / 2 - 2, 0, wood);
    draw_weapon_line(r, bx, by, dx, dy, length / 2 - 2, 0, length / 2, 3, wood);
    draw_weapon_line(r, bx, by, dx, dy, -1, -1, 1, -1, string);
    draw_weapon_line(r, bx, by, dx, dy, -length / 2, 3,
        length / 2, 3, string);
    draw_weapon_line(r, bx, by, dx, dy, -4, 0, length, 0, arrow);
    draw_weapon_line(r, bx, by, dx, dy, length - 2, -2,
        length, 0, arrow);
    draw_weapon_line(r, bx, by, dx, dy, length - 2, 2,
        length, 0, arrow);
}

static void player_hand_position(int x, int y, int dx, int dy, int side, int *hx, int *hy) {
    if (dx != 0) {
        int near = dx * side > 0;
        *hx = x + (near ? 12 : 15);
        if (dx < 0) {
            *hx = x + 23 - (*hx - x);
        }
        *hy = y + (near ? 18 : 13);
    } else {
        *hx = x + TILE_SIZE / 2 - dy * 6 * side;
        *hy = y + (dy > 0 ? 15 : 13);
    }
}

static void draw_equipped_player_weapon(Renderer *r, const Item *weapon, int x, int y, int dx, int dy, int hand_side) {
    if (!weapon || weapon->type != ITEM_WEAPON) {
        return;
    }
    if (dx == 0 && dy == 0) {
        dy = -1;
    } else if (dx != 0) {
        dx = dx < 0 ? -1 : 1;
        dy = 0;
    } else {
        dy = dy < 0 ? -1 : 1;
    }

    int bx, by;
    player_hand_position(x, y, dx, dy, hand_side, &bx, &by);
    if (weapon->weapon_family == WEAPON_FAMILY_HAMMER) {
        draw_weapon_line(r, bx, by, dx, dy, 0, 0, 10, 0, (SDL_Color){112, 76, 44, 255});
        for (int n = 7; n <= 11; n++) {
            draw_weapon_line(r, bx, by, dx, dy, n, -4, n, 4, (SDL_Color){n == 7 ? 84 : 173, n == 7 ? 87 : 170, n == 7 ? 77 : 143, 255});
        }
        draw_weapon_line(r, bx, by, dx, dy, 8, 0, 10, 0, (SDL_Color){145, 205, 111, 255});
    } else if (weapon->weapon_family == WEAPON_FAMILY_AXE) {
        draw_player_axe(r, weapon, bx, by, dx, dy);
    } else if (weapon->weapon_family == WEAPON_FAMILY_STAFF) {
        draw_player_staff(r, weapon, bx, by, dx, dy);
    } else if (weapon->weapon_family == WEAPON_FAMILY_BOW) {
        draw_player_bow(r, weapon, bx, by, dx, dy);
    } else {
        draw_player_sword(r, weapon, bx, by, dx, dy);
    }
}

static void draw_equipped_player_shield(Renderer *r, const Item *shield, int x, int y, int dx, int dy) {
    if (!shield || shield->type != ITEM_SHIELD) {
        return;
    }
    if (dx == 0 && dy == 0) {
        dy = -1;
    }
    int bx, by;
    player_hand_position(x, y, dx, dy, -1, &bx, &by);
    SDL_Color rim = {181, 188, 194, 255};
    SDL_Color face = {83, 91, 104, 255};
    SDL_Color mark = {199, 151, 48, 255};
    if (shield->visual_id == ITEM_VISUAL_BUCKLER) {
        face = (SDL_Color){126, 83, 45, 255};
    } else if (shield->visual_id == ITEM_VISUAL_KITE_SHIELD) {
        face = (SDL_Color){57, 82, 143, 255};
    } else if (shield->visual_id == ITEM_VISUAL_TOWER_SHIELD) {
        face = (SDL_Color){119, 124, 130, 255};
        mark = (SDL_Color){190, 61, 47, 255};
    } else if (shield->visual_id == ITEM_VISUAL_LABYRINTH_AEGIS) {
        rim = (SDL_Color){207, 172, 100, 255};
        face = (SDL_Color){61, 71, 102, 255};
        mark = (SDL_Color){238, 226, 171, 255};
    } else if (shield->visual_id == ITEM_VISUAL_MAGIC_SHIELD) {
        rim = (SDL_Color){88, 230, 247, 255};
        face = (SDL_Color){79, 65, 153, 255};
        mark = (SDL_Color){221, 250, 255, 255};
    } else if (shield->visual_id == ITEM_VISUAL_CASTELLAN_BULWARK) {
        rim = (SDL_Color){174, 180, 196, 255};
        face = (SDL_Color){88, 48, 65, 255};
        mark = (SDL_Color){222, 181, 91, 255};
    }
    if (strcmp(shield->name, "Goblin King's Shield") == 0) {
        face = (SDL_Color){74, 115, 59, 255};
    } else if (strcmp(shield->name, "Stoneguard Shield") == 0) {
        rim = (SDL_Color){173, 180, 151, 255};
        face = (SDL_Color){84, 100, 77, 255};
        mark = (SDL_Color){226, 193, 105, 255};
    }
    static const char shapes[4][13][12] = {
        {"...........", "...ooooo...", "..oRRRRRo..", ".oRRfffRRo.", ".oRfffffRo.", ".oRffmffRo.", ".oRfmRmfRo.", ".oRffmffRo.", ".oRfffffRo.", ".oRRfffRRo.", "..oRRRRRo..", "...ooooo...", "..........."},
        {".ooooooooo.", "oRRRRRRRRRo", "oRfffffffRo", "oRfffmfffRo", "oRfffmfffRo", "oRfmmmmmfRo", "oRfffmfffRo", "oRfffmfffRo", ".oRffmffRo.", ".oRRfffRRo.", "..oRRfRRo..", "...oRRRo...", "....ooo...."},
        {"..ooooooo..", ".oRRRRRRRo.", "oRRfffffRRo", "oRfffmfffRo", "oRfffmfffRo", "oRfmmmmmfRo", "oRfffmfffRo", "oRfffmfffRo", "oRfffmfffRo", "oRRfffffRRo", ".oRRRRRRRo.", "..ooooooo..", "..........."},
        {".ooooooooo.", "oRRRRRRRRRo", "oRfffffffRo", "oRfmfffmfRo", "oRfmmfmmfRo", "oRffmmmffRo", "oRffmomffRo", "oRfffmfffRo", ".oRfffffRo.", ".oRRfmfRRo.", "..oRRmRRo..", "...oRRRo...", "....ooo...."}
    };
    int shape = shield->visual_id == ITEM_VISUAL_LABYRINTH_AEGIS ? 3 : shield->visual_id == ITEM_VISUAL_BUCKLER ? 0 :
        shield->visual_id == ITEM_VISUAL_TOWER_SHIELD || shield->visual_id == ITEM_VISUAL_CASTELLAN_BULWARK ? 2 : 1;
    for (int row = 0; row < 13; row++) {
        for (int col = 0; col < 11; col++) {
            char ink = shapes[shape][row][col];
            if (ink == '.' || ink == ' ') {
                continue;
            }
            SDL_Color color = ink == 'R' ? rim : ink == 'f' ? face : ink == 'm' ? mark : (SDL_Color){16, 20, 29, 255};
            fill_rect(r, bx - 5 + col, by - 6 + row, 1, 1, color);
        }
    }
}

static void player_body_palette(SDL_Color *palette, PlayerClass player_class, const Item *armor) {
    palette['o'] = (SDL_Color){19, 23, 35, 255};
    palette['k'] = (SDL_Color){49, 31, 31, 255};
    palette['l'] = (SDL_Color){105, 65, 38, 255};
    palette['L'] = (SDL_Color){163, 105, 57, 255};
    palette['s'] = (SDL_Color){161, 99, 72, 255};
    palette['f'] = (SDL_Color){217, 158, 112, 255};
    palette['F'] = (SDL_Color){244, 199, 148, 255};
    palette['e'] = (SDL_Color){21, 30, 40, 255};
    palette['m'] = (SDL_Color){79, 94, 116, 255};
    palette['M'] = (SDL_Color){151, 174, 191, 255};
    palette['H'] = (SDL_Color){225, 237, 240, 255};
    palette['g'] = (SDL_Color){130, 88, 33, 255};
    palette['G'] = (SDL_Color){212, 163, 65, 255};
    palette['T'] = (SDL_Color){252, 222, 131, 255};
    palette['r'] = (SDL_Color){110, 229, 232, 255};

    SDL_Color cloth = player_class == CLASS_ROGUE
        ? (SDL_Color){57, 101, 63, 255} : (SDL_Color){48, 86, 163, 255};
    palette['b'] = (SDL_Color){cloth.r / 2, cloth.g / 2, cloth.b / 2, 255};
    palette['c'] = cloth;
    palette['C'] = (SDL_Color){cloth.r + 34, cloth.g + 40, cloth.b + 36, 255};

    SDL_Color suit = cloth;
    if (armor && armor->armor_family == ARMOR_FAMILY_HEAVY) {
        suit = (SDL_Color){139, 150, 162, 255};
        if (armor->visual_id == ITEM_VISUAL_MAGIC_PLATE) {
            suit = (SDL_Color){92, 190, 218, 255};
        } else if (armor->visual_id == ITEM_VISUAL_SCALE_MAIL) {
            suit = (SDL_Color){165, 125, 70, 255};
        } else if (armor->visual_id == ITEM_VISUAL_PLATE_ARMOR) {
            suit = (SDL_Color){190, 197, 205, 255};
        }
    } else if (armor && armor->armor_family == ARMOR_FAMILY_LIGHT) {
        suit = (SDL_Color){102, 66, 39, 255};
        if (armor->visual_id == ITEM_VISUAL_SUNWARDEN_MANTLE) {
            suit = (SDL_Color){172, 119, 55, 255};
        } else if (armor->visual_id == ITEM_VISUAL_SHADOW_ARMOR) {
            suit = (SDL_Color){67, 39, 88, 255};
        } else if (armor->visual_id == ITEM_VISUAL_BRIARWEAVE_CLOAK) {
            suit = (SDL_Color){61, 129, 112, 255};
        } else if (armor->visual_id == ITEM_VISUAL_RANGER_CLOAK) {
            suit = (SDL_Color){58, 112, 55, 255};
        } else if (armor->visual_id == ITEM_VISUAL_STUDDED_LEATHER) {
            suit = (SDL_Color){128, 88, 48, 255};
        }
    } else if (armor && armor->armor_family == ARMOR_FAMILY_ROBE) {
        suit = (SDL_Color){46, 88, 168, 255};
        if (armor->visual_id == ITEM_VISUAL_CROWN_CHAPEL_VESTMENTS) {
            suit = (SDL_Color){100, 64, 135, 255};
            palette['G'] = (SDL_Color){232, 193, 106, 255};
            palette['T'] = (SDL_Color){255, 230, 172, 255};
        } else if (armor->visual_id == ITEM_VISUAL_PRISMWEAVE_ROBES) {
            suit = (SDL_Color){91, 107, 173, 255};
        } else if (armor->visual_id == ITEM_VISUAL_ARCHMAGE_ROBES) {
            suit = (SDL_Color){112, 68, 178, 255};
        } else if (armor->visual_id == ITEM_VISUAL_ENCHANTER_ROBES) {
            suit = (SDL_Color){78, 66, 163, 255};
        } else if (armor->visual_id == ITEM_VISUAL_RUNED_ROBES) {
            suit = (SDL_Color){38, 112, 133, 255};
        }
        palette['r'] = armor->rarity == ITEM_RARITY_COMMON
            ? (SDL_Color){91, 132, 201, 255} : (SDL_Color){91, 231, 225, 255};
    }
    if (armor) {
        if (strcmp(armor->name, "Necromancer's Cloak") == 0) {
            suit = (SDL_Color){62, 46, 86, 255};
        } else if (strcmp(armor->name, "Gravekeeper's Mantle") == 0) {
            suit = (SDL_Color){81, 91, 102, 255};
        } else if (strcmp(armor->name, "Dragon Scale Mantle") == 0) {
            suit = (SDL_Color){165, 84, 55, 255};
        } else if (strcmp(armor->name, "Tidecaller Robes") == 0) {
            suit = (SDL_Color){38, 132, 146, 255};
        }
    }
    palette['a'] = (SDL_Color){suit.r * 2 / 3, suit.g * 2 / 3, suit.b * 2 / 3, 255};
    palette['A'] = suit;
    palette['v'] = (SDL_Color){suit.r + (255 - suit.r) / 3,
        suit.g + (255 - suit.g) / 3, suit.b + (255 - suit.b) / 3, 255};
}

static void draw_player_body(Renderer *r, int x, int y, PlayerClass player_class, const Item *armor, int dx, int dy) {
    SDL_Color palette[128] = {{0}};
    player_body_palette(palette, player_class, armor);
    int view = dx != 0 ? 2 : (dy > 0 ? 0 : 1);
    const char (*pixels)[25] = player_art[player_class][view];
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = pixels[row][col];
            int end = col + 1;
            while (end < 24 && pixels[row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                int start = dx < 0 ? 24 - end : col;
                for (int pixel = 0; pixel < end - col; pixel++) {
                    char detail = ink;
                    int source_col = dx < 0 ? end - pixel - 1 : col + pixel;
                    if (armor && ink == 'A') {
                        if (armor->visual_id == ITEM_VISUAL_CHAIN_MAIL) {
                            detail = (row + source_col) % 2 ? 'a' : 'v';
                        } else if (armor->visual_id == ITEM_VISUAL_SCALE_MAIL) {
                            detail = (source_col + row / 3) % 3 == 0 ? 'a' : row % 3 == 0 ? 'v' : 'A';
                        } else if (armor->visual_id == ITEM_VISUAL_BRIARWEAVE_CLOAK && (row + source_col) % 5 == 0) {
                            detail = 'v';
                        } else if (armor->visual_id == ITEM_VISUAL_STUDDED_LEATHER && row % 3 == 0 && source_col % 3 == 0) {
                            detail = 'H';
                        } else if (armor->armor_family == ARMOR_FAMILY_ROBE && armor->rarity != ITEM_RARITY_COMMON && row % 4 == 0 && source_col % 3 == 0) {
                            detail = 'r';
                        }
                    }
                    fill_rect(r, x + start + pixel, y + row, 1, 1, palette[(unsigned char)detail]);
                }
            }
            col = end;
        }
    }
}

static void draw_player_off_hand(Renderer *r, const Item *item, int x, int y, int dx, int dy) {
    if (item && item->type == ITEM_SHIELD) {
        draw_equipped_player_shield(r, item, x, y, dx, dy);
    } else {
        draw_equipped_player_weapon(r, item, x, y, dx, dy, -1);
    }
}

// Draws the player with its top-left corner at pixel (x, y), so it can be
// shown partway between tiles while sliding.
void draw_player_at(Renderer *r, int x, int y, PlayerClass player_class, const Item *main_hand, const Item *off_hand, const Item *armor, int facing_dx, int facing_dy) {
    if (player_class < CLASS_WARRIOR || player_class > CLASS_ROGUE) {
        player_class = CLASS_WARRIOR;
    }
    if (facing_dx != 0) {
        facing_dx = facing_dx < 0 ? -1 : 1;
        facing_dy = 0;
    } else {
        facing_dy = facing_dy > 0 ? 1 : -1;
    }
    // Far equipment sits behind the body; near equipment overlaps the hands.
    int main_behind = facing_dy < 0 || facing_dx < 0;
    int off_behind = facing_dy < 0 || facing_dx > 0;
    if (main_behind) {
        draw_equipped_player_weapon(r, main_hand, x, y, facing_dx, facing_dy, 1);
    }
    if (off_behind) {
        draw_player_off_hand(r, off_hand, x, y, facing_dx, facing_dy);
    }
    draw_player_body(r, x, y, player_class, armor, facing_dx, facing_dy);
    if (!main_behind) {
        draw_equipped_player_weapon(r, main_hand, x, y, facing_dx, facing_dy, 1);
    }
    if (!off_behind) {
        draw_player_off_hand(r, off_hand, x, y, facing_dx, facing_dy);
    }
}

void draw_stairs_up(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_STAIRS_UP, (SDL_Color){195, 159, 77, 255});
}

void draw_stairs_down(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_STAIRS_DOWN, (SDL_Color){112, 90, 165, 255});
}

void draw_dungeon_return_stairs(Renderer *r, int tile_x, int tile_y, int open) {
    draw_floor(r, tile_x, tile_y);
    SDL_Color accent = open ? (SDL_Color){146, 223, 190, 255} : (SDL_Color){84, 68, 103, 255};
    draw_world_prop(r, tile_x, tile_y, PROP_STAIRS_UP, accent);
    if (!open) {
        fill_rect(r, tile_x * TILE_SIZE + 5, tile_y * TILE_SIZE + 11, 14, 3, (SDL_Color){126, 54, 153, 255});
        fill_rect(r, tile_x * TILE_SIZE + 10, tile_y * TILE_SIZE + 9, 4, 6, (SDL_Color){209, 111, 230, 255});
    }
}

void draw_return_exit(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_EXIT, (SDL_Color){96, 210, 190, 255});
}

void draw_broken_burial_seal(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_BROKEN_SEAL, (SDL_Color){185, 55, 192, 255});
}

void draw_restored_burial_seal(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_SEAL, (SDL_Color){226, 191, 82, 255});
}

void draw_locked_door(Renderer *r, int tile_x, int tile_y) {
    draw_wall(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_DOOR, (SDL_Color){146, 142, 164, 255});
}

void draw_dungeon_door_support(Renderer *r, int tile_x, int tile_y, unsigned int side, int crypt) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {88, 79, 110, 255};
    SDL_Color edge = {134, 119, 153, 255};
    SDL_Color shadow = {33, 29, 49, 255};
    SDL_Color accent = crypt ? (SDL_Color){202, 164, 58, 255} :
        (SDL_Color){151, 76, 99, 255};

    if (side == DUNGEON_EDGE_NORTH) {
        fill_rect(r, x + 2, y, 20, 10, stone);
        fill_rect(r, x + 1, y + 8, 22, 2, edge);
        fill_rect(r, x + 4, y + 10, 16, 1, shadow);
        fill_rect(r, x + 9, y + 4, 6, 2, accent);
    } else if (side == DUNGEON_EDGE_EAST) {
        fill_rect(r, x + 14, y + 3, 10, 18, stone);
        fill_rect(r, x + 12, y + 1, 12, 3, edge);
        fill_rect(r, x + 12, y + 20, 12, 3, edge);
        fill_rect(r, x + 13, y + 4, 1, 16, shadow);
        fill_rect(r, x + 21, y + 9, 2, 6, accent);
    } else if (side == DUNGEON_EDGE_SOUTH) {
        fill_rect(r, x + 2, y + 14, 20, 10, stone);
        fill_rect(r, x + 1, y + 14, 22, 2, edge);
        fill_rect(r, x + 4, y + 13, 16, 1, shadow);
        fill_rect(r, x + 9, y + 18, 6, 2, accent);
    } else if (side == DUNGEON_EDGE_WEST) {
        fill_rect(r, x, y + 3, 10, 18, stone);
        fill_rect(r, x, y + 1, 12, 3, edge);
        fill_rect(r, x, y + 20, 12, 3, edge);
        fill_rect(r, x + 10, y + 4, 1, 16, shadow);
        fill_rect(r, x + 1, y + 9, 2, 6, accent);
    }
}

void draw_dungeon_key(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_KEY, (SDL_Color){226, 184, 54, 255});
}

void draw_crypt_door(Renderer *r, int tile_x, int tile_y) {
    draw_wall(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_DOOR, (SDL_Color){202, 164, 58, 255});
}

void draw_crypt_key(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_KEY, (SDL_Color){146, 166, 184, 255});
}

void draw_crypt_cache(Renderer *r, int tile_x, int tile_y) {
    draw_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_CHEST, (SDL_Color){226, 184, 54, 255});
}

void draw_portal(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_PORTAL, (SDL_Color){86, 174, 244, 255});
}

static void draw_dungeon_enemy_body(Renderer *r, int tile_x, int tile_y, int art) {
    static const SDL_Color bodies[DUNGEON_ART_COUNT] = {
        {210, 210, 190, 255}, {132, 146, 108, 255},
        {92, 116, 48, 255}, {142, 151, 141, 255},
        {54, 32, 72, 255}, {112, 224, 232, 255}, {172, 166, 134, 255}
    };
    static const SDL_Color clothes[DUNGEON_ART_COUNT] = {
        {92, 62, 38, 255}, {100, 48, 54, 255},
        {84, 48, 28, 255}, {92, 62, 38, 255},
        {92, 42, 126, 255}, {54, 112, 132, 255}, {62, 28, 92, 255}
    };
    static const SDL_Color eyes[DUNGEON_ART_COUNT] = {
        {18, 18, 35, 255}, {226, 207, 91, 255},
        {232, 166, 34, 255}, {34, 28, 28, 255},
        {244, 48, 62, 255}, {176, 242, 238, 255}, {78, 232, 76, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){20, 24, 30, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['e'] = (SDL_Color){18, 18, 35, 255};
    palette['y'] = eyes[art];
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['l'] = (SDL_Color){73, 43, 27, 255};
    palette['k'] = (SDL_Color){43, 31, 27, 255};
    palette['L'] = (SDL_Color){115, 73, 40, 255};
    palette['K'] = (SDL_Color){159, 112, 66, 255};
    palette['m'] = (SDL_Color){83, 94, 105, 255};
    palette['M'] = (SDL_Color){157, 174, 182, 255};
    palette['N'] = (SDL_Color){224, 235, 234, 255};
    palette['f'] = (SDL_Color){232, 224, 188, 255};
    palette['u'] = (SDL_Color){66, 52, 39, 255};
    palette['U'] = (SDL_Color){108, 89, 61, 255};
    palette['w'] = (SDL_Color){86, 58, 32, 255};
    palette['W'] = (SDL_Color){139, 100, 51, 255};
    palette['g'] = (SDL_Color){132, 88, 31, 255};
    palette['G'] = (SDL_Color){212, 168, 71, 255};
    palette['r'] = (SDL_Color){99, 51, 44, 255};
    palette['R'] = (SDL_Color){150, 70, 62, 255};
    palette['p'] = (SDL_Color){78, 232, 76, 255};
    palette['P'] = (SDL_Color){183, 255, 157, 255};
    if (art == DUNGEON_ART_SKELETON) {
        palette['s'] = (SDL_Color){145, 151, 145, 255};
    } else if (art == DUNGEON_ART_WRAITH) {
        palette['s'] = (SDL_Color){54, 112, 132, 255};
        palette['H'] = (SDL_Color){176, 242, 238, 255};
        palette['e'] = (SDL_Color){12, 20, 38, 255};
    }
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = dungeon_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && dungeon_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

void draw_skeleton(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_SKELETON);
}

void draw_armored_guard(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_ARMORED_GUARD);
}

void draw_zombie(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_ZOMBIE);
}

void draw_crypt_bat(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_CRYPT_BAT);
}

void draw_wraith(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_WRAITH);
}

void draw_necromancer(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_CRYPT_CONJURER);
}

void draw_giant(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_enemy_body(r, tile_x, tile_y, DUNGEON_ART_GIANT);
}

static void draw_dungeon_boss(Renderer *r, int tx, int ty, int art) {
    static const SDL_Color bodies[BOSS_ART_COUNT] = {
        {208, 204, 170, 255}, {174, 42, 24, 255}
    };
    static const SDL_Color clothes[BOSS_ART_COUNT] = {
        {55, 35, 88, 255}, {92, 24, 20, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){22, 19, 27, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 4,
        body.g + (255 - body.g) / 4, body.b + (255 - body.b) / 4, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){27, 18, 24, 255};
    palette['y'] = (SDL_Color){250, 76, 36, 255};
    palette['m'] = (SDL_Color){52, 58, 68, 255};
    palette['M'] = (SDL_Color){116, 133, 147, 255};
    palette['N'] = (SDL_Color){202, 216, 218, 255};
    palette['f'] = (SDL_Color){148, 129, 89, 255};
    palette['F'] = (SDL_Color){225, 210, 166, 255};
    palette['a'] = (SDL_Color){48, 22, 30, 255};
    palette['A'] = (SDL_Color){86, 30, 37, 255};
    palette['h'] = (SDL_Color){121, 49, 48, 255};
    palette['g'] = (SDL_Color){137, 91, 34, 255};
    palette['G'] = (SDL_Color){224, 177, 61, 255};
    palette['l'] = (SDL_Color){53, 36, 25, 255};
    palette['L'] = (SDL_Color){105, 69, 39, 255};
    palette['K'] = (SDL_Color){147, 106, 60, 255};
    palette['k'] = (SDL_Color){30, 28, 27, 255};
    palette['R'] = (SDL_Color){224, 66, 64, 255};
    palette['p'] = (SDL_Color){116, 55, 179, 255};
    palette['P'] = (SDL_Color){209, 151, 255, 255};
    if (art == BOSS_ART_LICH_KING) {
        palette['y'] = (SDL_Color){179, 106, 255, 255};
        palette['g'] = (SDL_Color){91, 73, 120, 255};
        palette['G'] = (SDL_Color){171, 156, 190, 255};
    } else if (art == BOSS_ART_RED_DRAGON) {
        palette['y'] = (SDL_Color){255, 202, 71, 255};
        palette['a'] = (SDL_Color){69, 24, 26, 255};
        palette['A'] = (SDL_Color){119, 34, 27, 255};
        palette['p'] = (SDL_Color){255, 109, 22, 255};
        palette['P'] = (SDL_Color){255, 214, 63, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = dungeon_boss_art[art][row][col];
            int end = col + 1;
            while (end < 24 && dungeon_boss_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

void draw_lich_king(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_boss(r, tile_x, tile_y, BOSS_ART_LICH_KING);
}

void draw_red_dragon(Renderer *r, int tile_x, int tile_y) {
    draw_dungeon_boss(r, tile_x, tile_y, BOSS_ART_RED_DRAGON);
}

static void draw_forest_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_PIXIE: art = FOREST_ART_PIXIE; break;
        case ENEMY_BLIGHTED_WOLF: art = FOREST_ART_BLIGHTED_WOLF; break;
        case ENEMY_GIANT_SPIDER: art = FOREST_ART_GIANT_SPIDER; break;
        case ENEMY_DARK_ELF: art = FOREST_ART_DARK_ELF; break;
        case ENEMY_GIANT_WURM: art = FOREST_ART_GIANT_WURM; break;
        case ENEMY_FOREST_TROLL: art = FOREST_ART_FOREST_TROLL; break;
        case ENEMY_FOREST_NECROMANCER: art = FOREST_ART_FOREST_NECROMANCER; break;
        default: return;
    }
    static const SDL_Color bodies[FOREST_ART_COUNT] = {
        {238, 189, 84, 255}, {83, 87, 108, 255}, {106, 64, 103, 255},
        {185, 166, 178, 255}, {127, 103, 65, 255}, {92, 111, 98, 255}, {208, 195, 163, 255}
    };
    static const SDL_Color clothes[FOREST_ART_COUNT] = {
        {100, 72, 133, 255}, {83, 87, 108, 255}, {106, 64, 103, 255},
        {83, 63, 109, 255}, {182, 151, 90, 255}, {105, 73, 47, 255}, {69, 53, 94, 255}
    };
    static const SDL_Color eyes[FOREST_ART_COUNT] = {
        {213, 238, 203, 255}, {125, 210, 102, 255}, {239, 117, 88, 255},
        {219, 111, 95, 255}, {238, 98, 75, 255}, {239, 175, 74, 255}, {151, 235, 110, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){19, 24, 30, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['e'] = (SDL_Color){18, 18, 32, 255};
    palette['y'] = eyes[art];
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['b'] = (SDL_Color){75, 120, 126, 255};
    palette['B'] = (SDL_Color){127, 181, 177, 255};
    palette['t'] = (SDL_Color){213, 238, 203, 255};
    palette['p'] = (SDL_Color){125, 210, 102, 255};
    palette['P'] = (SDL_Color){190, 245, 148, 255};
    palette['f'] = (SDL_Color){225, 219, 186, 255};
    palette['w'] = (SDL_Color){86, 58, 32, 255};
    palette['W'] = (SDL_Color){155, 116, 64, 255};
    palette['l'] = (SDL_Color){69, 43, 32, 255};
    palette['L'] = (SDL_Color){115, 73, 40, 255};
    palette['K'] = (SDL_Color){159, 112, 66, 255};
    palette['k'] = (SDL_Color){43, 31, 27, 255};
    palette['G'] = (SDL_Color){212, 168, 71, 255};
    palette['u'] = (SDL_Color){48, 65, 44, 255};
    palette['U'] = (SDL_Color){102, 129, 72, 255};
    palette['N'] = (SDL_Color){213, 226, 223, 255};
    if (art == FOREST_ART_BLIGHTED_WOLF) {
        palette['H'] = (SDL_Color){133, 139, 153, 255};
    } else if (art == FOREST_ART_FOREST_NECROMANCER) {
        palette['p'] = (SDL_Color){151, 235, 110, 255};
        palette['P'] = (SDL_Color){214, 255, 179, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = forest_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && forest_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_mountain_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_GOBLIN_SCOUT: art = MOUNTAIN_ART_GOBLIN_SCOUT; break;
        case ENEMY_GOBLIN_ARCHER: art = MOUNTAIN_ART_GOBLIN_ARCHER; break;
        case ENEMY_GOBLIN_BOMBER: art = MOUNTAIN_ART_GOBLIN_BOMBER; break;
        case ENEMY_TUNNEL_SPIDER: art = MOUNTAIN_ART_TUNNEL_SPIDER; break;
        case ENEMY_CAVE_TROLL: art = MOUNTAIN_ART_CAVE_TROLL; break;
        case ENEMY_HOBGOBLIN_GUARD: art = MOUNTAIN_ART_HOBGOBLIN_GUARD; break;
        case ENEMY_GOBLIN_SHAMAN: art = MOUNTAIN_ART_GOBLIN_SHAMAN; break;
        case ENEMY_MOUNTAIN_GOBLIN_KING: art = MOUNTAIN_ART_MOUNTAIN_GOBLIN_KING; break;
        default: return;
    }
    static const SDL_Color bodies[MOUNTAIN_ART_COUNT] = {
        {91, 121, 45, 255}, {91, 121, 45, 255}, {91, 121, 45, 255}, {103, 91, 88, 255},
        {103, 91, 88, 255}, {91, 121, 45, 255}, {91, 121, 45, 255}, {91, 121, 45, 255}
    };
    static const SDL_Color clothes[MOUNTAIN_ART_COUNT] = {
        {128, 48, 30, 255}, {128, 48, 30, 255}, {128, 48, 30, 255}, {128, 48, 30, 255},
        {85, 62, 45, 255}, {128, 48, 30, 255}, {128, 48, 30, 255}, {105, 35, 32, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){22, 23, 29, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['e'] = (SDL_Color){18, 18, 29, 255};
    palette['y'] = (SDL_Color){244, 67, 20, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['m'] = (SDL_Color){68, 72, 82, 255};
    palette['M'] = (SDL_Color){133, 141, 150, 255};
    palette['N'] = (SDL_Color){208, 220, 223, 255};
    palette['b'] = (SDL_Color){28, 28, 35, 255};
    palette['B'] = (SDL_Color){53, 54, 66, 255};
    palette['t'] = (SDL_Color){163, 151, 126, 255};
    palette['p'] = (SDL_Color){244, 67, 20, 255};
    palette['P'] = (SDL_Color){255, 208, 99, 255};
    palette['f'] = (SDL_Color){225, 219, 186, 255};
    palette['w'] = (SDL_Color){86, 58, 32, 255};
    palette['W'] = (SDL_Color){155, 116, 64, 255};
    palette['l'] = (SDL_Color){69, 43, 32, 255};
    palette['L'] = (SDL_Color){115, 73, 40, 255};
    palette['K'] = (SDL_Color){159, 112, 66, 255};
    palette['k'] = (SDL_Color){43, 31, 27, 255};
    palette['g'] = (SDL_Color){133, 89, 35, 255};
    palette['G'] = (SDL_Color){212, 168, 71, 255};
    palette['u'] = (SDL_Color){47, 43, 35, 255};
    palette['U'] = (SDL_Color){100, 81, 50, 255};
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = mountain_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && mountain_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_coast_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_ILLUSION: art = COAST_ART_ILLUSION; break;
        case ENEMY_MERFOLK: art = COAST_ART_MERFOLK; break;
        case ENEMY_SIREN: art = COAST_ART_SIREN; break;
        case ENEMY_GIANT_CRAB: art = COAST_ART_GIANT_CRAB; break;
        case ENEMY_ANIMATED_STATUE: art = COAST_ART_ANIMATED_STATUE; break;
        case ENEMY_WATER_ELEMENTAL: art = COAST_ART_WATER_ELEMENTAL; break;
        case ENEMY_SEA_SERPENT: art = COAST_ART_SEA_SERPENT; break;
        case ENEMY_DROWNED_QUEEN: art = COAST_ART_DROWNED_QUEEN; break;
        default: return;
    }
    static const SDL_Color bodies[COAST_ART_COUNT] = {
        {57, 48, 101, 255}, {57, 155, 151, 255}, {164, 214, 198, 255}, {211, 92, 67, 255},
        {118, 141, 128, 255}, {44, 151, 194, 255}, {49, 161, 153, 255}, {159, 214, 199, 255}
    };
    static const SDL_Color clothes[COAST_ART_COUNT] = {
        {109, 94, 169, 255}, {39, 97, 114, 255}, {133, 88, 144, 255}, {127, 48, 45, 255},
        {62, 85, 83, 255}, {33, 85, 147, 255}, {31, 94, 99, 255}, {43, 65, 109, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){15, 25, 38, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 3,
        cloth.g + (255 - cloth.g) / 3, cloth.b + (255 - cloth.b) / 3, 255};
    palette['e'] = (SDL_Color){11, 22, 33, 255};
    palette['y'] = (SDL_Color){119, 245, 219, 255};
    palette['f'] = (SDL_Color){192, 248, 233, 255};
    palette['u'] = (SDL_Color){44, 40, 72, 255};
    palette['U'] = (SDL_Color){92, 65, 114, 255};
    palette['r'] = (SDL_Color){39, 97, 114, 255};
    palette['R'] = (SDL_Color){89, 180, 191, 255};
    palette['g'] = (SDL_Color){135, 99, 50, 255};
    palette['G'] = (SDL_Color){211, 172, 94, 255};
    palette['m'] = (SDL_Color){53, 115, 84, 255};
    palette['b'] = (SDL_Color){223, 205, 157, 255};
    palette['p'] = (SDL_Color){229, 235, 206, 255};
    if (type == ENEMY_SEA_SERPENT || type == ENEMY_DROWNED_QUEEN) {
        palette['r'] = (SDL_Color){144, 62, 62, 255};
        palette['R'] = (SDL_Color){225, 111, 111, 255};
    }
    if (type == ENEMY_SEA_SERPENT) {
        palette['H'] = (SDL_Color){162, 211, 162, 255};
        palette['y'] = (SDL_Color){255, 191, 99, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = coast_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && coast_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_swamp_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_GIANT_RAT: art = SWAMP_ART_GIANT_RAT; break;
        case ENEMY_BANDIT: art = SWAMP_ART_BANDIT; break;
        case ENEMY_VAMPIRE: art = SWAMP_ART_VAMPIRE; break;
        case ENEMY_SWAMP_DEMON: art = SWAMP_ART_SWAMP_DEMON; break;
        case ENEMY_MINOTAUR: art = SWAMP_ART_MINOTAUR; break;
        default: return;
    }
    static const SDL_Color bodies[SWAMP_ART_COUNT] = {
        {80, 77, 65, 255}, {174, 128, 93, 255}, {151, 145, 126, 255},
        {63, 98, 66, 255}, {132, 77, 45, 255}
    };
    static const SDL_Color clothes[SWAMP_ART_COUNT] = {
        {80, 77, 65, 255}, {47, 57, 38, 255}, {92, 25, 46, 255},
        {25, 47, 45, 255}, {95, 53, 33, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){13, 21, 23, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){14, 15, 20, 255};
    palette['y'] = (SDL_Color){231, 76, 48, 255};
    palette['u'] = (SDL_Color){29, 25, 32, 255};
    palette['U'] = (SDL_Color){68, 48, 39, 255};
    palette['f'] = (SDL_Color){228, 206, 157, 255};
    palette['t'] = (SDL_Color){92, 55, 53, 255};
    palette['T'] = (SDL_Color){149, 96, 91, 255};
    palette['w'] = (SDL_Color){155, 145, 121, 255};
    palette['l'] = (SDL_Color){55, 35, 28, 255};
    palette['L'] = (SDL_Color){113, 73, 43, 255};
    palette['K'] = (SDL_Color){165, 118, 65, 255};
    palette['k'] = (SDL_Color){33, 28, 29, 255};
    palette['m'] = (SDL_Color){72, 86, 96, 255};
    palette['M'] = (SDL_Color){161, 176, 186, 255};
    palette['N'] = (SDL_Color){221, 229, 225, 255};
    palette['g'] = (SDL_Color){133, 92, 37, 255};
    palette['G'] = (SDL_Color){203, 164, 72, 255};
    palette['a'] = (SDL_Color){37, 68, 44, 255};
    palette['A'] = (SDL_Color){109, 132, 58, 255};
    if (type == ENEMY_BANDIT) {
        palette['y'] = (SDL_Color){174, 169, 117, 255};
    }
    if (type == ENEMY_SWAMP_DEMON) {
        palette['y'] = (SDL_Color){226, 232, 99, 255};
        palette['f'] = (SDL_Color){181, 185, 133, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = swamp_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && swamp_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_dragonspine_road_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_DRAKE: art = DRAGON_ROAD_ART_DRAKE; break;
        case ENEMY_FIRE_ELEMENTAL: art = DRAGON_ROAD_ART_FIRE_ELEMENTAL; break;
        case ENEMY_ROAD_ARCHER: art = DRAGON_ROAD_ART_ROAD_ARCHER; break;
        case ENEMY_HORSEMAN: art = DRAGON_ROAD_ART_HORSEMAN; break;
        default: return;
    }
    static const SDL_Color bodies[DRAGON_ROAD_ART_COUNT] = {
        {82, 141, 150, 255}, {237, 111, 39, 255}, {162, 121, 91, 255}, {137, 87, 58, 255}
    };
    static const SDL_Color clothes[DRAGON_ROAD_ART_COUNT] = {
        {41, 82, 101, 255}, {164, 54, 31, 255}, {132, 38, 39, 255}, {132, 38, 39, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){22, 25, 32, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){18, 20, 28, 255};
    palette['y'] = (SDL_Color){248, 193, 75, 255};
    palette['f'] = (SDL_Color){228, 219, 187, 255};
    palette['p'] = (SDL_Color){245, 134, 38, 255};
    palette['P'] = (SDL_Color){255, 231, 137, 255};
    palette['t'] = (SDL_Color){43, 83, 101, 255};
    palette['T'] = (SDL_Color){106, 172, 180, 255};
    palette['u'] = (SDL_Color){25, 23, 26, 255};
    palette['U'] = (SDL_Color){57, 41, 32, 255};
    palette['w'] = (SDL_Color){74, 46, 30, 255};
    palette['W'] = (SDL_Color){151, 111, 64, 255};
    palette['l'] = (SDL_Color){51, 32, 28, 255};
    palette['L'] = (SDL_Color){107, 67, 41, 255};
    palette['K'] = (SDL_Color){162, 114, 69, 255};
    palette['k'] = (SDL_Color){32, 26, 27, 255};
    palette['m'] = (SDL_Color){71, 82, 94, 255};
    palette['M'] = (SDL_Color){169, 170, 160, 255};
    palette['N'] = (SDL_Color){224, 233, 226, 255};
    palette['g'] = (SDL_Color){138, 96, 43, 255};
    palette['G'] = (SDL_Color){222, 172, 83, 255};
    palette['b'] = (SDL_Color){139, 94, 69, 255};
    palette['B'] = (SDL_Color){192, 146, 107, 255};
    if (type == ENEMY_FIRE_ELEMENTAL) {
        palette['s'] = (SDL_Color){164, 54, 31, 255};
        palette['H'] = (SDL_Color){255, 200, 76, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = dragonspine_road_art[art][row][col];
            int end = col + 1;
            while (end < 24 && dragonspine_road_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_frostfell_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_ICE_WOLF: art = FROST_ART_ICE_WOLF; break;
        case ENEMY_FROST_ARCHER: art = FROST_ART_FROST_ARCHER; break;
        case ENEMY_YETI: art = FROST_ART_YETI; break;
        case ENEMY_FROST_WRAITH: art = FROST_ART_FROST_WRAITH; break;
        case ENEMY_ICE_GOLEM: art = FROST_ART_ICE_GOLEM; break;
        case ENEMY_FROST_GIANT: art = FROST_ART_FROST_GIANT; break;
        default: return;
    }
    static const SDL_Color bodies[FROST_ART_COUNT] = {
        {226, 234, 242, 255}, {190, 150, 120, 255}, {226, 234, 242, 255},
        {118, 178, 224, 255}, {136, 194, 230, 255}, {112, 152, 196, 255}
    };
    static const SDL_Color clothes[FROST_ART_COUNT] = {
        {154, 176, 200, 255}, {204, 216, 230, 255}, {154, 176, 200, 255},
        {118, 178, 224, 255}, {72, 128, 176, 255}, {130, 100, 74, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){24, 34, 52, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){15, 25, 43, 255};
    palette['y'] = (SDL_Color){170, 246, 255, 255};
    palette['j'] = (SDL_Color){66, 86, 123, 255};
    palette['J'] = (SDL_Color){116, 146, 178, 255};
    palette['f'] = (SDL_Color){226, 234, 242, 255};
    palette['F'] = (SDL_Color){154, 176, 200, 255};
    palette['p'] = (SDL_Color){230, 238, 243, 255};
    palette['i'] = (SDL_Color){72, 128, 176, 255};
    palette['I'] = (SDL_Color){210, 238, 252, 255};
    palette['l'] = (SDL_Color){50, 42, 38, 255};
    palette['L'] = (SDL_Color){108, 81, 59, 255};
    palette['K'] = (SDL_Color){153, 116, 76, 255};
    palette['k'] = (SDL_Color){33, 34, 43, 255};
    palette['g'] = (SDL_Color){130, 105, 52, 255};
    palette['G'] = (SDL_Color){207, 177, 91, 255};
    if (type == ENEMY_ICE_WOLF) {
        palette['y'] = (SDL_Color){52, 156, 232, 255};
    }
    if (type == ENEMY_FROST_WRAITH) {
        palette['J'] = (SDL_Color){28, 48, 80, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = frostfell_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && frostfell_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

// Both rendering paths use the same 96-pixel footprint above the lake tile.
static void draw_frost_kraken(Renderer *r, int tx, int ty) {
    SDL_Rect destination = {tx * TILE_SIZE - 36, ty * TILE_SIZE - 48, 96, 96};
    if (r->frost_kraken_texture) {
        SDL_RenderCopy(r->sdl, r->frost_kraken_texture, NULL, &destination);
        return;
    }
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){24, 28, 55, 255};
    palette['s'] = (SDL_Color){40, 53, 94, 255};
    palette['S'] = (SDL_Color){45, 100, 131, 255};
    palette['H'] = (SDL_Color){76, 148, 174, 255};
    palette['I'] = (SDL_Color){147, 199, 234, 255};
    palette['f'] = (SDL_Color){234, 245, 255, 255};
    palette['p'] = (SDL_Color){190, 147, 216, 255};
    palette['y'] = (SDL_Color){115, 240, 248, 255};
    palette['w'] = (SDL_Color){23, 60, 83, 255};
    for (int row = 0; row < 48; row++) {
        for (int col = 0; col < 48;) {
            char ink = frost_kraken_art[row][col];
            int end = col + 1;
            while (end < 48 && frost_kraken_art[row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, destination.x + col * 2, destination.y + row * 2, (end - col) * 2, 2, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_desert_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_SCARAB: art = DESERT_ART_SCARAB; break;
        case ENEMY_VIPER: art = DESERT_ART_VIPER; break;
        case ENEMY_MUMMY: art = DESERT_ART_MUMMY; break;
        case ENEMY_DJINN: art = DESERT_ART_DJINN; break;
        case ENEMY_GOLEM: art = DESERT_ART_GOLEM; break;
        case ENEMY_DESERT_PHARAOH: art = DESERT_ART_DESERT_PHARAOH; break;
        default: return;
    }
    static const SDL_Color bodies[DESERT_ART_COUNT] = {
        {61, 115, 83, 255}, {153, 66, 44, 255}, {213, 195, 150, 255},
        {58, 165, 171, 255}, {165, 112, 66, 255}, {230, 214, 169, 255}
    };
    static const SDL_Color clothes[DESERT_ART_COUNT] = {
        {61, 115, 83, 255}, {153, 66, 44, 255}, {141, 120, 86, 255},
        {38, 92, 115, 255}, {104, 69, 46, 255}, {42, 85, 138, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){43, 30, 24, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){29, 22, 24, 255};
    palette['y'] = (SDL_Color){107, 245, 221, 255};
    palette['b'] = (SDL_Color){141, 120, 86, 255};
    palette['f'] = (SDL_Color){230, 214, 169, 255};
    palette['u'] = (SDL_Color){23, 44, 67, 255};
    palette['U'] = (SDL_Color){42, 85, 138, 255};
    palette['r'] = (SDL_Color){142, 38, 46, 255};
    palette['R'] = (SDL_Color){225, 65, 76, 255};
    palette['g'] = (SDL_Color){142, 99, 41, 255};
    palette['G'] = (SDL_Color){227, 180, 71, 255};
    palette['l'] = (SDL_Color){59, 41, 30, 255};
    palette['L'] = (SDL_Color){114, 80, 46, 255};
    palette['K'] = (SDL_Color){161, 121, 69, 255};
    palette['k'] = (SDL_Color){44, 31, 28, 255};
    if (type == ENEMY_VIPER) {
        palette['y'] = (SDL_Color){244, 187, 80, 255};
    }
    if (type == ENEMY_MUMMY) {
        palette['y'] = (SDL_Color){215, 70, 38, 255};
    }
    if (type == ENEMY_GOLEM) {
        palette['b'] = (SDL_Color){104, 69, 46, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = desert_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && desert_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

static void draw_ashen_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    int ember = -1;
    switch (type) {
        case ENEMY_CINDER_IMP: art = ASHEN_ART_CINDER_IMP; ember = 0; break;
        case ENEMY_ASH_HOUND: art = ASHEN_ART_ASH_HOUND; ember = 1; break;
        case ENEMY_OBSIDIAN_GUARDIAN: art = ASHEN_ART_OBSIDIAN_GUARDIAN; break;
        case ENEMY_CINDER_LORD: art = ASHEN_ART_CINDER_LORD; ember = 2; break;
        default: return;
    }
    static const SDL_Color bodies[ASHEN_ART_COUNT] = {
        {174, 65, 40, 255}, {42, 38, 47, 255}, {42, 38, 47, 255}, {42, 38, 47, 255}
    };
    static const SDL_Color clothes[ASHEN_ART_COUNT] = {
        {42, 38, 47, 255}, {42, 38, 47, 255}, {42, 38, 47, 255}, {79, 36, 46, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){15, 14, 20, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){19, 14, 23, 255};
    palette['y'] = (SDL_Color){255, 204, 92, 255};
    palette['m'] = (SDL_Color){61, 49, 61, 255};
    palette['M'] = (SDL_Color){101, 85, 87, 255};
    palette['N'] = (SDL_Color){167, 148, 154, 255};
    palette['f'] = (SDL_Color){177, 157, 131, 255};
    palette['u'] = (SDL_Color){35, 27, 40, 255};
    palette['U'] = (SDL_Color){82, 62, 66, 255};
    palette['p'] = (SDL_Color){239, 96, 38, 255};
    palette['P'] = (SDL_Color){255, 204, 92, 255};
    palette['l'] = (SDL_Color){62, 40, 31, 255};
    palette['L'] = (SDL_Color){123, 83, 53, 255};
    if (type == ENEMY_CINDER_IMP) {
        palette['H'] = (SDL_Color){239, 96, 38, 255};
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    int flicker = (int)((SDL_GetTicks() / AMBIENT_FRAME_MS) & 1u);
    for (int layer = 0; layer < 2; layer++) {
        if (layer == 1 && ember < 0) {
            continue;
        }
        const char (*rows)[25] = layer == 0 ? ashen_enemy_art[art] : ashen_ember_art[ember];
        int offset = layer == 0 ? 0 : type == ENEMY_CINDER_LORD ? flicker : -flicker;
        for (int row = 0; row < 24; row++) {
            for (int col = 0; col < 24;) {
                char ink = rows[row][col];
                int end = col + 1;
                while (end < 24 && rows[row][end] == ink) {
                    end++;
                }
                if (ink != '.') {
                    fill_rect(r, x + col, y + row + offset, end - col, 1, palette[(unsigned char)ink]);
                }
                col = end;
            }
        }
    }
}

static void draw_glassdeep_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    if (draw_stonewake_enemy(r, tx, ty, type)) {
        return;
    }
    int art;
    switch (type) {
        case ENEMY_CRYSTAL_SPIDER: art = GLASS_ART_CRYSTAL_SPIDER; break;
        case ENEMY_BLIND_STALKER: art = GLASS_ART_BLIND_STALKER; break;
        case ENEMY_SHARD_GOLEM:
        case ENEMY_IRON_SENTINEL: art = GLASS_ART_SHARD_GOLEM; break;
        case ENEMY_PRISM_SOVEREIGN: art = GLASS_ART_PRISM_SOVEREIGN; break;
        default: return;
    }
    static const SDL_Color bodies[GLASS_ART_COUNT] = {
        {86, 181, 216, 255}, {163, 170, 185, 255}, {86, 181, 216, 255}, {146, 109, 213, 255}
    };
    static const SDL_Color clothes[GLASS_ART_COUNT] = {
        {146, 109, 213, 255}, {69, 63, 89, 255}, {49, 41, 78, 255}, {146, 109, 213, 255}
    };
    SDL_Color body = bodies[art];
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){22, 22, 41, 255};
    palette['s'] = (SDL_Color){body.r * 2 / 3, body.g * 2 / 3, body.b * 2 / 3, 255};
    palette['S'] = body;
    palette['H'] = (SDL_Color){body.r + (255 - body.r) / 3,
        body.g + (255 - body.g) / 3, body.b + (255 - body.b) / 3, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){17, 19, 35, 255};
    palette['y'] = (SDL_Color){152, 236, 242, 255};
    palette['m'] = (SDL_Color){49, 41, 78, 255};
    palette['M'] = (SDL_Color){86, 181, 216, 255};
    palette['N'] = (SDL_Color){191, 238, 247, 255};
    palette['f'] = (SDL_Color){191, 238, 247, 255};
    palette['r'] = (SDL_Color){87, 96, 125, 255};
    palette['R'] = (SDL_Color){131, 150, 179, 255};
    palette['j'] = (SDL_Color){31, 29, 49, 255};
    palette['J'] = (SDL_Color){49, 41, 78, 255};
    palette['p'] = (SDL_Color){106, 200, 228, 255};
    palette['P'] = (SDL_Color){191, 238, 247, 255};
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    int pulse = (int)((SDL_GetTicks() / AMBIENT_FRAME_MS) & 1u);
    for (int layer = 0; layer < 2; layer++) {
        if (layer == 1 && type != ENEMY_PRISM_SOVEREIGN) {
            continue;
        }
        const char (*rows)[25] = layer == 0 ? glassdeep_enemy_art[art] : glassdeep_prism_art[pulse];
        for (int row = 0; row < 24; row++) {
            for (int col = 0; col < 24;) {
                char ink = rows[row][col];
                int end = col + 1;
                while (end < 24 && rows[row][end] == ink) {
                    end++;
                }
                if (ink != '.') {
                    fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
                }
                col = end;
            }
        }
    }
}

static void draw_catacombs_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int art;
    switch (type) {
        case ENEMY_ANCIENT_SKELETON: art = CATA_ART_ANCIENT_SKELETON; break;
        case ENEMY_BONE_SENTINEL: art = CATA_ART_BONE_SENTINEL; break;
        case ENEMY_GRAVE_MARSHAL: art = CATA_ART_GRAVE_MARSHAL; break;
        default: return;
    }
    static const SDL_Color clothes[CATA_ART_COUNT] = {
        {62, 65, 70, 255}, {49, 57, 75, 255}, {111, 38, 47, 255}
    };
    SDL_Color cloth = clothes[art];
    SDL_Color palette[128] = {{0}};
    palette['o'] = (SDL_Color){19, 20, 29, 255};
    palette['s'] = (SDL_Color){147, 140, 122, 255};
    palette['S'] = (SDL_Color){220, 211, 184, 255};
    palette['H'] = (SDL_Color){239, 234, 210, 255};
    palette['c'] = (SDL_Color){cloth.r * 2 / 3, cloth.g * 2 / 3, cloth.b * 2 / 3, 255};
    palette['C'] = cloth;
    palette['v'] = (SDL_Color){cloth.r + (255 - cloth.r) / 4,
        cloth.g + (255 - cloth.g) / 4, cloth.b + (255 - cloth.b) / 4, 255};
    palette['e'] = (SDL_Color){15, 18, 29, 255};
    palette['y'] = (SDL_Color){105, 216, 232, 255};
    palette['m'] = (SDL_Color){45, 48, 62, 255};
    palette['M'] = (SDL_Color){83, 94, 114, 255};
    palette['N'] = (SDL_Color){192, 205, 213, 255};
    palette['r'] = (SDL_Color){105, 72, 42, 255};
    palette['R'] = (SDL_Color){159, 115, 62, 255};
    palette['f'] = (SDL_Color){233, 226, 201, 255};
    palette['p'] = (SDL_Color){56, 121, 150, 255};
    palette['P'] = (SDL_Color){145, 237, 244, 255};
    palette['g'] = (SDL_Color){125, 93, 41, 255};
    palette['G'] = (SDL_Color){190, 159, 84, 255};
    palette['w'] = (SDL_Color){62, 43, 32, 255};
    palette['W'] = (SDL_Color){133, 100, 61, 255};
    palette['l'] = (SDL_Color){51, 37, 29, 255};
    palette['L'] = (SDL_Color){110, 77, 46, 255};
    palette['K'] = (SDL_Color){150, 116, 73, 255};
    palette['k'] = (SDL_Color){34, 29, 27, 255};
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int row = 0; row < 24; row++) {
        for (int col = 0; col < 24;) {
            char ink = catacombs_enemy_art[art][row][col];
            int end = col + 1;
            while (end < 24 && catacombs_enemy_art[art][row][end] == ink) {
                end++;
            }
            if (ink != '.') {
                fill_rect(r, x + col, y + row, end - col, 1, palette[(unsigned char)ink]);
            }
            col = end;
        }
    }
}

void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type) {
    if (type >= ENEMY_HARPY_RAIDER && type <= ENEMY_TEMPEST_MATRIARCH) {
        SDL_Rect source = {(type - ENEMY_HARPY_RAIDER) * 48, (SDL_GetTicks() / 450 % 2) * 48, 48, 48};
        SDL_Rect dest = {tile_x * TILE_SIZE - 12, tile_y * TILE_SIZE - 24, 48, 48};
        SDL_RenderCopy(r->sdl, r->stormwatch_enemy_texture, &source, &dest);
        return;
    }
    if ((type >= ENEMY_LIZARDFOLK_SCOUT && type <= ENEMY_MARSH_SPITTER) ||
        (type >= ENEMY_LIZARDFOLK_SCALEGUARD && type <= ENEMY_SSZARETH)) {
        int index = type - ENEMY_LIZARDFOLK_SCOUT - (type >= ENEMY_LIZARDFOLK_SCALEGUARD);
        SDL_Rect source = {index * 48, 0, 48, 48};
        SDL_Rect dest = {tile_x * TILE_SIZE - 12, tile_y * TILE_SIZE - 24, 48, 48};
        SDL_RenderCopy(r->sdl, r->mangrove_enemy_texture, &source, &dest);
        return;
    }
    if (type >= ENEMY_BRIAR_WOLF && type <= ENEMY_ELDERTHORN) {
        int index = type - ENEMY_BRIAR_WOLF;
        SDL_Rect source = {index * 64, 0, 64, 64};
        SDL_Rect dest = {tile_x * TILE_SIZE - 20, tile_y * TILE_SIZE - 40, 64, 64};
        SDL_RenderCopy(r->sdl, r->elderwood_enemy_texture, &source, &dest);
        return;
    }
    switch (type) {
        case ENEMY_GOBLIN_MINE_BOSS: {
            SDL_Rect destination = {tile_x * TILE_SIZE, tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_RenderCopy(r->sdl, r->ironfang_boss_texture, NULL, &destination);
            break;
        }
        case ENEMY_LIZARDFOLK_SCOUT:
        case ENEMY_REED_SPEARHUNTER:
        case ENEMY_MARSH_SPITTER:
        case ENEMY_LIZARDFOLK_SCALEGUARD:
        case ENEMY_HARPY_RAIDER:
        case ENEMY_CLIFF_GARGOYLE:
        case ENEMY_HARPY_STORMCALLER:
        case ENEMY_STORM_ELEMENTAL:
        case ENEMY_TEMPEST_MATRIARCH:
        case ENEMY_SSZARETH:
            break; // Drawn from the Saltwind atlas above.
        case ENEMY_ROYAL_GUARD:
            draw_royal_guard_overlay(r, tile_x, tile_y, 0);
            break;
        case ENEMY_BRIAR_WOLF:
        case ENEMY_IRONHIDE_BOAR:
        case ENEMY_THORN_DRYAD:
        case ENEMY_TREEFOLK:
        case ENEMY_ELDERTHORN:
            break; // Drawn from the Elderwood atlas above.
        case ENEMY_OATHBOUND_SOLDIER:
        case ENEMY_IRON_WARDEN:
        case ENEMY_ROYAL_MARKSMAN:
        case ENEMY_COURT_HEXER:
        case ENEMY_BELL_HERALD:
        case ENEMY_CASTELLAN:
        case ENEMY_ROYAL_ARCANIST:
        case ENEMY_LORD_VEYR:
            draw_armored_guard(r, tile_x, tile_y);
            break;
        case ENEMY_ANCIENT_SKELETON:
        case ENEMY_BONE_SENTINEL:
        case ENEMY_GRAVE_MARSHAL:
            draw_catacombs_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_MINOTAUR:
            draw_swamp_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_IRON_SENTINEL:
        case ENEMY_CRYSTAL_SPIDER:
        case ENEMY_BLIND_STALKER:
        case ENEMY_SHARD_GOLEM:
        case ENEMY_PRISM_SOVEREIGN:
            draw_glassdeep_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_CINDER_IMP:
        case ENEMY_ASH_HOUND:
        case ENEMY_OBSIDIAN_GUARDIAN:
        case ENEMY_CINDER_LORD:
            draw_ashen_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_SCARAB:
        case ENEMY_VIPER:
        case ENEMY_MUMMY:
        case ENEMY_DJINN:
        case ENEMY_GOLEM:
        case ENEMY_DESERT_PHARAOH:
            draw_desert_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_FROST_KRAKEN:
            draw_frost_kraken(r, tile_x, tile_y);
            break;
        case ENEMY_ROAD_ARCHER:
        case ENEMY_HORSEMAN:
            draw_dragonspine_road_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_ICE_WOLF:
        case ENEMY_FROST_ARCHER:
        case ENEMY_YETI:
        case ENEMY_FROST_WRAITH:
        case ENEMY_ICE_GOLEM:
        case ENEMY_FROST_GIANT:
            draw_frostfell_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_GIANT_RAT:
        case ENEMY_BANDIT:
        case ENEMY_VAMPIRE:
        case ENEMY_SWAMP_DEMON:
            draw_swamp_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_SKELETON: draw_skeleton(r, tile_x, tile_y); break;
        case ENEMY_ZOMBIE:   draw_zombie(r, tile_x, tile_y);   break;
        case ENEMY_CRYPT_BAT: draw_crypt_bat(r, tile_x, tile_y); break;
        case ENEMY_WRAITH: draw_wraith(r, tile_x, tile_y); break;
        case ENEMY_CRYPT_CONJURER: draw_necromancer(r, tile_x, tile_y); break;
        case ENEMY_PIXIE:
        case ENEMY_BLIGHTED_WOLF:
        case ENEMY_GIANT_SPIDER:
        case ENEMY_DARK_ELF:
        case ENEMY_GIANT_WURM:
        case ENEMY_FOREST_TROLL:
        case ENEMY_FOREST_NECROMANCER:
            draw_forest_enemy(r, tile_x, tile_y, type); break;
        case ENEMY_GOBLIN_SCOUT:
        case ENEMY_GOBLIN_ARCHER:
        case ENEMY_GOBLIN_BOMBER:
        case ENEMY_TUNNEL_SPIDER:
        case ENEMY_CAVE_TROLL:
        case ENEMY_HOBGOBLIN_GUARD:
        case ENEMY_GOBLIN_SHAMAN:
        case ENEMY_MOUNTAIN_GOBLIN_KING:
            draw_mountain_enemy(r, tile_x, tile_y, type); break;
        case ENEMY_ILLUSION:
        case ENEMY_MERFOLK:
        case ENEMY_SIREN:
        case ENEMY_GIANT_CRAB:
        case ENEMY_ANIMATED_STATUE:
        case ENEMY_WATER_ELEMENTAL:
        case ENEMY_SEA_SERPENT:
        case ENEMY_DROWNED_QUEEN:
            draw_coast_enemy(r, tile_x, tile_y, type); break;
        case ENEMY_RELIC_SCARABS:
        case ENEMY_TEMPLE_STALKER:
        case ENEMY_BLOWDART_HUNTER:
        case ENEMY_VINEBOUND_GUARDIAN:
        case ENEMY_SUN_PRIEST:
        case ENEMY_SERPENT_SPIRIT:
        case ENEMY_TREASURE_WRAITH:
        case ENEMY_LUNAR_EFFIGY:
        case ENEMY_MOONBOUND_SENTINEL:
        case ENEMY_FALLEN_SUN_GUARDIAN: {
            if (!r->temple_enemy_texture) {
                break;
            }
            int art = 0;
            switch (type) {
                case ENEMY_TEMPLE_STALKER: art = 1; break;
                case ENEMY_BLOWDART_HUNTER: art = 2; break;
                case ENEMY_VINEBOUND_GUARDIAN: art = 3; break;
                case ENEMY_SUN_PRIEST: art = 4; break;
                case ENEMY_SERPENT_SPIRIT: art = 5; break;
                case ENEMY_TREASURE_WRAITH: art = 6; break;
                case ENEMY_LUNAR_EFFIGY: art = 7; break;
                case ENEMY_MOONBOUND_SENTINEL: art = 8; break;
                case ENEMY_FALLEN_SUN_GUARDIAN: art = 9; break;
                default: break;
            }
            int size = type == ENEMY_FALLEN_SUN_GUARDIAN ? 40 : 28;
            // Native frames occupy bottom-centered slots in the 48-pixel atlas.
            SDL_Rect source = {art % 4 * 48 + (48 - size) / 2,
                art / 4 * 48 + 48 - size, size, size};
            SDL_Rect destination = {
                tile_x * TILE_SIZE + (TILE_SIZE - size) / 2,
                tile_y * TILE_SIZE + TILE_SIZE - size, size, size
            };
            SDL_RenderCopy(r->sdl, r->temple_enemy_texture, &source,
                &destination);
            break;
        }
        case ENEMY_OGRE:
        case ENEMY_HILL_GIANT:
        case ENEMY_LAST_JARL:
            giants_draw_enemy(r, tile_x, tile_y, type);
            break;
        case ENEMY_STONE_GIANT: draw_giant(r, tile_x, tile_y); break;
        case ENEMY_LICH_KING:   draw_lich_king(r, tile_x, tile_y);   break;
        case ENEMY_RED_DRAGON:  draw_red_dragon(r, tile_x, tile_y);  break;
        case ENEMY_DRAKE:
        case ENEMY_FIRE_ELEMENTAL:
            draw_dragonspine_road_enemy(r, tile_x, tile_y, type); break;
    }
}

void draw_fallen_sun_guardian_broken(Renderer *r, int tile_x, int tile_y) {
    if (!r->temple_enemy_texture) {
        return;
    }
    SDL_Rect source = {100, 104, 40, 40};
    SDL_Rect destination = {
        tile_x * TILE_SIZE - 8,
        tile_y * TILE_SIZE - 16, 40, 40
    };
    SDL_RenderCopy(r->sdl, r->temple_enemy_texture, &source, &destination);
}

void draw_temple_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = ((map_x + map_y) & 1)
        ? (SDL_Color){49, 57, 47, 255} : (SDL_Color){55, 64, 52, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, stone);
    fill_rect(r, x, y, TILE_SIZE, 2, (SDL_Color){76, 83, 65, 255});
    fill_rect(r, x, y, 2, TILE_SIZE, (SDL_Color){38, 43, 37, 255});
    if ((map_x * 7 + map_y * 11) % 9 == 0) {
        fill_rect(r, x + 4, y + 16, 9, 2, (SDL_Color){29, 70, 40, 255});
        fill_rect(r, x + 8, y + 13, 2, 5, (SDL_Color){43, 101, 49, 255});
    }
}

void draw_temple_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){25, 31, 28, 255});
    fill_rect(r, x + 1, y + 2, 22, 8, (SDL_Color){63, 68, 57, 255});
    fill_rect(r, x + 1, y + 12, 22, 9, (SDL_Color){54, 61, 52, 255});
    fill_rect(r, x, y + 10, TILE_SIZE, 2, (SDL_Color){24, 30, 27, 255});
    int seam = ((map_x + map_y) & 1) ? 7 : 15;
    fill_rect(r, x + seam, y + 2, 2, 8, (SDL_Color){37, 42, 37, 255});
    fill_rect(r, x + (22 - seam), y + 12, 2, 9, (SDL_Color){35, 41, 36, 255});
    if ((map_x * 5 + map_y) % 7 == 0) {
        fill_rect(r, x + 2, y + 3, 3, 15, (SDL_Color){31, 91, 43, 255});
    }
}

void draw_temple_entrance(Renderer *r, int tile_x, int tile_y) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_STAIRS_UP, (SDL_Color){195, 159, 77, 255});
}

void draw_temple_altar(Renderer *r, int tile_x, int tile_y, int moon) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    SDL_Color accent = moon ? (SDL_Color){85, 218, 240, 255} : (SDL_Color){235, 174, 49, 255};
    draw_world_prop(r, tile_x, tile_y, PROP_ALTAR, accent);
}

void draw_temple_moon_door(Renderer *r, int tile_x, int tile_y, int open) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    if (!open) {
        draw_world_prop(r, tile_x, tile_y, PROP_MOON_DOOR, (SDL_Color){111, 224, 233, 255});
    }
}

void draw_temple_solar_trap(Renderer *r, int tile_x, int tile_y, int active) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    SDL_Color accent = active ? (SDL_Color){244, 138, 27, 255} : (SDL_Color){101, 82, 48, 255};
    draw_world_prop(r, tile_x, tile_y, PROP_SUN_PLATE, accent);
}

void draw_temple_dormant_sentinel(Renderer *r, int tile_x, int tile_y) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_SENTINEL, (SDL_Color){90, 120, 106, 255});
}

void draw_temple_vault_door(Renderer *r, int tile_x, int tile_y) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_VAULT_DOOR, (SDL_Color){247, 165, 39, 255});
}

void draw_temple_treasure(Renderer *r, int tile_x, int tile_y) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_CHEST, (SDL_Color){244, 190, 52, 255});
}

void draw_temple_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){18, 83, 88, 255});
    int wave = (map_x * 5 + map_y * 3) % 11;
    fill_rect(r, x + wave, y + 7, 10, 2, (SDL_Color){54, 151, 146, 255});
    fill_rect(r, x + (wave + 7) % 13, y + 17, 9, 2,
        (SDL_Color){35, 125, 126, 255});
}

void draw_temple_rubble(Renderer *r, int tile_x, int tile_y) {
    draw_temple_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_RUBBLE, (SDL_Color){112, 98, 74, 255});
}

void draw_town_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color grass[4] = {
        {15, 35, 21, 255},
        {17, 39, 23, 255},
        {13, 32, 19, 255},
        {18, 37, 20, 255}
    };
    SDL_Color dark = {10, 27, 16, 255};
    SDL_Color blade = {29, 56, 29, 255};
    SDL_Color blade_light = {39, 69, 35, 255};
    SDL_Color soil = {51, 43, 28, 255};
    SDL_Color soil_light = {72, 57, 34, 255};
    unsigned int seed = (unsigned int)tile_x * 1103515245u +
        (unsigned int)tile_y * 2654435761u +
        (unsigned int)(tile_x * tile_y) * 2246822519u;
    int pattern = (int)((seed >> 8) & 3u);
    int detail = (int)((seed >> 16) % 31u);

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, grass[pattern]);
    if (pattern == 0) {
        fill_rect(r, x + 2, y + 15, 8, 5, dark);
        fill_rect(r, x + 5, y + 13, 7, 6, grass[2]);
    } else if (pattern == 1) {
        fill_rect(r, x + 14, y + 3, 8, 7, dark);
        fill_rect(r, x + 12, y + 5, 8, 6, grass[0]);
    } else if (pattern == 2) {
        fill_rect(r, x + 1, y + 2, 9, 5, grass[1]);
        fill_rect(r, x + 16, y + 17, 7, 5, dark);
    } else {
        fill_rect(r, x + 7, y + 8, 11, 7, grass[2]);
    }

    int tuft_x = 3 + (int)((seed >> 3) % 16u);
    int tuft_y = 5 + (int)((seed >> 11) % 13u);
    fill_rect(r, x + tuft_x, y + tuft_y, 2, 4, blade);
    fill_rect(r, x + tuft_x - 2, y + tuft_y + 2, 2, 2, blade);
    fill_rect(r, x + tuft_x + 2, y + tuft_y + 1, 2, 3, blade_light);

    if (detail == 0 || detail == 17) {
        fill_rect(r, x + 4, y + 17, 9, 4, soil);
        fill_rect(r, x + 7, y + 16, 8, 3, soil);
        fill_rect(r, x + 7, y + 17, 4, 1, soil_light);
    } else if (detail == 7) {
        fill_rect(r, x + 17, y + 7, 2, 2,
            (SDL_Color){204, 181, 76, 255});
        fill_rect(r, x + 16, y + 9, 1, 3, blade_light);
    } else if (detail == 23) {
        fill_rect(r, x + 5, y + 6, 3, 2,
            (SDL_Color){83, 88, 82, 255});
        fill_rect(r, x + 6, y + 5, 2, 1,
            (SDL_Color){115, 118, 106, 255});
    }
}

void draw_tavern_floor(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color oak = {77, 45, 27, 255};
    SDL_Color plank = {102, 59, 31, 255};
    SDL_Color seam = {44, 27, 20, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, oak);
    fill_rect(r, x, y + 2, TILE_SIZE, 9, plank);
    fill_rect(r, x, y + 13, TILE_SIZE, 9, plank);
    fill_rect(r, x, y + 11, TILE_SIZE, 2, seam);
    fill_rect(r, x + 7, y + 2, 2, 9, seam);
    fill_rect(r, x + 17, y + 13, 2, 9, seam);
}

void draw_tavern_wall(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color timber = {50, 29, 22, 255};
    SDL_Color plaster = {122, 84, 52, 255};
    SDL_Color highlight = {152, 105, 62, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, timber);
    fill_rect(r, x + 3, y + 3, 18, 18, plaster);
    fill_rect(r, x + 3, y + 3, 18, 3, highlight);
    fill_rect(r, x + 10, y + 3, 4, 18, timber);
}

void draw_tavern_exit(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_TAVERN_DOOR, (SDL_Color){210, 161, 64, 255});
}

void draw_tavern_table(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_TABLE, (SDL_Color){151, 91, 42, 255});
}

void draw_cain(Renderer *r, int tile_x, int tile_y) {
    draw_town_path(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_CAIN, 0);
}

void draw_rowan(Renderer *r, int tile_x, int tile_y) {
    npc_draw(r, tile_x, tile_y, NPC_ART_ROWAN, 0);
}

void draw_nahla(Renderer *r, int tile_x, int tile_y) {
    npc_draw(r, tile_x, tile_y, NPC_ART_NAHLA, 0);
}

void draw_healer_house(Renderer *r, int tile_x, int tile_y) {
    if (r->healer_texture) {
        SDL_Rect destination = {tile_x * TILE_SIZE, tile_y * TILE_SIZE,
            TOWN_HEALER_W * TILE_SIZE, TOWN_HEALER_H * TILE_SIZE};
        SDL_RenderCopy(r->sdl, r->healer_texture, NULL, &destination);
        return;
    }
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int w = TOWN_HEALER_W * TILE_SIZE;
    int h = TOWN_HEALER_H * TILE_SIZE;
    SDL_Color timber = {70, 46, 34, 255};
    SDL_Color plaster = {207, 193, 155, 255};
    SDL_Color teal = {45, 102, 88, 255};
    fill_rect(r, x + 4, y + 26, w - 8, h - 26, timber);
    fill_rect(r, x + 9, y + 30, w - 18, h - 35, plaster);
    for (int bx = 10; bx < w - 10; bx += 12) {
        fill_rect(r, x + bx, y + h - 8, 10, 6, (SDL_Color){113, 109, 96, 255});
        fill_rect(r, x + bx, y + h - 8, 10, 1, (SDL_Color){161, 151, 127, 255});
    }
    for (int beam = 0; beam < 4; beam++) {
        fill_rect(r, x + 9 + beam * 33, y + 30, 4, h - 38, timber);
    }
    fill_rect(r, x + 9, y + 72, w - 18, 3, timber);
    for (int row = 0; row < 30; row++) {
        int inset = (29 - row) / 2;
        fill_rect(r, x + inset, y + row, w - inset * 2, 1,
            row % 6 == 0 ? timber : teal);
        if (row % 6 != 0) {
            for (int seam = inset + (row / 6 % 2) * 6; seam < w - inset; seam += 12) {
                fill_rect(r, x + seam, y + row, 1, 1, (SDL_Color){31, 74, 63, 255});
            }
        }
    }
    fill_rect(r, x + 52, y + 61, 16, h - 61, timber);
    fill_rect(r, x + 56, y + 65, 9, h - 66, (SDL_Color){38, 29, 30, 255});
    for (int side = 0; side < 2; side++) {
        int wx = x + 18 + side * 66;
        fill_rect(r, wx, y + 47, 18, 22, timber);
        fill_rect(r, wx + 3, y + 50, 12, 16, (SDL_Color){239, 198, 109, 255});
        fill_rect(r, wx + 8, y + 49, 2, 18, timber);
        fill_rect(r, wx - 2, y + 73, 22, 7, teal);
    }
    fill_rect(r, x + 50, y + 32, 20, 22, teal);
    fill_rect(r, x + 58, y + 35, 4, 16, plaster);
    fill_rect(r, x + 53, y + 41, 14, 4, plaster);
}

void draw_witch_hut(Renderer *r, int tile_x, int tile_y) {
    if (!r->witch_texture) {
        draw_healer_house(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TOWN_WITCH_W * TILE_SIZE, TOWN_WITCH_H * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->witch_texture, NULL, &destination);
}

static void healer_rect(Renderer *r, int x, int y, int scale, int px, int py, int pw, int ph, SDL_Color color) {
    fill_rect(r, x + px * scale, y + py * scale, pw * scale, ph * scale, color);
}

void draw_healer_portrait(Renderer *r, int x, int y, int scale) {
    SDL_Color outline = {35, 27, 29, 255};
    SDL_Color hair_dark = {77, 40, 30, 255};
    SDL_Color hair = {126, 70, 41, 255};
    SDL_Color hair_light = {177, 105, 55, 255};
    SDL_Color skin_shadow = {180, 118, 88, 255};
    SDL_Color skin = {224, 166, 124, 255};
    SDL_Color skin_light = {241, 197, 154, 255};
    SDL_Color robe_dark = {31, 79, 69, 255};
    SDL_Color robe = {52, 126, 103, 255};
    SDL_Color robe_light = {82, 162, 128, 255};
    SDL_Color cream_shadow = {180, 170, 137, 255};
    SDL_Color cream = {226, 218, 181, 255};
    SDL_Color ivory = {244, 235, 198, 255};
    SDL_Color leather = {86, 52, 35, 255};
    SDL_Color leather_light = {143, 86, 45, 255};
    SDL_Color bottle = {76, 180, 164, 255};
    SDL_Color tonic = {229, 183, 66, 255};

    healer_rect(r, x, y, scale, 9, 1, 14, 2, outline);
    healer_rect(r, x, y, scale, 6, 3, 20, 3, outline);
    healer_rect(r, x, y, scale, 4, 6, 24, 9, outline);
    healer_rect(r, x, y, scale, 5, 14, 5, 9, outline);
    healer_rect(r, x, y, scale, 22, 14, 5, 9, outline);
    healer_rect(r, x, y, scale, 10, 2, 12, 2, hair);
    healer_rect(r, x, y, scale, 7, 4, 18, 4, hair);
    healer_rect(r, x, y, scale, 5, 7, 22, 7, hair_dark);
    healer_rect(r, x, y, scale, 6, 7, 5, 8, hair);
    healer_rect(r, x, y, scale, 21, 7, 5, 8, hair);
    healer_rect(r, x, y, scale, 9, 4, 9, 2, hair_light);
    healer_rect(r, x, y, scale, 7, 6, 4, 2, hair_light);

    healer_rect(r, x, y, scale, 9, 6, 14, 11, outline);
    healer_rect(r, x, y, scale, 10, 7, 12, 9, skin);
    healer_rect(r, x, y, scale, 10, 13, 12, 3, skin_shadow);
    healer_rect(r, x, y, scale, 11, 7, 9, 2, skin_light);
    healer_rect(r, x, y, scale, 9, 5, 6, 3, hair);
    healer_rect(r, x, y, scale, 14, 5, 8, 2, hair_light);
    healer_rect(r, x, y, scale, 19, 6, 4, 3, hair);
    healer_rect(r, x, y, scale, 12, 10, 2, 2, outline);
    healer_rect(r, x, y, scale, 19, 10, 2, 2, outline);
    healer_rect(r, x, y, scale, 13, 10, 1, 1, ivory);
    healer_rect(r, x, y, scale, 18, 10, 1, 1, ivory);
    healer_rect(r, x, y, scale, 16, 11, 1, 3, skin_shadow);
    healer_rect(r, x, y, scale, 14, 14, 5, 1, hair_dark);
    healer_rect(r, x, y, scale, 15, 14, 3, 1, skin_light);
    healer_rect(r, x, y, scale, 13, 16, 7, 4, outline);
    healer_rect(r, x, y, scale, 14, 16, 5, 3, skin);

    healer_rect(r, x, y, scale, 5, 14, 4, 3, hair_light);
    healer_rect(r, x, y, scale, 6, 17, 4, 3, hair);
    healer_rect(r, x, y, scale, 5, 20, 4, 3, hair_light);
    healer_rect(r, x, y, scale, 23, 14, 3, 3, hair_light);
    healer_rect(r, x, y, scale, 22, 17, 4, 3, hair);
    healer_rect(r, x, y, scale, 23, 20, 4, 3, hair_light);

    healer_rect(r, x, y, scale, 5, 18, 22, 4, outline);
    healer_rect(r, x, y, scale, 3, 21, 26, 12, outline);
    healer_rect(r, x, y, scale, 6, 32, 20, 7, outline);
    healer_rect(r, x, y, scale, 6, 19, 20, 4, robe_dark);
    healer_rect(r, x, y, scale, 4, 22, 24, 10, robe);
    healer_rect(r, x, y, scale, 7, 31, 18, 7, robe_dark);
    healer_rect(r, x, y, scale, 5, 23, 4, 8, robe_light);
    healer_rect(r, x, y, scale, 24, 23, 3, 8, robe_dark);
    healer_rect(r, x, y, scale, 8, 32, 4, 5, robe);
    healer_rect(r, x, y, scale, 20, 32, 3, 5, robe_light);

    healer_rect(r, x, y, scale, 10, 19, 12, 3, cream_shadow);
    healer_rect(r, x, y, scale, 11, 20, 10, 3, cream);
    healer_rect(r, x, y, scale, 11, 22, 10, 14, cream_shadow);
    healer_rect(r, x, y, scale, 12, 22, 8, 13, cream);
    healer_rect(r, x, y, scale, 13, 23, 6, 2, ivory);
    healer_rect(r, x, y, scale, 15, 23, 2, 6, robe_dark);
    healer_rect(r, x, y, scale, 13, 25, 6, 2, robe_dark);
    healer_rect(r, x, y, scale, 13, 30, 1, 4, ivory);
    healer_rect(r, x, y, scale, 18, 29, 1, 5, cream_shadow);
    healer_rect(r, x, y, scale, 4, 30, 24, 2, leather);
    healer_rect(r, x, y, scale, 12, 30, 9, 2, leather_light);

    healer_rect(r, x, y, scale, 1, 23, 5, 3, outline);
    healer_rect(r, x, y, scale, 1, 25, 5, 7, robe_dark);
    healer_rect(r, x, y, scale, 2, 25, 4, 6, robe_light);
    healer_rect(r, x, y, scale, 2, 31, 4, 4, outline);
    healer_rect(r, x, y, scale, 2, 31, 3, 3, skin);
    healer_rect(r, x, y, scale, 27, 23, 4, 3, outline);
    healer_rect(r, x, y, scale, 27, 25, 4, 7, robe_dark);
    healer_rect(r, x, y, scale, 27, 31, 4, 4, outline);
    healer_rect(r, x, y, scale, 28, 31, 3, 3, skin);

    healer_rect(r, x, y, scale, 23, 28, 8, 8, outline);
    healer_rect(r, x, y, scale, 24, 29, 6, 6, leather);
    healer_rect(r, x, y, scale, 25, 29, 4, 2, leather_light);
    healer_rect(r, x, y, scale, 26, 32, 2, 2, tonic);
    healer_rect(r, x, y, scale, 2, 26, 3, 2, outline);
    healer_rect(r, x, y, scale, 1, 28, 5, 7, outline);
    healer_rect(r, x, y, scale, 2, 28, 3, 2, bottle);
    healer_rect(r, x, y, scale, 2, 30, 3, 4, tonic);
    healer_rect(r, x, y, scale, 3, 30, 1, 3, ivory);

    healer_rect(r, x, y, scale, 7, 38, 7, 2, outline);
    healer_rect(r, x, y, scale, 19, 38, 7, 2, outline);
    healer_rect(r, x, y, scale, 8, 38, 5, 1, leather_light);
    healer_rect(r, x, y, scale, 20, 38, 5, 1, leather_light);
}

void draw_witch_portrait(Renderer *r, int x, int y, int scale) {
    SDL_Color outline = {28, 20, 35, 255};
    SDL_Color hat_dark = {49, 33, 67, 255};
    SDL_Color hat = {78, 50, 101, 255};
    SDL_Color hat_light = {112, 72, 137, 255};
    SDL_Color hair = {181, 162, 130, 255};
    SDL_Color skin = {193, 139, 105, 255};
    SDL_Color skin_light = {225, 176, 132, 255};
    SDL_Color robe_dark = {32, 66, 59, 255};
    SDL_Color robe = {48, 105, 82, 255};
    SDL_Color robe_light = {72, 139, 101, 255};
    SDL_Color moon = {232, 197, 91, 255};
    SDL_Color potion = {120, 205, 177, 255};

    healer_rect(r, x, y, scale, 13, 0, 8, 4, outline);
    healer_rect(r, x, y, scale, 11, 3, 12, 5, outline);
    healer_rect(r, x, y, scale, 8, 7, 18, 6, outline);
    healer_rect(r, x, y, scale, 2, 12, 29, 4, outline);
    healer_rect(r, x, y, scale, 14, 1, 6, 4, hat_light);
    healer_rect(r, x, y, scale, 12, 4, 10, 5, hat);
    healer_rect(r, x, y, scale, 9, 8, 16, 5, hat_dark);
    healer_rect(r, x, y, scale, 4, 13, 25, 2, hat);
    healer_rect(r, x, y, scale, 18, 4, 2, 3, moon);

    healer_rect(r, x, y, scale, 8, 15, 17, 15, outline);
    healer_rect(r, x, y, scale, 10, 16, 13, 12, skin);
    healer_rect(r, x, y, scale, 11, 17, 10, 3, skin_light);
    healer_rect(r, x, y, scale, 8, 16, 4, 13, hair);
    healer_rect(r, x, y, scale, 22, 16, 3, 13, hair);
    healer_rect(r, x, y, scale, 12, 21, 2, 2, outline);
    healer_rect(r, x, y, scale, 19, 21, 2, 2, outline);
    healer_rect(r, x, y, scale, 15, 26, 5, 1, outline);

    healer_rect(r, x, y, scale, 5, 29, 23, 4, outline);
    healer_rect(r, x, y, scale, 3, 32, 27, 8, outline);
    healer_rect(r, x, y, scale, 6, 30, 21, 4, robe_dark);
    healer_rect(r, x, y, scale, 5, 33, 23, 7, robe);
    healer_rect(r, x, y, scale, 7, 34, 4, 6, robe_light);
    healer_rect(r, x, y, scale, 15, 31, 4, 7, outline);
    healer_rect(r, x, y, scale, 16, 32, 2, 5, moon);
    healer_rect(r, x, y, scale, 27, 31, 5, 8, outline);
    healer_rect(r, x, y, scale, 28, 32, 3, 6, potion);
    healer_rect(r, x, y, scale, 29, 33, 1, 2, skin_light);
}

void draw_elowen(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_ELOWEN, 0);
}

void draw_dain(Renderer *r, int tile_x, int tile_y) {
    npc_draw(r, tile_x, tile_y, NPC_ART_DAIN, 0);
}

void draw_alder(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_ALDER, 0);
}

void draw_mara(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_MARA, 0);
}

void draw_royal_guard(Renderer *r, int tile_x, int tile_y, int halberd_left) {
    draw_town_floor(r, tile_x, tile_y);
    draw_royal_guard_overlay(r, tile_x, tile_y, halberd_left);
}

void draw_royal_guard_overlay(Renderer *r, int tile_x, int tile_y, int halberd_left) {
    npc_draw(r, tile_x, tile_y, NPC_ART_GUARD, halberd_left);
}

void draw_rook(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_ROOK, 0);
}

void draw_innkeeper(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_BRAM_INN, 0);
}

void draw_swamp_daughter(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_swamp_floor(r, tile_x, tile_y, map_x, map_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_MIRA, 0);
}

static unsigned int dragonspine_noise(int x, int y) {
    unsigned int seed = (unsigned int)x * 2246822519u ^ (unsigned int)y * 3266489917u;
    seed ^= seed >> 16;
    seed *= 668265263u;
    return seed ^ (seed >> 15);
}

static int dragonspine_shade(int x, int y, int width, int height) {
    int gx = x / width;
    int gy = y / height;
    int fx = x % width;
    int fy = y % height;
    int a = (int)(dragonspine_noise(gx, gy) & 255u);
    int b = (int)(dragonspine_noise(gx + 1, gy) & 255u);
    int c = (int)(dragonspine_noise(gx, gy + 1) & 255u);
    int d = (int)(dragonspine_noise(gx + 1, gy + 1) & 255u);
    int top = (a * (width - fx) + b * fx) / width;
    int bottom = (c * (width - fx) + d * fx) / width;
    return (top * (height - fy) + bottom * fy) / height;
}

// Clip world-space texture clusters to a tile so they join across tile boundaries.
static void dragonspine_patch(Renderer *r, int tx, int ty, int px, int py, int w, int h, SDL_Color color) {
    int right = px + w < TILE_SIZE ? px + w : TILE_SIZE;
    int bottom = py + h < TILE_SIZE ? py + h : TILE_SIZE;
    if (px < 0) {
        px = 0;
    }
    if (py < 0) {
        py = 0;
    }
    if (right > px && bottom > py) {
        fill_rect(r, tx * TILE_SIZE + px, ty * TILE_SIZE + py, right - px, bottom - py, color);
    }
}

void draw_dragonspine_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int terrain) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int world_x = map_x * TILE_SIZE;
    int world_y = map_y * TILE_SIZE;
    unsigned int seed = dragonspine_noise(map_x, map_y);
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){166, 184, 198, 255});
    for (int gy = world_y / 28 - 1; gy <= (world_y + TILE_SIZE) / 28; gy++) {
        for (int gx = world_x / 36 - 1; gx <= (world_x + TILE_SIZE) / 36; gx++) {
            unsigned int drift = dragonspine_noise(gx, gy);
            if (drift % 3u == 0u) {
                continue;
            }
            int px = gx * 36 + (int)(drift % 28u) - world_x;
            int py = gy * 28 + (int)((drift >> 8) % 22u) - world_y;
            int width = 17 + (int)((drift >> 16) % 11u);
            for (int row = 0; row < 3; row++) {
                int inset = row == 1 ? 0 : 4;
                dragonspine_patch(r, tile_x, tile_y, px + inset, py + row * 2, width - inset * 2, 2,
                    (SDL_Color){183, 201, 213, 255});
            }
        }
    }
    int cx = 6 + (int)(seed % 12u);
    int cy = 6 + (int)((seed >> 8) % 12u);
    if (terrain != 0) {
        for (int row = -3; row <= 3; row++) {
            int width = 13 - abs(row) * 2 + (int)((seed >> (row + 3)) & 3u);
            dragonspine_patch(r, tile_x, tile_y, cx - width / 2, cy + row, width, 1,
                (SDL_Color){137, 154, 169, 255});
        }
        fill_rect(r, x + cx - 3, y + cy, 6, 2, (SDL_Color){111, 130, 147, 255});
        if (terrain == 1 && seed % 4u == 0u) {
            fill_rect(r, x + cx - 1, y + cy - 2, 2, 2, (SDL_Color){222, 126, 60, 255});
            fill_rect(r, x + cx, y + cy - 2, 1, 1, (SDL_Color){250, 191, 106, 255});
        } else if (terrain == 2) {
            for (int coin = 0; coin < 3; coin++) {
                int px = cx - 4 + coin * 4;
                int py = cy + (int)((seed >> (coin * 3)) & 3u);
                fill_rect(r, x + px, y + py + 1, 4, 2, (SDL_Color){151, 110, 48, 255});
                fill_rect(r, x + px + 1, y + py, 3, 2, (SDL_Color){239, 197, 93, 255});
            }
        }
    } else if (seed % 5u == 0u) {
        fill_rect(r, x + cx, y + cy, 3, 2, (SDL_Color){141, 161, 179, 255});
        fill_rect(r, x + cx + 1, y + cy, 2, 1, (SDL_Color){209, 223, 232, 255});
    }
}

static int dragonspine_cliff(TileType tile) {
    return tile == TILE_DRAGON_WALL || tile == TILE_DRAGON_RAVINE;
}

void draw_dragonspine_wall(Renderer *r, const Map *map, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int world_x = map_x * TILE_SIZE;
    int world_y = map_y * TILE_SIZE;
    SDL_Rect patches[8][64];
    int counts[8] = {0};
    for (int py = 0; py < TILE_SIZE; py += 3) {
        for (int px = 0; px < TILE_SIZE; px += 3) {
            int broad = dragonspine_shade(world_x + px, world_y + py, 48, 36);
            int detail = dragonspine_shade(world_x + px, world_y + py, 15, 21);
            int shade = (broad * 3 + detail) / 128;
            patches[shade][counts[shade]++] = (SDL_Rect){x + px, y + py, 3, 3};
        }
    }
    for (int shade = 0; shade < 8; shade++) {
        if (counts[shade] > 0) {
            SDL_SetRenderDrawColor(r->sdl, 35 + shade * 7, 52 + shade * 7, 70 + shade * 7, 255);
            SDL_RenderFillRects(r->sdl, patches[shade], counts[shade]);
        }
    }
    int north = map_y > 0 && !dragonspine_cliff(map->tiles[map_y - 1][map_x]);
    int east = map_x < MAP_W - 1 && !dragonspine_cliff(map->tiles[map_y][map_x + 1]);
    int south = map_y < MAP_H - 1 && !dragonspine_cliff(map->tiles[map_y + 1][map_x]);
    int west = map_x > 0 && !dragonspine_cliff(map->tiles[map_y][map_x - 1]);
    for (int step = 0; step < TILE_SIZE; step += 3) {
        int depth = 2 + (int)(dragonspine_noise(world_x + step, world_y) % 3u);
        if (north) {
            fill_rect(r, x + step, y, 3, depth, (SDL_Color){190, 208, 220, 255});
        }
        if (south) {
            fill_rect(r, x + step, y + TILE_SIZE - depth, 3, depth, (SDL_Color){26, 43, 59, 255});
        }
        if (west) {
            fill_rect(r, x, y + step, depth, 3, (SDL_Color){110, 136, 157, 255});
        }
        if (east) {
            fill_rect(r, x + TILE_SIZE - depth, y + step, depth, 3, (SDL_Color){30, 47, 64, 255});
        }
    }
    if (north && west) {
        fill_rect(r, x, y, 3, 3, (SDL_Color){166, 184, 198, 255});
    }
    if (north && east) {
        fill_rect(r, x + TILE_SIZE - 3, y, 3, 3, (SDL_Color){166, 184, 198, 255});
    }
    if (south && west) {
        fill_rect(r, x, y + TILE_SIZE - 3, 3, 3, (SDL_Color){166, 184, 198, 255});
    }
    if (south && east) {
        fill_rect(r, x + TILE_SIZE - 3, y + TILE_SIZE - 3, 3, 3, (SDL_Color){166, 184, 198, 255});
    }
}

void draw_dragonspine_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int forward) {
    (void)forward;
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_dragonspine_floor(r, tile_x, tile_y, map_x, map_y, 0);
    fill_rect(r, x + 2, y + 1, 7, 3, (SDL_Color){82, 101, 118, 255});
    fill_rect(r, x + 15, y + 20, 7, 3, (SDL_Color){82, 101, 118, 255});
}

void draw_frostfell_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int wx = map_x * TILE_SIZE;
    int wy = map_y * TILE_SIZE;
    unsigned int seed = dragonspine_noise(map_x, map_y);
    int shade = dragonspine_shade(wx, wy, 96, 72) / 64;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){217 + shade * 3, 229 + shade * 2, 239 + shade, 255});
    // The mountain renderer's clipped patches also let these wind drifts
    // continue across snow tiles without a visible tile-sized repeat.
    for (int gy = wy / 38 - 1; gy <= (wy + TILE_SIZE) / 38; gy++) {
        for (int gx = wx / 76 - 1; gx <= (wx + TILE_SIZE) / 76; gx++) {
            unsigned int drift = dragonspine_noise(gx, gy);
            int px = gx * 76 + (int)(drift % 30u) - wx;
            int py = gy * 38 + (int)((drift >> 8) % 23u) - wy;
            int width = 34 + (int)((drift >> 16) % 25u);
            for (int row = 0; row < 7; row++) {
                int inset = abs(row - 3) * abs(row - 3);
                dragonspine_patch(r, tile_x, tile_y, px + inset, py + row, width - inset * 2, 1,
                    row < 4 ? (SDL_Color){239, 246, 250, 255} : (SDL_Color){199, 216, 232, 255});
            }
        }
    }
    if (seed % 31u == 0u) {
        int px = x + 5 + (int)((seed >> 8) % 10u);
        int py = y + 8 + (int)((seed >> 16) % 9u);
        fill_rect(r, px + 1, py, 5, 2, (SDL_Color){244, 249, 252, 255});
        fill_rect(r, px, py + 2, 7, 2, (SDL_Color){142, 167, 190, 255});
        fill_rect(r, px + 2, py + 4, 4, 1, (SDL_Color){186, 205, 222, 255});
    } else if (seed % 43u == 0u) {
        for (int step = 0; step < 9; step++) {
            fill_rect(r, x + 6 + step, y + 10 + step / 3, 1, 1, (SDL_Color){120, 126, 132, 255});
        }
        fill_rect(r, x + 10, y + 9, 1, 3, (SDL_Color){120, 126, 132, 255});
        fill_rect(r, x + 7, y + 10, 5, 1, (SDL_Color){249, 252, 254, 255});
    }
}

static void draw_frostfell_pine(Renderer *r, int x, int y, unsigned int seed) {
    int height = 16 + (int)(seed % 6u);
    int cx = x + 10 + (int)((seed >> 8) % 5u);
    int top = y + 22 - height;
    fill_rect(r, cx - 5, y + 21, 12, 2, (SDL_Color){109, 155, 180, 255});
    fill_rect(r, cx - 1, y + 18, 3, 5, (SDL_Color){83, 70, 61, 255});
    for (int row = 0; row < height - 2; row++) {
        int width = 1 + row * 8 / height;
        int tier = row % 6;
        width += tier >= 3 ? 2 : 0;
        fill_rect(r, cx - width, top + row, width * 2 + 1, 1, (SDL_Color){37, 74, 85, 255});
        fill_rect(r, cx - width, top + row, width, 1, (SDL_Color){56, 104, 112, 255});
        if (tier == 0 || tier == 1 || row == height - 3) {
            int snow_width = width - (int)((seed >> (row % 16)) & 1u);
            fill_rect(r, cx - width, top + row, snow_width * 2 + 1, 1, (SDL_Color){239, 247, 252, 255});
        }
    }
}

// Connected glacier shelves: shade the whole formation, exposing cliff
// faces only where the neighboring tile is open terrain.
void draw_frostfell_wall(Renderer *r, const Map *map, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int wx = map_x * TILE_SIZE;
    int wy = map_y * TILE_SIZE;
    unsigned int seed = dragonspine_noise(map_x, map_y);
    int north = map_y > 0 && map->tiles[map_y - 1][map_x] != TILE_FROST_WALL;
    int south = map_y < MAP_H - 1 && map->tiles[map_y + 1][map_x] != TILE_FROST_WALL;
    int west = map_x > 0 && map->tiles[map_y][map_x - 1] != TILE_FROST_WALL;
    int east = map_x < MAP_W - 1 && map->tiles[map_y][map_x + 1] != TILE_FROST_WALL;
    SDL_Rect patches[6][64];
    int counts[6] = {0};
    for (int py = 0; py < TILE_SIZE; py += 3) {
        for (int px = 0; px < TILE_SIZE; px += 3) {
            int shade = (dragonspine_shade(wx + px, wy + py, 90, 66) * 3 +
                dragonspine_shade(wx + px, wy + py, 24, 33)) / 171;
            patches[shade][counts[shade]++] = (SDL_Rect){x + px, y + py, 3, 3};
        }
    }
    for (int shade = 0; shade < 6; shade++) {
        if (counts[shade] > 0) {
            SDL_SetRenderDrawColor(r->sdl, 118 + shade * 16, 171 + shade * 11, 202 + shade * 9, 255);
            SDL_RenderFillRects(r->sdl, patches[shade], counts[shade]);
        }
    }
    SDL_Color snow = {238, 247, 251, 255};
    for (int step = 0; step < TILE_SIZE; step += 2) {
        int depth = 2 + dragonspine_shade(wx + step, wy, 12, 18) / 70;
        if (north) {
            fill_rect(r, x + step, y, 2, depth + 1, (SDL_Color){92, 145, 180, 255});
            fill_rect(r, x + step, y + 1, 2, depth, snow);
        }
        if (south) {
            int face = 7 + dragonspine_shade(wx + step, wy + TILE_SIZE, 18, 12) / 52;
            fill_rect(r, x + step, y + TILE_SIZE - face, 2, face, (SDL_Color){62, 112, 151, 255});
            fill_rect(r, x + step, y + TILE_SIZE - face + 2, 2, face / 2, (SDL_Color){91, 146, 181, 255});
            fill_rect(r, x + step, y + TILE_SIZE - face, 2, 2, snow);
            unsigned int icicle = dragonspine_noise((wx + step) / 7, map_y);
            if (icicle % 3u == 0u && (wx + step) % 7 < 2) {
                int length = 2 + (int)((icicle >> 8) % 4u);
                fill_rect(r, x + step, y + TILE_SIZE - face + 2, 1, length, (SDL_Color){180, 222, 239, 255});
            }
            fill_rect(r, x + step, y + TILE_SIZE - 2, 2, 2, (SDL_Color){45, 87, 124, 255});
        }
        if (west) {
            fill_rect(r, x, y + step, depth + 1, 2, (SDL_Color){220, 238, 246, 255});
        }
        if (east) {
            fill_rect(r, x + TILE_SIZE - depth - 1, y + step, depth + 1, 2, (SDL_Color){69, 119, 157, 255});
            fill_rect(r, x + TILE_SIZE - depth - 2, y + step, 1, 2, snow);
        }
    }
    for (int cy = 0; cy < 2; cy++) {
        for (int cx = 0; cx < 2; cx++) {
            if ((cy == 0 ? north : south) && (cx == 0 ? west : east)) {
                fill_rect(r, x + cx * (TILE_SIZE - 3), y + cy * (TILE_SIZE - 3), 3, 3,
                    (SDL_Color){217, 229, 239, 255});
            }
        }
    }
    // Broad noise groups pines into stands rather than a regular scatter.
    if (!south && dragonspine_shade(wx, wy, 96, 72) > 142 && seed % 3u != 0u) {
        draw_frostfell_pine(r, x, y, seed);
    }
}

// Decorative expedition traces stay on safe snow and cannot become pickups.
void draw_frostfell_details(Renderer *r, const Map *map, int tx, int ty, int mx, int my) {
    if (map->tiles[my][mx] != TILE_FROST_FLOOR) {
        return;
    }
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = dragonspine_noise(mx, my);
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int qx = mx + dx;
            int qy = my + dy;
            if (qx < 0 || qy < 0 || qx >= MAP_W || qy >= MAP_H ||
                (map->tiles[qy][qx] != TILE_FROST_JOURNAL && map->tiles[qy][qx] != TILE_NPC_FROST_SURVIVOR)) {
                continue;
            }
            if (dy == 0 || dx == 0) {
                for (int step = 0; step < 3; step++) {
                    int px = x + (dy == 0 ? 4 + step * 7 : 9 + (step % 2) * 4);
                    int py = y + (dx == 0 ? 4 + step * 7 : 9 + (step % 2) * 4);
                    fill_rect(r, px, py, 2, 3, (SDL_Color){169, 194, 215, 255});
                    fill_rect(r, px, py, 1, 1, (SDL_Color){204, 222, 235, 255});
                }
            } else if (seed % 3u == 0u) {
                fill_rect(r, x + 6, y + 10, 11, 7, (SDL_Color){111, 89, 72, 255});
                fill_rect(r, x + 7, y + 11, 9, 1, (SDL_Color){166, 139, 108, 255});
                fill_rect(r, x + 9, y + 11, 1, 5, (SDL_Color){69, 65, 65, 255});
                fill_rect(r, x + 14, y + 11, 1, 5, (SDL_Color){69, 65, 65, 255});
                fill_rect(r, x + 5, y + 9, 9, 3, (SDL_Color){242, 249, 252, 255});
                fill_rect(r, x + 9, y + 16, 10, 3, (SDL_Color){224, 237, 246, 255});
            }
            return;
        }
    }
}

// Blue-flagged stakes mark the trail in and out of each Frostfell stage.
void draw_frostfell_edge(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color wood = {92, 64, 42, 255};
    SDL_Color flag = {58, 116, 196, 255};
    draw_frostfell_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 3, y + 1, 2, 9, wood);
    fill_rect(r, x + 5, y + 1, 5, 3, flag);
    fill_rect(r, x + 17, y + 13, 2, 9, wood);
    fill_rect(r, x + 19, y + 13, 5, 3, flag);
}

void draw_glassdeep_floor(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = forest_tile_seed(mx, my);
    int shade = (int)(seed % 5u);
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){143 + shade, 135 + shade, 108 + shade, 255});
    for (int n = 0; n < 4; n++) {
        int px = (int)((seed >> (n * 3)) % 20u) + 1;
        int py = (int)((seed >> (n * 4 + 2)) % 21u) + 1;
        fill_rect(r, x + px, y + py, 2 + n % 2, 1, (SDL_Color){174, 167, 139, 255});
        fill_rect(r, x + px + 1, y + py + 1, 2, 1, (SDL_Color){116, 111, 89, 255});
    }
    if (seed % 19u == 0) {
        fill_rect(r, x + 7, y + 12, 4, 2, (SDL_Color){94, 117, 68, 255});
        fill_rect(r, x + 10, y + 13, 3, 2, (SDL_Color){112, 133, 79, 255});
    }
}

void draw_glassdeep_wall(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = forest_tile_seed(mx, my);
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){85, 79, 65, 255});
    for (int row = 0; row < 2; row++) {
        int shade = (int)((seed >> (row * 3)) % 12u);
        int top = y + row * 12;
        fill_rect(r, x, top, TILE_SIZE, 10, (SDL_Color){155 + shade, 147 + shade, 123 + shade, 255});
        fill_rect(r, x, top, TILE_SIZE, 1, (SDL_Color){205, 197, 170, 255});
        fill_rect(r, x, top + 10, TILE_SIZE, 1, (SDL_Color){111, 104, 84, 255});
        int seam = (int)((seed >> (row * 4)) % 18u) + 3;
        fill_rect(r, x + seam, top + 1, 1, 9, (SDL_Color){127, 120, 97, 255});
        fill_rect(r, x + seam + 2, top + 5, 4, 1, (SDL_Color){173, 165, 139, 255});
    }
    if (seed % 11u == 0) {
        fill_rect(r, x + 5, y + 13, 7, 2, (SDL_Color){92, 109, 65, 255});
    }
}

void draw_glassdeep_pool(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = forest_tile_seed(mx, my);
    int ripple = (int)((SDL_GetTicks() / AMBIENT_FRAME_MS + seed) % 4u);
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){22, 84, 94, 255});
    fill_rect(r, x + 2, y + 4 + ripple, 14, 1, (SDL_Color){39, 119, 124, 255});
    fill_rect(r, x + 7, y + 16 - ripple, 13, 1, (SDL_Color){65, 151, 147, 255});
    fill_rect(r, x + 12, y + 5 + ripple, 3, 1, (SDL_Color){126, 185, 169, 255});
}

void draw_glassdeep_ruin(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    draw_glassdeep_floor(r, tx, ty, mx, my);
    for (int row = 0; row < 4; row++) {
        fill_rect(r, x + 1, y + 2 + row * 5, 22, 4, (SDL_Color){114, 83, 49, 255});
        fill_rect(r, x + 2, y + 2 + row * 5, 20, 1, (SDL_Color){159, 119, 70, 255});
        fill_rect(r, x + 3, y + 3 + row * 5, 1, 1, (SDL_Color){53, 51, 44, 255});
        fill_rect(r, x + 20, y + 3 + row * 5, 1, 1, (SDL_Color){53, 51, 44, 255});
    }
}

void draw_glassdeep_edge(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    draw_glassdeep_floor(r, tx, ty, mx, my);
    // Rails and sleepers mark the boundary ramp; no floating doorway overlay.
    for (int row = 3; row < 24; row += 6) {
        fill_rect(r, x + 3, y + row, 18, 3, (SDL_Color){91, 65, 39, 255});
        fill_rect(r, x + 3, y + row, 18, 1, (SDL_Color){145, 105, 59, 255});
    }
    fill_rect(r, x + 6, y, 2, TILE_SIZE, (SDL_Color){63, 72, 65, 255});
    fill_rect(r, x + 17, y, 2, TILE_SIZE, (SDL_Color){133, 143, 128, 255});
}

void draw_ashen_floor(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = forest_tile_seed(mx, my);
    int shade = (int)(seed % 5u);
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){51 + shade, 46 + shade, 49 + shade, 255});
    fill_rect(r, x + 3 + (int)(seed % 12u), y + 4 + (int)((seed >> 5) % 14u), 4, 1,
        (SDL_Color){80, 72, 72, 255});
    fill_rect(r, x + 7, y + 18, 3, 2, (SDL_Color){37, 32, 39, 255});
    if (seed % 11u == 0) {
        int ember = (int)((SDL_GetTicks() / AMBIENT_FRAME_MS + seed) % 3u);
        fill_rect(r, x + 16, y + 11, 2, 1, (SDL_Color){188 + ember * 20, 80 + ember * 15, 34, 255});
    }
}

void draw_ashen_wall(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = forest_tile_seed(mx, my);
    SDL_Color stone = {30, 27, 36, 255};
    SDL_Color rim = {77, 65, 74, 255};
    draw_ashen_floor(r, tx, ty, mx, my);
    if (seed % 5u == 0) {
        // A charred tree's silhouette leaves ash visible between branches.
        fill_rect(r, x + 10, y + 4, 5, 20, stone);
        fill_rect(r, x + 11, y + 4, 1, 19, rim);
        fill_rect(r, x + 4, y + 8, 7, 3, stone);
        fill_rect(r, x + 3, y + 4, 3, 6, stone);
        fill_rect(r, x + 14, y + 12, 7, 3, stone);
        fill_rect(r, x + 19, y + 6, 3, 7, stone);
        return;
    }
    if (seed % 5u == 1) {
        // Broken stonework from a settlement swallowed by the volcano.
        fill_rect(r, x + 2, y + 12, 20, 10, stone);
        fill_rect(r, x + 3, y + 7, 8, 6, rim);
        fill_rect(r, x + 12, y + 10, 9, 3, rim);
        fill_rect(r, x + 4, y + 16, 16, 1, rim);
        fill_rect(r, x + 10, y + 17, 1, 4, rim);
        return;
    }
    for (int column = 0; column < 3; column++) {
        int top = 2 + (int)((seed >> (column * 4)) % 8u);
        int left = x + column * 8 + 1;
        fill_rect(r, left, y + top + 2, 7, 22 - top, stone);
        fill_rect(r, left + 1, y + top, 5, 3, rim);
        fill_rect(r, left + 1, y + top + 3, 1, 18 - top, rim);
    }
    if (seed % 7u == 0) {
        fill_rect(r, x + 12, y + 12, 1, 9, (SDL_Color){191, 77, 39, 255});
        fill_rect(r, x + 9, y + 18, 4, 1, (SDL_Color){191, 77, 39, 255});
    }
}

void draw_ashen_lava(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    unsigned int seed = forest_tile_seed(mx, my);
    int flow = (int)((SDL_GetTicks() / AMBIENT_FRAME_MS + seed) % 6u);
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){159, 43, 26, 255});
    fill_rect(r, x, y + 4 + flow, TILE_SIZE, 4, (SDL_Color){228, 91, 30, 255});
    fill_rect(r, x, y + 16 - flow, TILE_SIZE, 3, (SDL_Color){246, 135, 41, 255});
    fill_rect(r, x + 5 + flow, y + 7, 3, 2, (SDL_Color){255, 205, 95, 255});
    fill_rect(r, x + 16 - flow, y + 19, 2, 2, (SDL_Color){255, 182, 69, 255});
}

void draw_ashen_ruin(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    draw_ashen_floor(r, tx, ty, mx, my);
    fill_rect(r, x + 2, y + 4, 10, 7, (SDL_Color){105, 85, 78, 255});
    fill_rect(r, x + 13, y + 6, 8, 6, (SDL_Color){87, 73, 72, 255});
    fill_rect(r, x + 5, y + 14, 13, 7, (SDL_Color){95, 77, 71, 255});
    fill_rect(r, x + 6, y + 15, 10, 1, (SDL_Color){153, 114, 89, 255});
}

void draw_ashen_edge(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    draw_ashen_floor(r, tx, ty, mx, my);
    fill_rect(r, x + 1, y + 1, 5, 22, (SDL_Color){101, 78, 76, 255});
    fill_rect(r, x + 18, y + 1, 5, 22, (SDL_Color){101, 78, 76, 255});
    fill_rect(r, x + 2, y + 3, 3, 3, (SDL_Color){240, 136, 52, 255});
    fill_rect(r, x + 19, y + 3, 3, 3, (SDL_Color){240, 136, 52, 255});
}

static void desert_dune_patch(SDL_Rect *rects, int *count, int x, int y, int px, int py, int w, int h) {
    int right = px + w < TILE_SIZE ? px + w : TILE_SIZE;
    int bottom = py + h < TILE_SIZE ? py + h : TILE_SIZE;
    px = px < 0 ? 0 : px;
    py = py < 0 ? 0 : py;
    if (right > px && bottom > py) {
        rects[(*count)++] = (SDL_Rect){x + px, y + py, right - px, bottom - py};
    }
}

void draw_desert_floor(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    int wx = mx * TILE_SIZE;
    int wy = my * TILE_SIZE;
    unsigned int seed = dragonspine_noise(mx, my);
    int shade = dragonspine_shade(wx, wy, 120, 90) / 64;
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){211 + shade * 3, 171 + shade * 3, 102 + shade * 2, 255});
    // Clip dunes in world coordinates so ridges join across tiles. At most
    // nine anchors contribute thirteen clipped segments each per color.
    SDL_Rect dunes[3][128];
    int counts[3] = {0};
    for (int gy = wy / 46 - 1; gy <= (wy + TILE_SIZE) / 46; gy++) {
        for (int gx = wx / 90 - 1; gx <= (wx + TILE_SIZE) / 90; gx++) {
            unsigned int dune = dragonspine_noise(gx + 31, gy + 7);
            int px = gx * 90 + (int)(dune % 28u) - wx;
            int py = gy * 46 + (int)((dune >> 8) % 26u) - wy;
            int width = 47 + (int)((dune >> 16) % 28u);
            int height = (width / 2) * (width / 2) / 100 + width / 12 + 7;
            if (px >= TILE_SIZE || px + width + 3 <= 0 || py >= TILE_SIZE || py + height <= 0) {
                continue;
            }
            for (int step = 0; step < width; step += 2) {
                int curve = (step - width / 2) * (step - width / 2) / 100;
                int cy = py + curve + step / 12;
                int span = step + 1 < width ? 2 : 1;
                desert_dune_patch(dunes[0], &counts[0], x, y, px + step, cy, span, 2);
                desert_dune_patch(dunes[1], &counts[1], x, y, px + step, cy + 2, span, 2);
                desert_dune_patch(dunes[2], &counts[2], x, y, px + step + 3, cy + 6, span, 1);
            }
        }
    }
    const SDL_Color colors[3] = {{239, 199, 128, 255}, {192, 151, 87, 255}, {222, 182, 109, 255}};
    for (int color = 0; color < 3; color++) {
        if (counts[color] > 0) {
            SDL_SetRenderDrawColor(r->sdl, colors[color].r, colors[color].g, colors[color].b, 255);
            SDL_RenderFillRects(r->sdl, dunes[color], counts[color]);
        }
    }
    if (seed % 19u == 0u) {
        int px = x + 5 + (int)((seed >> 8) % 10u);
        int py = y + 7 + (int)((seed >> 16) % 10u);
        fill_rect(r, px + 1, py, 3, 1, (SDL_Color){179, 135, 83, 255});
        fill_rect(r, px, py + 1, 5, 2, (SDL_Color){149, 107, 66, 255});
        fill_rect(r, px + 1, py + 1, 3, 1, (SDL_Color){211, 172, 111, 255});
        fill_rect(r, px + 7, py + 4, 2, 1, (SDL_Color){175, 134, 78, 255});
    } else if (seed % 47u == 0u) {
        fill_rect(r, x + 11, y + 13, 1, 5, (SDL_Color){137, 113, 61, 255});
        fill_rect(r, x + 8, y + 12, 1, 3, (SDL_Color){155, 129, 66, 255});
        fill_rect(r, x + 9, y + 15, 5, 1, (SDL_Color){137, 113, 61, 255});
        fill_rect(r, x + 15, y + 11, 1, 4, (SDL_Color){155, 129, 66, 255});
    }
}

static void draw_desert_cactus(Renderer *r, int x, int y, unsigned int seed) {
    SDL_Color cactus = {44, 71, 39, 255};
    SDL_Color light = {90, 116, 53, 255};
    SDL_Color shade = {27, 49, 31, 255};
    fill_rect(r, x + 5, y + 21, 15, 2, (SDL_Color){106, 81, 48, 255});
    if (seed & 1u) {
        int trunk = x + 9 + (int)((seed >> 5) % 4u);
        int top = y + 2 + (int)((seed >> 9) % 4u);
        fill_rect(r, trunk + 1, top, 4, 2, cactus);
        fill_rect(r, trunk, top + 2, 6, y + 22 - top - 2, cactus);
        fill_rect(r, trunk + 1, top + 3, 1, y + 20 - top - 3, light);
        fill_rect(r, trunk + 4, top + 3, 2, y + 22 - top - 3, shade);
        int arm = y + 9 + (int)((seed >> 13) % 4u);
        fill_rect(r, trunk - 5, arm, 6, 4, cactus);
        fill_rect(r, trunk - 6, arm - 6, 4, 8, cactus);
        fill_rect(r, trunk - 5, arm - 5, 1, 6, light);
        fill_rect(r, trunk - 5, arm, 5, 1, light);
        if ((seed >> 17) & 1u) {
            fill_rect(r, trunk + 5, arm + 3, 5, 4, cactus);
            fill_rect(r, trunk + 8, arm - 4, 4, 9, cactus);
            fill_rect(r, trunk + 8, arm - 3, 1, 7, light);
            fill_rect(r, trunk + 10, arm - 3, 2, 10, shade);
        }
        fill_rect(r, trunk + 2, top + 7, 1, 2, shade);
        fill_rect(r, trunk + 2, top + 13, 1, 2, shade);
    } else {
        // Short barrel cacti have rounded shoulders, ribs, and pale spines.
        int height = 10 + (int)((seed >> 8) % 5u);
        int top = y + 22 - height;
        fill_rect(r, x + 9, top, 7, 2, cactus);
        fill_rect(r, x + 7, top + 2, 11, height - 4, cactus);
        fill_rect(r, x + 8, y + 20, 9, 2, shade);
        fill_rect(r, x + 8, top + 3, 2, height - 5, light);
        fill_rect(r, x + 12, top + 2, 1, height - 3, light);
        fill_rect(r, x + 16, top + 3, 2, height - 5, shade);
        for (int row = 0; row < 3; row++) {
            fill_rect(r, x + 9 + row % 2, top + 3 + row * 3, 1, 1, (SDL_Color){203, 188, 113, 255});
            fill_rect(r, x + 14, top + 4 + row * 3, 1, 1, (SDL_Color){203, 188, 113, 255});
        }
        if (seed % 5u == 0u) {
            fill_rect(r, x + 11, top - 1, 3, 2, (SDL_Color){189, 106, 76, 255});
        }
    }
}

static void draw_desert_boulder(Renderer *r, int x, int y, unsigned int seed) {
    // Stepped outlines form a spire, rounded boulder, leaning rock, or broad mesa.
    static const int profiles[4][6][2] = {
        {{9, 5}, {7, 9}, {6, 11}, {4, 14}, {3, 17}, {2, 19}},
        {{6, 11}, {3, 17}, {1, 21}, {2, 20}, {3, 18}, {5, 14}},
        {{12, 7}, {9, 11}, {6, 14}, {4, 16}, {2, 18}, {1, 20}},
        {{4, 14}, {2, 18}, {3, 17}, {5, 15}, {4, 16}, {2, 19}}
    };
    int shape = (int)((seed >> 7) % 4u);
    int height = 2 + (int)((seed >> 24) & 1u);
    int top = 22 - height * 6 - (int)((seed >> 12) % 3u);
    SDL_Color stone = {184 + (int)((seed >> 16) % 12u), 128, 73, 255};
    SDL_Color light = {217, 166, 101, 255};
    SDL_Color shade = {136, 87, 49, 255};
    for (int band = 0; band < 6; band++) {
        int offset = profiles[shape][band][0];
        int width = profiles[shape][band][1];
        int left = x + (((seed >> 22) & 1u) ? TILE_SIZE - offset - width : offset);
        int row = y + top + band * height;
        fill_rect(r, left, row, width, height, stone);
        fill_rect(r, left, row, 2, height, light);
        fill_rect(r, left + width - 3, row, 3, height, shade);
        if (band == 0 || band == 3) {
            fill_rect(r, left + 2, row, width - 5, 1, light);
        } else if (band == 2 || band == 5) {
            fill_rect(r, left + 2, row + height - 1, width - 5, 1, shade);
        }
    }
    int crack = x + 10 + (int)((seed >> 20) % 5u);
    fill_rect(r, crack, y + top + height * 2, 1, height + 1, shade);
    fill_rect(r, crack - 2, y + top + height * 3, 3, 1, shade);
    fill_rect(r, crack - 2, y + top + height * 3 + 1, 1, height, shade);
}

// Joined sandstone outcrops retain boulders and cactus stands without
// outlining a separate rock in every blocked tile.
static int desert_rock(TileType tile) {
    return tile == TILE_DESERT_WALL || tile == TILE_DESERT_MESA;
}

void draw_desert_wall(Renderer *r, const Map *map, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    int wx = mx * TILE_SIZE;
    int wy = my * TILE_SIZE;
    unsigned int seed = dragonspine_noise(mx, my);
    int north = my > 0 && !desert_rock(map->tiles[my - 1][mx]);
    int south = my < MAP_H - 1 && !desert_rock(map->tiles[my + 1][mx]);
    int west = mx > 0 && !desert_rock(map->tiles[my][mx - 1]);
    int east = mx < MAP_W - 1 && !desert_rock(map->tiles[my][mx + 1]);
    SDL_Rect patches[7][32];
    int counts[7] = {0};
    for (int py = 0; py < TILE_SIZE; py += 3) {
        for (int px = 0; px < TILE_SIZE; px += 6) {
            int broad = dragonspine_shade(wx + px, wy + py, 90, 72);
            int detail = dragonspine_shade(wx + px, wy + py, 27, 39);
            int strata = ((wy + py + broad / 24) / 7) % 4 == 0;
            int shade = (broad * 3 + detail) / 171 + strata;
            patches[shade][counts[shade]++] = (SDL_Rect){x + px, y + py, 6, 3};
        }
    }
    for (int shade = 0; shade < 7; shade++) {
        if (counts[shade] > 0) {
            SDL_SetRenderDrawColor(r->sdl, 132 + shade * 14, 83 + shade * 11, 49 + shade * 7, 255);
            SDL_RenderFillRects(r->sdl, patches[shade], counts[shade]);
        }
    }
    for (int step = 0; step < TILE_SIZE; step += 2) {
        int horizontal = 2 + dragonspine_shade(wx + step, wy, 18, 21) / 70;
        int vertical = 2 + dragonspine_shade(wx, wy + step, 21, 18) / 70;
        if (north) {
            fill_rect(r, x + step, y, 2, horizontal, (SDL_Color){230, 177, 110, 255});
            fill_rect(r, x + step, y + horizontal, 2, 1, (SDL_Color){153, 101, 58, 255});
        }
        if (south) {
            int face = 8 + dragonspine_shade(wx + step, wy + TILE_SIZE, 27, 18) / 50;
            fill_rect(r, x + step, y + TILE_SIZE - face, 2, face, (SDL_Color){121, 77, 45, 255});
            fill_rect(r, x + step, y + TILE_SIZE - face, 2, 2, (SDL_Color){222, 162, 96, 255});
            fill_rect(r, x + step, y + TILE_SIZE - face + 4, 2, 2, (SDL_Color){166, 108, 62, 255});
            fill_rect(r, x + step, y + TILE_SIZE - 2, 2, 2, (SDL_Color){90, 62, 40, 255});
        }
        if (west) {
            fill_rect(r, x, y + step, vertical, 2, (SDL_Color){218, 159, 95, 255});
        }
        if (east) {
            fill_rect(r, x + TILE_SIZE - vertical, y + step, vertical, 2, (SDL_Color){106, 73, 47, 255});
            fill_rect(r, x + TILE_SIZE - vertical - 1, y + step, 1, 2, (SDL_Color){181, 126, 74, 255});
        }
    }
    for (int cy = 0; cy < 2; cy++) {
        for (int cx = 0; cx < 2; cx++) {
            if ((cy == 0 ? north : south) && (cx == 0 ? west : east)) {
                int size = 3 + (int)((seed >> (cx + cy * 2)) & 1u);
                fill_rect(r, x + cx * (TILE_SIZE - size), y + cy * (TILE_SIZE - size), size, size,
                    (SDL_Color){217, 177, 106, 255});
            }
        }
    }
    int cactus = (dragonspine_shade(wx, wy, 120, 96) > 140 && seed % 3u == 0u) || seed % 19u == 0u;
    if (cactus && !south) {
        draw_desert_cactus(r, x, y, seed);
    } else if ((!north && !south && !west && !east && seed % 11u == 0u) || (north && south && west && east)) {
        draw_desert_boulder(r, x, y, seed);
    }
}

void draw_desert_edge(Renderer *r, int tx, int ty, int mx, int my) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    draw_desert_floor(r, tx, ty, mx, my);
    fill_rect(r, x + 3, y + 1, 2, 9, (SDL_Color){99, 65, 38, 255});
    fill_rect(r, x + 5, y + 1, 5, 3, (SDL_Color){176, 57, 37, 255});
    fill_rect(r, x + 17, y + 13, 2, 9, (SDL_Color){99, 65, 38, 255});
    fill_rect(r, x + 19, y + 13, 5, 3, (SDL_Color){176, 57, 37, 255});
}

static void draw_frostfell_sheet(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int slick) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int wx = map_x * TILE_SIZE;
    int wy = map_y * TILE_SIZE;
    int shade = dragonspine_shade(wx, wy, 72, 60) / 48;
    SDL_Color base = slick ? (SDL_Color){139 + shade * 3, 207 + shade * 2, 216 + shade, 255} :
        (SDL_Color){159 + shade * 3, 199 + shade * 2, 225 + shade * 2, 255};
    SDL_Color crack = slick ? (SDL_Color){112, 181, 194, 255} : (SDL_Color){125, 170, 205, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, base);
    for (int gy = wy / 60 - 1; gy <= (wy + TILE_SIZE) / 60 + 1; gy++) {
        for (int gx = wx / 72 - 1; gx <= (wx + TILE_SIZE) / 72; gx++) {
            unsigned int seed = dragonspine_noise(gx + 17, gy + 29);
            int px = gx * 72 + (int)(seed % 22u) - wx;
            int py = gy * 60 + (int)((seed >> 8) % 25u) - wy;
            int dir = seed & 1u ? 1 : -1;
            for (int step = 0; step < 52; step++) {
                int bend = step / 19;
                int cy = py + dir * (step / 2 + bend * 3);
                dragonspine_patch(r, tile_x, tile_y, px + step, cy, 1, 1, crack);
                if (step >= 23 && step < 38) {
                    dragonspine_patch(r, tile_x, tile_y, px + 23 + (step - 23) / 2,
                        py + dir * 14 - dir * (step - 23), 1, 1, crack);
                }
            }
            // A few slanted reflections, spaced across the sheet rather
            // than stamped once into every tile.
            if (seed % 3u == 0u || slick) {
                for (int step = 0; step < 12; step++) {
                    dragonspine_patch(r, tile_x, tile_y, px + 13 + step * 2,
                        py + 24 - step, 2, 1, (SDL_Color){224, 244, 250, 255});
                }
            }
        }
    }
}

// Clear lake ice stays blue; slick ice uses a separate teal palette.
void draw_frostfell_lake(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_frostfell_sheet(r, tile_x, tile_y, map_x, map_y, 0);
}

void draw_frostfell_bank(Renderer *r, int tx, int ty, int mx, int my, unsigned int edges) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    for (int step = 0; step < TILE_SIZE; step++) {
        int horizontal = 2 + dragonspine_shade(mx * TILE_SIZE + step, my * TILE_SIZE, 12, 18) / 65;
        int vertical = 2 + dragonspine_shade(mx * TILE_SIZE, my * TILE_SIZE + step, 18, 12) / 65;
        SDL_Color snow = {232, 243, 249, 255};
        SDL_Color rim = {115, 162, 197, 255};
        if (edges & FROST_EDGE_NORTH) {
            fill_rect(r, x + step, y, 1, horizontal, snow);
            fill_rect(r, x + step, y + horizontal, 1, 1, rim);
        }
        if (edges & FROST_EDGE_SOUTH) {
            fill_rect(r, x + step, y + TILE_SIZE - horizontal, 1, horizontal, snow);
            fill_rect(r, x + step, y + TILE_SIZE - horizontal - 1, 1, 1, rim);
        }
        if (edges & FROST_EDGE_WEST) {
            fill_rect(r, x, y + step, vertical, 1, snow);
            fill_rect(r, x + vertical, y + step, 1, 1, rim);
        }
        if (edges & FROST_EDGE_EAST) {
            fill_rect(r, x + TILE_SIZE - vertical, y + step, vertical, 1, snow);
            fill_rect(r, x + TILE_SIZE - vertical - 1, y + step, 1, 1, rim);
        }
    }
}

// A dark hole in the lake ice. On the Kraken's warning turn a tentacle rises
// out of it.
void draw_frostfell_lake_hole(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int tentacle) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_frostfell_lake(r, tile_x, tile_y, map_x, map_y);
    unsigned int seed = dragonspine_noise(map_x, map_y);
    for (int row = -7; row <= 7; row++) {
        int width = 9 - row * row / 9;
        int shift = (int)((seed >> (row + 7)) & 1u);
        fill_rect(r, x + 12 - width + shift, y + 12 + row, width * 2, 1, (SDL_Color){231, 246, 252, 255});
        if (width > 2 && abs(row) < 6) {
            fill_rect(r, x + 14 - width + shift, y + 12 + row, width * 2 - 4, 1, (SDL_Color){22, 48, 78, 255});
        }
    }
    fill_rect(r, x + 7, y + 16, 9, 1, (SDL_Color){61, 104, 143, 255});
    fill_rect(r, x + 4, y + 10, 2, 3, (SDL_Color){155, 205, 230, 255});
    fill_rect(r, x + 16, y + 6, 3, 2, (SDL_Color){249, 253, 255, 255});
    if (!tentacle) {
        return;
    }
    SDL_Color flesh = {200, 168, 218, 255};
    SDL_Color sucker = {246, 226, 240, 255};
    fill_rect(r, x + 8, y + 1, 7, 13, flesh);
    fill_rect(r, x + 13, y - 2, 5, 5, flesh);
    fill_rect(r, x + 17, y - 4, 4, 4, flesh);
    fill_rect(r, x + 10, y + 4, 3, 2, sucker);
    fill_rect(r, x + 10, y + 8, 3, 2, sucker);
    fill_rect(r, x + 14, y, 2, 2, sucker);
}

// Slick ice: a pale teal sheet with a diagonal glare, so it reads apart from
// the snow, the bluer Kraken lake and the glacier walls.
void draw_frostfell_ice(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_frostfell_sheet(r, tile_x, tile_y, map_x, map_y, 1);
}

// Thin ice: greyer and darker than the snow it bridges, split by a fracture
// with dark water showing through, so it reads as fragile before a step.
// Alternate tiles mirror their side cracks so a shortcut never looks stamped.
void draw_frostfell_thin_ice(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = (unsigned int)map_x * 89u + (unsigned int)map_y * 47u;
    int dir = seed % 2u == 0u ? 1 : -1;
    SDL_Color crack = {84, 116, 148, 255};
    SDL_Color water = {40, 70, 104, 255};
    SDL_Color glint = {236, 246, 252, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, (SDL_Color){180, 201, 220, 255});
    int cx = x + 9 + (int)(seed % 6u);
    int cy = y + 9 + (int)(seed % 5u);
    fill_rect(r, x + 1, cy, TILE_SIZE - 2, 2, crack);
    fill_rect(r, x + 4, cy, TILE_SIZE - 8, 1, water);
    for (int step = 0; step < 4; step++) {
        fill_rect(r, cx + dir * step * 2, cy + 2 + step * 2, 2, 1, crack);
    }
    fill_rect(r, cx - dir * 4, cy - 6, 1, 6, crack);
    fill_rect(r, x + 4, y + 4, 3, 1, glint);
    fill_rect(r, x + 15, y + 19, 2, 1, glint);
}

void draw_dragonspine_gate(Renderer *r, int tile_x, int tile_y) {
    draw_town_path(r, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_ARCH, (SDL_Color){164, 188, 202, 255});
}

void draw_dragon_goblet(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_dragonspine_floor(r, tile_x, tile_y, map_x, map_y, 2);
    draw_world_prop(r, tile_x, tile_y, PROP_GOBLET, (SDL_Color){244, 195, 70, 255});
}

void draw_dragon_seeker(Renderer *r, int tile_x, int tile_y) {
    draw_tavern_floor(r, tile_x, tile_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_ILYA, 0);
}

void draw_forest_warden(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    npc_draw(r, tile_x, tile_y, NPC_ART_WARDEN, 0);
}

void draw_guild_seeker(Renderer *r, int tx, int ty) {
    draw_tavern_floor(r, tx, ty);
    npc_draw(r, tx, ty, NPC_ART_ZARA, 0);
}

void draw_desert_lamp(Renderer *r, int tx, int ty, int mx, int my) {
    draw_desert_floor(r, tx, ty, mx, my);
    draw_world_prop(r, tx, ty, PROP_LAMP, (SDL_Color){109, 231, 213, 255});
}

void draw_town_path(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color mortar = {43, 39, 36, 255};
    SDL_Color stones[3] = {
        {91, 91, 82, 255},
        {105, 101, 88, 255},
        {76, 79, 75, 255}
    };
    SDL_Color highlight = {126, 121, 103, 255};
    SDL_Color shadow = {58, 57, 54, 255};
    int narrow_x[3] = {1, 8, 18};
    int narrow_w[3] = {6, 9, 5};
    int wide_x[3] = {1, 10, 17};
    int wide_w[3] = {8, 6, 6};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, mortar);
    for (int row = 0; row < 3; row++) {
        int *stone_x = ((tile_x + tile_y + row) % 2 == 0)
            ? narrow_x : wide_x;
        int *stone_w = ((tile_x + tile_y + row) % 2 == 0)
            ? narrow_w : wide_w;
        int stone_y = 1 + row * 8;
        for (int column = 0; column < 3; column++) {
            int color = (tile_x * 3 + tile_y * 5 + row + column) % 3;
            int stone_height = row == 2 ? 6 : 7;
            fill_rect(r, x + stone_x[column], y + stone_y,
                stone_w[column], stone_height, stones[color]);
            fill_rect(r, x + stone_x[column] + 1, y + stone_y,
                stone_w[column] - 2, 1, highlight);
            fill_rect(r, x + stone_x[column] + 1,
                y + stone_y + stone_height - 1,
                stone_w[column] - 1, 1, shadow);
        }
    }
}

void draw_town_gate(Renderer *r, int tx, int ty, Location town, int dx, int dy) {
    if (!r->town_gate_texture) {
        return;
    }
    int facing = dx < 0 ? 1 : dx > 0 ? 3 : dy < 0 ? 2 : 0;
    int row = town_road_index(town);
    if (row < 0) {
        row = 5; // Castle Grounds uses the same stonework with crimson banners.
    }
    const int anchor_x[] = {84, 168, 84, 0};
    const int anchor_y[] = {0, 80, 168, 80};
    SDL_Rect source = {facing * 192, row * 192, 192, 192};
    SDL_Rect destination = {tx * TILE_SIZE - anchor_x[facing], ty * TILE_SIZE - anchor_y[facing], 192, 192};
    SDL_RenderCopy(r->sdl, r->town_gate_texture, &source, &destination);
}

void draw_town_road_sign(Renderer *r, int tile_x, int tile_y, const char *name) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int text_w = 0;
    int text_h = 0;
    const char *second = strchr(name, '\n');
    const char *first = name;
    char line[64];
    int second_w = 0;
    if (second) {
        size_t length = (size_t)(second - name);
        if (length >= sizeof(line)) {
            length = sizeof(line) - 1;
        }
        memcpy(line, name, length);
        line[length] = '\0';
        first = line;
        second++;
        TTF_SizeText(r->font_tiny, second, &second_w, NULL);
    }
    TTF_SizeText(r->font_tiny, first, &text_w, &text_h);
    int widest = text_w > second_w ? text_w : second_w;
    int height = second ? 32 : 18;
    int width = widest + 12;
    if (width < 66) {
        width = 66;
    }
    int post = width - 10;
    SDL_Color shadow = {37, 29, 24, 255};
    SDL_Color wood = {93, 62, 37, 255};
    SDL_Color bark = {154, 109, 59, 255};
    SDL_Color brass = {215, 178, 97, 255};
    fill_rect(r, x + post, y + 16, 9, 47, shadow);
    fill_rect(r, x + post + 2, y + 18, 5, 44, wood);
    fill_rect(r, x + post + 2, y + 20, 2, 40, bark);
    fill_rect(r, x + post - 3, y + 60, 15, 4, shadow);
    fill_rect(r, x + 4, y + 2, width + 6, height + 6, shadow);
    fill_rect(r, x + 7, y + 5, width, height, wood);
    fill_rect(r, x + 7, y + 5, width, 2, bark);
    fill_rect(r, x + 10, y + 9, 2, 2, brass);
    fill_rect(r, x + width + 2, y + 9, 2, 2, brass);
    int text_y = second ? y + 9 : y + 5 + (height - text_h) / 2;
    renderer_draw_text(r, first, x + 7 + (width - text_w) / 2,
        text_y, (SDL_Color){233, 201, 133, 255}, r->font_tiny);
    if (second) {
        renderer_draw_text(r, second, x + 7 + (width - second_w) / 2,
            text_y + text_h + 4, (SDL_Color){233, 201, 133, 255}, r->font_tiny);
    }
}

static void draw_dungeon_door_panel(Renderer *r, int x, int width, int right) {
    SDL_Rect previous_clip;
    SDL_bool had_clip = SDL_RenderIsClipEnabled(r->sdl);
    if (had_clip) {
        SDL_RenderGetClipRect(r->sdl, &previous_clip);
    }
    SDL_Rect clip = {x, 0, width, r->screen_h};
    SDL_RenderSetClipRect(r->sdl, &clip);
    fill_rect(r, x, 0, width, r->screen_h,
        (SDL_Color){21, 20, 34, 255});
    for (int row = 0; row * 24 < r->screen_h; row++) {
        int y = row * 24;
        int offset = row % 2 == 0 ? 0 : 24;
        for (int column = -offset; column < width; column += 48) {
            SDL_Color stone = ((column / 48 + row) & 1) == 0 ?
                (SDL_Color){53, 49, 68, 255} :
                (SDL_Color){61, 55, 76, 255};
            fill_rect(r, x + column + 1, y + 2, 46, 20, stone);
            fill_rect(r, x + column + 3, y + 3, 42, 2,
                (SDL_Color){81, 73, 98, 255});
            fill_rect(r, x + column + 3, y + 20, 42, 2,
                (SDL_Color){34, 31, 49, 255});
        }
    }
    int band_x = right ? x : x + width - 8;
    fill_rect(r, band_x, 0, 8, r->screen_h,
        (SDL_Color){24, 27, 39, 255});
    fill_rect(r, band_x + 2, 0, 2, r->screen_h,
        (SDL_Color){75, 78, 87, 255});
    for (int y = 18; y < r->screen_h; y += 48) {
        fill_rect(r, band_x + 3, y, 3, 4,
            (SDL_Color){155, 124, 65, 255});
    }
    int plate_x = right ? x + 8 : x + width - 24;
    int plate_y = r->screen_h / 2 - 18;
    fill_rect(r, plate_x, plate_y, 16, 36,
        (SDL_Color){28, 30, 43, 255});
    fill_rect(r, plate_x + 2, plate_y + 2, 12, 32,
        (SDL_Color){79, 80, 89, 255});
    fill_rect(r, plate_x + 5, plate_y + 11, 6, 14,
        (SDL_Color){159, 127, 66, 255});
    fill_rect(r, plate_x + 7, plate_y + 14, 2, 8,
        (SDL_Color){50, 43, 45, 255});
    SDL_RenderSetClipRect(r->sdl, had_clip ? &previous_clip : NULL);
}

void draw_dungeon_transition(Renderer *r, int covered_width) {
    if (covered_width <= 0) {
        return;
    }
    int max_width = (r->screen_w + 1) / 2;
    if (covered_width > max_width) {
        covered_width = max_width;
    }
    draw_dungeon_door_panel(r, 0, covered_width, 0);
    draw_dungeon_door_panel(r, r->screen_w - covered_width,
        covered_width, 1);
}

static void draw_forest_canopy_panel(Renderer *r, int x, int width, int right) {
    SDL_Rect previous_clip;
    SDL_bool had_clip = SDL_RenderIsClipEnabled(r->sdl);
    if (had_clip) {
        SDL_RenderGetClipRect(r->sdl, &previous_clip);
    }
    SDL_Rect clip = {x, 0, width, r->screen_h};
    SDL_RenderSetClipRect(r->sdl, &clip);
    fill_rect(r, x, 0, width, r->screen_h,
        (SDL_Color){9, 29, 21, 255});

    for (int trunk = 44; trunk < width + 60; trunk += 128) {
        fill_rect(r, x + trunk, 0, 13, r->screen_h,
            (SDL_Color){37, 38, 27, 255});
        fill_rect(r, x + trunk + 3, 0, 3, r->screen_h,
            (SDL_Color){72, 63, 39, 255});
        for (int y = 32; y < r->screen_h; y += 96) {
            fill_rect(r, x + trunk - 23, y, 28, 5,
                (SDL_Color){52, 51, 31, 255});
            fill_rect(r, x + trunk + 9, y + 44, 34, 5,
                (SDL_Color){52, 51, 31, 255});
        }
    }

    SDL_Color leaves[3] = {
        {24, 66, 37, 255},
        {30, 81, 42, 255},
        {39, 96, 49, 255}
    };
    for (int row = 0; row * 22 < r->screen_h; row++) {
        int y = row * 22;
        int offset = row % 2 == 0 ? 0 : 14;
        for (int column = -offset; column < width; column += 28) {
            int index = (row * 7 + (column + offset) / 28) % 3;
            int leaf_x = x + column;
            fill_rect(r, leaf_x + 3, y + 4, 22, 15, leaves[index]);
            fill_rect(r, leaf_x + 7, y + 1, 14, 21, leaves[index]);
            fill_rect(r, leaf_x + 9, y + 5, 8, 2,
                (SDL_Color){65, 119, 59, 255});
            fill_rect(r, leaf_x + 5, y + 16, 5, 2,
                (SDL_Color){16, 48, 29, 255});
        }
    }

    int edge_x = right ? x : x + width - 7;
    fill_rect(r, edge_x, 0, 7, r->screen_h,
        (SDL_Color){16, 46, 29, 255});
    for (int y = 8; y < r->screen_h; y += 34) {
        fill_rect(r, edge_x + 1, y, 6, 11,
            (SDL_Color){40, 94, 47, 255});
        fill_rect(r, edge_x + 3, y + 3, 3, 3,
            (SDL_Color){79, 132, 66, 255});
    }
    SDL_RenderSetClipRect(r->sdl, had_clip ? &previous_clip : NULL);
}

void draw_forest_transition(Renderer *r, int covered_width) {
    if (covered_width <= 0) {
        return;
    }
    int max_width = (r->screen_w + 1) / 2;
    if (covered_width > max_width) {
        covered_width = max_width;
    }
    draw_forest_canopy_panel(r, 0, covered_width, 0);
    draw_forest_canopy_panel(r, r->screen_w - covered_width,
        covered_width, 1);
}

static void draw_mountain_cliff_panel(Renderer *r, int x, int width, int right) {
    SDL_Rect previous_clip;
    SDL_bool had_clip = SDL_RenderIsClipEnabled(r->sdl);
    if (had_clip) {
        SDL_RenderGetClipRect(r->sdl, &previous_clip);
    }
    SDL_Rect clip = {x, 0, width, r->screen_h};
    SDL_RenderSetClipRect(r->sdl, &clip);
    fill_rect(r, x, 0, width, r->screen_h,
        (SDL_Color){27, 25, 32, 255});

    SDL_Color rock[3] = {
        {52, 46, 50, 255},
        {64, 51, 50, 255},
        {45, 42, 49, 255}
    };
    SDL_Color ridge = {93, 75, 69, 255};
    SDL_Color shadow = {20, 19, 26, 255};
    for (int row = 0; row * 40 < r->screen_h; row++) {
        int y = row * 40;
        int offset = row % 2 == 0 ? 0 : 22;
        for (int column = -offset; column < width; column += 44) {
            int piece = (column + offset) / 44;
            int shift = (row * 17 + piece * 11) % 13;
            int rock_x = x + column + shift - 6;
            int index = (row * 7 + piece) % 3;
            fill_rect(r, rock_x, y + 2, 48, 40, rock[index]);
            fill_rect(r, rock_x + 7, y + 1, 28, 8, rock[index]);
            fill_rect(r, rock_x + 8, y + 5, 23, 2, ridge);
            fill_rect(r, rock_x + 4, y + 10, 2, 17,
                (SDL_Color){74, 62, 62, 255});
            fill_rect(r, rock_x + 33, y + 25, 14, 10,
                (SDL_Color){38, 35, 41, 255});
            if ((row + piece) % 3 == 0) {
                fill_rect(r, rock_x + 22, y + 16, 2, 13, shadow);
                fill_rect(r, rock_x + 17, y + 27, 7, 2, shadow);
            }
            if ((row + piece) % 5 == 1) {
                fill_rect(r, rock_x + 31, y + 11, 8, 2,
                    (SDL_Color){111, 87, 74, 255});
            }
        }
    }

    int edge_x = right ? x : x + width - 9;
    fill_rect(r, edge_x, 0, 9, r->screen_h, shadow);
    fill_rect(r, edge_x + (right ? 6 : 1), 0, 2, r->screen_h, ridge);
    for (int y = 12; y < r->screen_h; y += 58) {
        fill_rect(r, edge_x + 3, y, 3, 12,
            (SDL_Color){112, 62, 49, 255});
        fill_rect(r, edge_x + 3, y + 3, 2, 5,
            (SDL_Color){185, 92, 57, 255});
    }
    SDL_RenderSetClipRect(r->sdl, had_clip ? &previous_clip : NULL);
    for (int y = 4; y < r->screen_h; y += 32) {
        int ledge = 5 + ((y / 32) * 7) % 10;
        int ledge_x = right ? x - ledge : x + width;
        fill_rect(r, ledge_x, y + 5, ledge, 12,
            rock[(y / 32) % 3]);
        fill_rect(r, ledge_x + (right ? 3 : 0), y + 2, ledge - 3, 3,
            ridge);
    }
}

void draw_mountain_transition(Renderer *r, int covered_width) {
    if (covered_width <= 0) {
        return;
    }
    int max_width = (r->screen_w + 1) / 2;
    if (covered_width > max_width) {
        covered_width = max_width;
    }
    draw_mountain_cliff_panel(r, 0, covered_width, 0);
    draw_mountain_cliff_panel(r, r->screen_w - covered_width,
        covered_width, 1);
}

void draw_coast_transition(Renderer *r, int covered_width) {
    if (covered_width <= 0) {
        return;
    }
    int max_width = (r->screen_w + 1) / 2;
    if (covered_width > max_width) {
        covered_width = max_width;
    }
    int height = (int)((Uint64)r->screen_h * covered_width / max_width);
    int water_y = r->screen_h - height;
    SDL_Color deep = {6, 38, 60, 255};
    SDL_Color swell = {14, 65, 82, 255};
    SDL_Color ripple = {37, 108, 125, 255};
    SDL_Color glint = {83, 165, 161, 255};
    SDL_Color foam = {135, 201, 188, 255};
    fill_rect(r, 0, water_y, r->screen_w, height, deep);
    for (int row = 0; row * 42 < height; row++) {
        int wave_y = water_y + row * 42;
        int offset = row % 2 == 0 ? 0 : 38;
        for (int x = -offset; x < r->screen_w; x += 76) {
            fill_rect(r, x + 3, wave_y + 18, 49, 7, swell);
            fill_rect(r, x + 10, wave_y + 15, 33, 3, ripple);
            fill_rect(r, x + 17, wave_y + 13, 18, 2, glint);
            fill_rect(r, x + 45, wave_y + 30, 21, 2, ripple);
        }
    }

    fill_rect(r, 0, water_y, r->screen_w, 5, glint);
    for (int x = 0; x < r->screen_w; x += 48) {
        int crest = 3 + ((x / 48) * 7) % 7;
        fill_rect(r, x + 5, water_y - crest, 28, crest + 4, foam);
        fill_rect(r, x + 11, water_y + 4, 30, 3, ripple);
    }
}

static void draw_transition_terrain(Renderer *r, const Map *map, int tx, int ty, int mx, int my, TownExitStyle style) {
    TileType tile = map->tiles[my][mx];
    switch (style) {
        case TOWN_EXIT_FROST:
            if (tile == TILE_FROST_WALL) {
                draw_frostfell_wall(r, map, tx, ty, mx, my);
            } else if (tile == TILE_FROST_ICE) {
                draw_frostfell_ice(r, tx, ty, mx, my);
            } else {
                draw_frostfell_floor(r, tx, ty, mx, my);
            }
            break;
        case TOWN_EXIT_DESERT:
            if (desert_rock(tile)) {
                draw_desert_wall(r, map, tx, ty, mx, my);
            } else {
                draw_desert_floor(r, tx, ty, mx, my);
            }
            break;
        case TOWN_EXIT_ASHEN:
            if (tile == TILE_ASHEN_WALL) {
                draw_ashen_wall(r, tx, ty, mx, my);
            } else if (tile == TILE_ASHEN_LAVA) {
                draw_ashen_lava(r, tx, ty, mx, my);
            } else {
                draw_ashen_floor(r, tx, ty, mx, my);
            }
            break;
        case TOWN_EXIT_GLASSDEEP:
            if (tile == TILE_GLASSDEEP_WALL) {
                draw_glassdeep_wall(r, tx, ty, mx, my);
            } else if (tile == TILE_GLASSDEEP_POOL) {
                draw_glassdeep_pool(r, tx, ty, mx, my);
            } else {
                draw_glassdeep_floor(r, tx, ty, mx, my);
            }
            break;
        default:
            break;
    }
}

void draw_area_transition(Renderer *r, int covered_width, TownExitStyle style) {
    switch (style) {
        case TOWN_EXIT_FOREST:
        case TOWN_EXIT_ROAD:
        case TOWN_EXIT_SWAMP:
            draw_forest_transition(r, covered_width);
            return;
        case TOWN_EXIT_MOUNTAINS:
        case TOWN_EXIT_DRAGONSPINE:
            draw_mountain_transition(r, covered_width);
            return;
        case TOWN_EXIT_COAST:
            draw_coast_transition(r, covered_width);
            return;
        case TOWN_EXIT_FROST:
        case TOWN_EXIT_DESERT:
        case TOWN_EXIT_ASHEN:
        case TOWN_EXIT_GLASSDEEP:
            break;
        default:
            draw_dungeon_transition(r, covered_width);
            return;
    }
    if (covered_width <= 0) {
        return;
    }
    int max_width = (r->screen_w + 1) / 2;
    if (covered_width > max_width) {
        covered_width = max_width;
    }
    int columns = (max_width + TILE_SIZE - 1) / TILE_SIZE;
    int rows = (r->screen_h + TILE_SIZE - 1) / TILE_SIZE;
    static const TileType walls[] = {TILE_FROST_WALL, TILE_DESERT_WALL, TILE_ASHEN_WALL, TILE_GLASSDEEP_WALL};
    static const TileType floors[] = {TILE_FROST_FLOOR, TILE_DESERT_FLOOR, TILE_ASHEN_FLOOR, TILE_GLASSDEEP_FLOOR};
    static const TileType pools[] = {TILE_FROST_ICE, TILE_DESERT_FLOOR, TILE_ASHEN_LAVA, TILE_GLASSDEEP_POOL};
    int biome = style - TOWN_EXIT_FROST;
    // A decorative map lets connected snow and sandstone reuse their normal
    // terrain shading. It never touches the player's map or random sequence.
    Map terrain = {0};
    for (int my = 0; my < MAP_H && my < rows + 2; my++) {
        for (int mx = 0; mx < MAP_W && mx < columns * 2 + 2; mx++) {
            int shade = dragonspine_shade(mx * TILE_SIZE, my * TILE_SIZE, 108, 84);
            terrain.tiles[my][mx] = shade > 122 ? walls[biome] : shade < 72 ? pools[biome] : floors[biome];
        }
    }
    SDL_Rect previous_view;
    SDL_Rect previous_clip;
    SDL_bool had_clip = SDL_RenderIsClipEnabled(r->sdl);
    SDL_RenderGetViewport(r->sdl, &previous_view);
    SDL_Rect bounds = previous_view;
    if (had_clip) {
        SDL_RenderGetClipRect(r->sdl, &previous_clip);
        bounds = (SDL_Rect){previous_view.x + previous_clip.x, previous_view.y + previous_clip.y, previous_clip.w, previous_clip.h};
    }
    for (int side = 0; side < 2; side++) {
        int x = side ? r->screen_w - covered_width : covered_width - max_width;
        SDL_Rect view = {x, 0, max_width, r->screen_h};
        SDL_RenderSetViewport(r->sdl, &view);
        SDL_Rect local_bounds = {bounds.x - x, bounds.y, bounds.w, bounds.h};
        for (int row = 0; row < rows; row++) {
            int inset = dragonspine_shade(side * 96, row * TILE_SIZE, 36, 72) / 20;
            if (inset > max_width - covered_width) {
                inset = max_width - covered_width;
            }
            if (inset >= covered_width) {
                continue;
            }
            SDL_Rect edge = {side ? inset : max_width - covered_width, row * TILE_SIZE, covered_width - inset, TILE_SIZE};
            SDL_Rect clip;
            if (!SDL_IntersectRect(&edge, &local_bounds, &clip)) {
                continue;
            }
            SDL_RenderSetClipRect(r->sdl, &clip);
            for (int col = clip.x / TILE_SIZE; col * TILE_SIZE < clip.x + clip.w; col++) {
                int mx = (col + side * columns) % (MAP_W - 2) + 1;
                int my = row % (MAP_H - 2) + 1;
                draw_transition_terrain(r, &terrain, col, row, mx, my, style);
            }
        }
    }
    SDL_RenderSetViewport(r->sdl, &previous_view);
    SDL_RenderSetClipRect(r->sdl, had_clip ? &previous_clip : NULL);
}

static void draw_shop_blacksmith_fallback(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = { 18,  14,  14, 255};
    SDL_Color stone   = { 72,  70,  68, 255};
    SDL_Color mortar  = { 42,  40,  40, 255};
    SDL_Color timber  = { 82,  48,  28, 255};
    SDL_Color wood_hi = {116,  70,  36, 255};
    SDL_Color roof    = { 42,  42,  48, 255};
    SDL_Color roof_hi = { 68,  66,  70, 255};
    SDL_Color brick   = {104,  48,  34, 255};
    SDL_Color ember   = {255, 112,  16, 255};
    SDL_Color flame   = {255, 202,  54, 255};
    SDL_Color iron    = {132, 138, 142, 255};

    /* Foundation and timber facade. */
    fill_rect(r, x+4, y+78, 112, 16, outline);
    fill_rect(r, x+6, y+80, 108, 12, stone);
    for (int sx = 8; sx < 112; sx += 18)
        fill_rect(r, x+sx, y+86, 1, 6, mortar);
    fill_rect(r, x+8, y+55, 104, 27, timber);
    fill_rect(r, x+8, y+55, 104, 4, wood_hi);
    fill_rect(r, x+12, y+56, 5, 26, outline);
    fill_rect(r, x+103, y+56, 5, 26, outline);

    /* Broad charcoal roof with staggered shingle highlights. */
    fill_rect(r, x+5,  y+25, 110, 34, outline);
    fill_rect(r, x+9,  y+22, 102, 35, roof);
    fill_rect(r, x+13, y+18,  94,  7, roof_hi);
    for (int row = 0; row < 3; row++) {
        int sy = y + 28 + row * 9;
        int offset = (row & 1) ? 8 : 0;
        fill_rect(r, x+10, sy+6, 100, 2, outline);
        for (int sx = 14 + offset; sx < 108; sx += 16)
            fill_rect(r, x+sx, sy, 2, 7, outline);
    }

    /* Sooted brick chimney and live forge. */
    fill_rect(r, x+10, y+4, 24, 31, outline);
    fill_rect(r, x+13, y+7, 18, 27, brick);
    fill_rect(r, x+13, y+15, 18, 2, outline);
    fill_rect(r, x+20, y+7,  2, 27, outline);
    fill_rect(r, x+7,  y+59, 39, 25, outline);
    fill_rect(r, x+11, y+63, 31, 18, (SDL_Color){56, 24, 16, 255});
    fill_rect(r, x+16, y+73, 21,  7, ember);
    fill_rect(r, x+21, y+67, 11, 12, flame);

    /* Workshop door, iron braces, woodpile, and anvil sign. */
    fill_rect(r, x+52, y+60, 29, 33, outline);
    fill_rect(r, x+56, y+64, 21, 29, (SDL_Color){58, 34, 22, 255});
    fill_rect(r, x+59, y+69, 15, 2, wood_hi);
    fill_rect(r, x+59, y+80, 15, 2, wood_hi);
    fill_rect(r, x+62, y+65, 2, 26, iron);
    fill_rect(r, x+72, y+77, 3, 3, iron);
    fill_rect(r, x+87, y+70, 18, 4, iron);
    fill_rect(r, x+91, y+74, 10, 3, iron);
    fill_rect(r, x+94, y+77,  4, 4, outline);
    for (int i = 0; i < 3; i++) {
        fill_rect(r, x+85+i*9, y+84, 8, 7, (SDL_Color){92, 52, 26, 255});
        fill_rect(r, x+87+i*9, y+84, 4, 2, wood_hi);
    }
}

void draw_shop_blacksmith(Renderer *r, int tile_x, int tile_y) {
    if (!r->blacksmith_texture) {
        draw_shop_blacksmith_fallback(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        5 * TILE_SIZE, 4 * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->blacksmith_texture, NULL, &destination);
}

static void draw_shop_alchemist_fallback(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = { 14,  14,  24, 255};
    SDL_Color stone   = { 66,  70,  76, 255};
    SDL_Color wall    = { 48,  38,  66, 255};
    SDL_Color timber  = { 28,  24,  42, 255};
    SDL_Color roof    = { 24,  72,  70, 255};
    SDL_Color roof_hi = { 42, 104,  94, 255};
    SDL_Color purple  = {176,  68, 220, 255};
    SDL_Color violet  = {108,  62, 188, 255};
    SDL_Color green   = {112, 214,  92, 255};
    SDL_Color glass   = {202, 228, 210, 255};
    SDL_Color herb    = { 76, 116,  48, 255};

    /* Stone base and crooked indigo timber facade. */
    fill_rect(r, x+5, y+79, 110, 15, outline);
    fill_rect(r, x+8, y+81, 104, 11, stone);
    fill_rect(r, x+9, y+52, 102, 31, wall);
    fill_rect(r, x+13, y+53, 5, 30, timber);
    fill_rect(r, x+102,y+53, 5, 30, timber);
    fill_rect(r, x+14, y+58, 92, 4, timber);

    /* Steep teal roof and central gable. */
    fill_rect(r, x+5,  y+27, 110, 31, outline);
    fill_rect(r, x+9,  y+24, 102, 31, roof);
    for (int row = 0; row < 3; row++) {
        int sy = y + 28 + row * 8;
        int offset = (row & 1) ? 7 : 0;
        fill_rect(r, x+10, sy+6, 100, 2, outline);
        for (int sx = 13 + offset; sx < 109; sx += 14)
            fill_rect(r, x+sx, sy, 2, 7, roof_hi);
    }
    fill_rect(r, x+44, y+22, 32, 36, outline);
    fill_rect(r, x+48, y+25, 24, 33, (SDL_Color){42, 32, 58, 255});
    fill_rect(r, x+56, y+17,  8,  9, roof_hi);

    /* Bottle-shaped glowing gable window. */
    fill_rect(r, x+56, y+31, 8, 4, glass);
    fill_rect(r, x+53, y+35,14,14, glass);
    fill_rect(r, x+55, y+37,10,10, violet);
    fill_rect(r, x+58, y+39, 6, 6, purple);
    fill_rect(r, x+61, y+39, 2, 2, (SDL_Color){244, 184, 255, 255});

    /* South door, luminous window, herbs, and potion display. */
    fill_rect(r, x+48, y+59, 27, 34, outline);
    fill_rect(r, x+52, y+63, 19, 30, (SDL_Color){54, 34, 30, 255});
    fill_rect(r, x+55, y+70, 13, 2, (SDL_Color){102, 66, 42, 255});
    fill_rect(r, x+55, y+81, 13, 2, (SDL_Color){102, 66, 42, 255});
    fill_rect(r, x+66, y+77, 3, 3, glass);
    fill_rect(r, x+18, y+63, 22, 18, outline);
    fill_rect(r, x+21, y+66, 16, 12, green);
    fill_rect(r, x+28, y+66, 2, 12, outline);
    fill_rect(r, x+21, y+72, 16, 2, outline);
    for (int i = 0; i < 3; i++) {
        fill_rect(r, x+78+i*7, y+68, 4, 6, herb);
        fill_rect(r, x+79+i*7, y+74, 2, 7, herb);
    }
    fill_rect(r, x+80, y+83, 27, 3, timber);
    fill_rect(r, x+83, y+78, 5, 5, glass);
    fill_rect(r, x+84, y+81, 3, 4, purple);
    fill_rect(r, x+92, y+76, 5, 7, glass);
    fill_rect(r, x+93, y+80, 3, 5, green);
    fill_rect(r, x+101,y+79, 5, 6, glass);
    fill_rect(r, x+102,y+82, 3, 3, violet);
}

void draw_shop_alchemist(Renderer *r, int tile_x, int tile_y) {
    if (!r->alchemist_texture) {
        draw_shop_alchemist_fallback(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        5 * TILE_SIZE, 4 * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->alchemist_texture, NULL, &destination);
}

static void draw_tavern_fallback(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = {24, 16, 14, 255};
    SDL_Color plaster = {128, 91, 58, 255};
    SDL_Color plaster_hi = {158, 112, 67, 255};
    SDL_Color timber = {66, 38, 24, 255};
    SDL_Color timber_hi = {103, 58, 30, 255};
    SDL_Color roof = {52, 33, 35, 255};
    SDL_Color roof_hi = {82, 48, 43, 255};
    SDL_Color window = {255, 177, 54, 255};
    SDL_Color glow = {255, 220, 105, 255};
    SDL_Color stone = {76, 70, 67, 255};

    // Stone foundation and broad half-timbered facade.
    fill_rect(r, x + 4, y + 101, 160, 15, outline);
    fill_rect(r, x + 7, y + 104, 154, 10, stone);
    fill_rect(r, x + 9, y + 55, 150, 51, outline);
    fill_rect(r, x + 13, y + 59, 142, 43, plaster);
    fill_rect(r, x + 13, y + 59, 142, 5, plaster_hi);
    fill_rect(r, x + 13, y + 78, 142, 5, timber);
    for (int beam_x = 16; beam_x < 154; beam_x += 28) {
        fill_rect(r, x + beam_x, y + 59, 5, 45, timber);
    }

    // Deep shingled roof and off-center chimney.
    fill_rect(r, x + 2, y + 29, 164, 34, outline);
    fill_rect(r, x + 7, y + 25, 154, 34, roof);
    fill_rect(r, x + 14, y + 20, 140, 8, roof_hi);
    for (int row = 0; row < 3; row++) {
        int roof_y = y + 31 + row * 9;
        int offset = row % 2 == 0 ? 0 : 10;
        fill_rect(r, x + 9, roof_y + 6, 150, 2, outline);
        for (int shingle_x = 14 + offset; shingle_x < 156;
            shingle_x += 20) {
            fill_rect(r, x + shingle_x, roof_y, 2, 7, outline);
        }
    }
    fill_rect(r, x + 29, y + 5, 22, 31, outline);
    fill_rect(r, x + 33, y + 8, 14, 27, (SDL_Color){91, 55, 43, 255});
    fill_rect(r, x + 30, y + 4, 20, 6, stone);

    // Warm leaded windows.
    for (int window_x = 28; window_x <= 124; window_x += 96) {
        fill_rect(r, x + window_x - 4, y + 68, 30, 25, outline);
        fill_rect(r, x + window_x, y + 72, 22, 17, window);
        fill_rect(r, x + window_x + 10, y + 72, 3, 17, timber);
        fill_rect(r, x + window_x, y + 79, 22, 3, timber);
        fill_rect(r, x + window_x + 3, y + 74, 5, 4, glow);
    }

    // Elowen waits inside the left window in a pale hood and blue robes.
    fill_rect(r, x + 32, y + 72, 12, 5, (SDL_Color){224, 216, 190, 255});
    fill_rect(r, x + 34, y + 76, 8, 5, (SDL_Color){201, 154, 119, 255});
    fill_rect(r, x + 31, y + 81, 14, 8, (SDL_Color){52, 81, 135, 255});
    fill_rect(r, x + 36, y + 78, 2, 2, (SDL_Color){45, 38, 55, 255});
    fill_rect(r, x + 40, y + 78, 2, 2, (SDL_Color){45, 38, 55, 255});

    // Central oak door with an iron handle and lit transom.
    fill_rect(r, x + 68, y + 65, 34, 49, outline);
    fill_rect(r, x + 73, y + 70, 24, 44, timber);
    fill_rect(r, x + 77, y + 74, 16, 8, window);
    fill_rect(r, x + 77, y + 85, 16, 3, timber_hi);
    fill_rect(r, x + 77, y + 96, 16, 3, timber_hi);
    fill_rect(r, x + 91, y + 91, 3, 3, glow);

    // Hanging tavern sign with a simple golden mug emblem.
    fill_rect(r, x + 135, y + 45, 4, 24, timber);
    fill_rect(r, x + 126, y + 46, 13, 4, timber);
    fill_rect(r, x + 114, y + 49, 24, 20, outline);
    fill_rect(r, x + 117, y + 52, 18, 14, timber_hi);
    fill_rect(r, x + 121, y + 55, 8, 8, glow);
    fill_rect(r, x + 129, y + 56, 4, 6, glow);
    fill_rect(r, x + 123, y + 53, 4, 3, window);
}

void draw_apothecary(Renderer *r, int tile_x, int tile_y) {
    if (!r->apothecary_texture) {
        draw_shop_alchemist(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TOWN_APOTHECARY_W * TILE_SIZE, TOWN_APOTHECARY_H * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->apothecary_texture, NULL, &destination);
}

void draw_adventurers_guild(Renderer *r, int tile_x, int tile_y) {
    if (!r->guild_texture) {
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TOWN_GUILD_W * TILE_SIZE, TOWN_GUILD_H * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->guild_texture, NULL, &destination);
}

void draw_workshop(Renderer *r, int tile_x, int tile_y) {
    if (!r->workshop_texture) {
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TOWN4_WORKSHOP_W * TILE_SIZE, TOWN4_WORKSHOP_H * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->workshop_texture, NULL, &destination);
}

void draw_town_hall(Renderer *r, int tile_x, int tile_y) {
    if (!r->town_hall_texture) {
        draw_tavern(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TOWN4_HALL_W * TILE_SIZE, TOWN4_HALL_H * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->town_hall_texture, NULL, &destination);
}

void draw_tavern(Renderer *r, int tile_x, int tile_y) {
    if (!r->tavern_texture) {
        draw_tavern_fallback(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        7 * TILE_SIZE, 5 * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->tavern_texture, NULL, &destination);
}

void draw_inn(Renderer *r, int tile_x, int tile_y) {
    if (!r->inn_texture) {
        draw_tavern(r, tile_x, tile_y);
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        7 * TILE_SIZE, 5 * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->inn_texture, NULL, &destination);
}

void draw_harbor(Renderer *r, int tile_x, int tile_y) {
    if (!r->harbor_texture) {
        return;
    }
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE,
        TOWN_HARBOR_W * TILE_SIZE, TOWN_HARBOR_H * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->harbor_texture, NULL, &destination);
}

static void draw_island_crop(Renderer *r, SDL_Rect source, SDL_Rect destination) {
    if (!r->island_texture) {
        return;
    }
    SDL_RenderCopy(r->sdl, r->island_texture, &source, &destination);
}

static void draw_island_tile(Renderer *r, int tile_x, int tile_y, SDL_Color base, SDL_Rect source) {
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, TILE_SIZE + 1, TILE_SIZE + 1
    };
    SDL_SetRenderDrawColor(r->sdl, base.r, base.g, base.b, base.a);
    SDL_RenderFillRect(r->sdl, &destination);
    draw_island_crop(r, source, destination);
}

void draw_island_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int frame = ((int)(SDL_GetTicks() / 500) + map_x + map_y) % 3;
    static const SDL_Rect sources[3] = {
        {18, 18, 110, 110}, {24, 22, 104, 104}, {16, 28, 112, 100}
    };
    draw_island_tile(r, tile_x, tile_y, (SDL_Color){8, 91, 113, 255},
        sources[frame]);
}

void draw_island_sand(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    SDL_Rect source = (map_x + map_y) % 3 == 0
        ? (SDL_Rect){635, 155, 85, 62}
        : (SDL_Rect){520, 155, 90, 62};
    draw_island_tile(r, tile_x, tile_y, (SDL_Color){221, 190, 122, 255}, source);
}

void draw_island_grass(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    static const SDL_Rect sources[3] = {
        {20, 238, 105, 90}, {150, 238, 105, 90}, {285, 238, 105, 90}
    };
    draw_island_tile(r, tile_x, tile_y, (SDL_Color){37, 94, 43, 255},
        sources[(map_x * 3 + map_y) % 3]);
}

void draw_island_jungle(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    static const SDL_Rect sources[3] = {
        {720, 345, 135, 130}, {860, 345, 135, 130}, {1000, 345, 125, 130}
    };
    draw_island_tile(r, tile_x, tile_y, (SDL_Color){18, 65, 31, 255},
        sources[(map_x + map_y * 2) % 3]);
}

void draw_island_path(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    SDL_Rect source = (map_x + map_y) % 2 == 0
        ? (SDL_Rect){745, 238, 105, 90}
        : (SDL_Rect){880, 238, 105, 90};
    draw_island_tile(r, tile_x, tile_y, (SDL_Color){74, 91, 58, 255}, source);
}

void draw_island_temple(Renderer *r, int tile_x, int tile_y) {
    if (!r->island_temple_texture) {
        return;
    }
    SDL_Rect destination = {tile_x * TILE_SIZE, tile_y * TILE_SIZE, ISLAND_TEMPLE_W * TILE_SIZE, ISLAND_TEMPLE_H * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->island_temple_texture, NULL, &destination);
}

void draw_island_camp(Renderer *r, int tile_x, int tile_y) {
    SDL_Rect source = {360, 730, 380, 230};
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, 12 * TILE_SIZE, 7 * TILE_SIZE
    };
    draw_island_crop(r, source, destination);
}

void draw_island_marker(Renderer *r, int tile_x, int tile_y) {
    SDL_Rect source = {730, 775, 150, 185};
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, 5 * TILE_SIZE, 5 * TILE_SIZE
    };
    draw_island_crop(r, source, destination);
}

void draw_island_statue(Renderer *r, int tile_x, int tile_y) {
    SDL_Rect source = {860, 765, 160, 195};
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, 5 * TILE_SIZE, 6 * TILE_SIZE
    };
    draw_island_crop(r, source, destination);
}

void draw_island_lagoon(Renderer *r, int tile_x, int tile_y) {
    SDL_Rect source = {710, 585, 504, 220};
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, 13 * TILE_SIZE, 7 * TILE_SIZE
    };
    draw_island_crop(r, source, destination);
}

void draw_island_dock(Renderer *r, int tile_x, int tile_y) {
    SDL_Rect source = {0, 735, 365, 230};
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, 12 * TILE_SIZE, 7 * TILE_SIZE
    };
    draw_island_crop(r, source, destination);
}

void draw_island_ship(Renderer *r, int tile_x, int tile_y) {
    if (!r->island_ship_texture) {
        return;
    }
    SDL_Rect source = {330, 30, 800, 1230};
    SDL_Rect destination = {
        tile_x * TILE_SIZE, tile_y * TILE_SIZE, 14 * TILE_SIZE, 8 * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->island_ship_texture, &source, &destination);
}

void draw_floor_item(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_BAG, (SDL_Color){100, 180, 255, 255});
}

static void draw_labyrinth_entrance_fallback(Renderer *r, int tile_x, int tile_y, int open) {
    int x = (tile_x - 1) * TILE_SIZE;
    int y = (tile_y - 2) * TILE_SIZE;
    SDL_Color shadow = {25, 29, 25, 255};
    SDL_Color mortar = {48, 49, 40, 255};
    SDL_Color stones[3] = {
        {105, 109, 88, 255}, {123, 124, 100, 255}, {91, 98, 81, 255}
    };
    SDL_Color edge = {166, 161, 126, 255};
    SDL_Color worn = {133, 139, 104, 255};
    SDL_Color moss = {40, 74, 37, 255};
    SDL_Color leaf = {69, 109, 45, 255};
    SDL_Color iron = {50, 58, 53, 255};
    SDL_Color metal = {116, 126, 105, 255};
    SDL_Color brass = {168, 125, 57, 255};
    SDL_Color dark = {10, 17, 16, 255};

    // Weathered masonry, with staggered joints and chipped block faces.
    fill_rect(r, x + 3, y + 64, 67, 8, shadow);
    fill_rect(r, x + 7, y + 16, 58, 51, mortar);
    for (int row = 0; row < 6; row++) {
        for (int column = 0; column < 5; column++) {
            int left = 8 + column * 14 - (row % 2) * 7;
            int right = left + 13;
            if (left < 8) {
                left = 8;
            }
            if (right > 64) {
                right = 64;
            }
            if (left >= right) {
                continue;
            }
            int top = 18 + row * 8;
            fill_rect(r, x + left, y + top, right - left, 7,
                stones[(row + column * 2) % 3]);
            fill_rect(r, x + left + 1, y + top, right - left - 1, 1, edge);
            fill_rect(r, x + left, y + top + 1, 1, 4, worn);
            fill_rect(r, x + right - 1, y + top + 1, 1, 6, mortar);
            fill_rect(r, x + left + 1, y + top + 6, right - left - 1, 1, stones[2]);
            fill_rect(r, x + left + 2, y + top + 5, 2, 1, mortar);
        }
    }

    // Layered coping and an ivory rook crest above the arch.
    fill_rect(r, x + 10, y + 11, 52, 6, shadow);
    fill_rect(r, x + 12, y + 9, 48, 5, stones[2]);
    fill_rect(r, x + 13, y + 9, 46, 1, edge);
    fill_rect(r, x + 16, y + 6, 40, 3, stones[0]);
    fill_rect(r, x + 17, y + 6, 38, 1, worn);
    fill_rect(r, x + 28, y + 1, 16, 18, shadow);
    fill_rect(r, x + 29, y + 2, 14, 15, brass);
    fill_rect(r, x + 30, y + 3, 12, 13, mortar);
    fill_rect(r, x + 33, y + 7, 6, 6, edge);
    for (int tooth = 0; tooth < 3; tooth++) {
        fill_rect(r, x + 32 + tooth * 3, y + 5, 2, 4,
            (SDL_Color){225, 217, 173, 255});
    }
    fill_rect(r, x + 32, y + 12, 8, 2, edge);
    fill_rect(r, x + 31, y + 14, 10, 1, worn);

    // Stepped arch stones surround a deep, inset doorway.
    const int arch_widths[5] = {7, 11, 15, 18, 20};
    for (int row = 0; row < 5; row++) {
        int half = arch_widths[row];
        int top = 19 + row * 4;
        fill_rect(r, x + 36 - half, y + top, half * 2, 4, edge);
        fill_rect(r, x + 37 - half, y + top + 1, half * 2 - 2, 3, stones[1]);
        fill_rect(r, x + 40 - half, y + top + 3, half * 2 - 8, 4, dark);
    }
    fill_rect(r, x + 16, y + 39, 40, 27, stones[1]);
    fill_rect(r, x + 17, y + 39, 2, 27, edge);
    fill_rect(r, x + 53, y + 39, 3, 27, stones[2]);
    fill_rect(r, x + 20, y + 39, 32, 27, dark);
    for (int joint = 0; joint < 3; joint++) {
        fill_rect(r, x + 16, y + 44 + joint * 8, 4, 1, mortar);
        fill_rect(r, x + 52, y + 44 + joint * 8, 4, 1, mortar);
    }
    fill_rect(r, x + 34, y + 19, 4, 6, worn);
    fill_rect(r, x + 35, y + 20, 2, 3, edge);

    if (open) {
        // Receding side walls and steps make the open passage readable.
        fill_rect(r, x + 21, y + 40, 3, 24, mortar);
        fill_rect(r, x + 49, y + 40, 2, 24, shadow);
        for (int step = 0; step < 4; step++) {
            int inset = 6 - step * 2;
            fill_rect(r, x + 24 + inset, y + 52 + step * 4,
                24 - inset * 2, 3, stones[2]);
            fill_rect(r, x + 24 + inset, y + 52 + step * 4,
                24 - inset * 2, 1, worn);
        }
    } else {
        for (int bar = 0; bar < 5; bar++) {
            int top = bar == 0 || bar == 4 ? 36 : 29;
            int bx = x + 23 + bar * 6;
            fill_rect(r, bx, y + top, 3, 66 - top, iron);
            fill_rect(r, bx, y + top, 1, 65 - top, metal);
            fill_rect(r, bx + 1, y + 57, 1, 3, brass);
        }
        for (int brace = 0; brace < 2; brace++) {
            int by = y + 43 + brace * 13;
            fill_rect(r, x + 21, by, 30, 4, iron);
            fill_rect(r, x + 21, by, 30, 1, metal);
            for (int rivet = 0; rivet < 5; rivet++) {
                fill_rect(r, x + 23 + rivet * 6, by + 1, 1, 1, brass);
            }
        }
        fill_rect(r, x + 33, y + 47, 6, 7, brass);
        fill_rect(r, x + 35, y + 49, 2, 3, shadow);
    }

    // Buttresses carry iron sconces; ivy grows into their outer joints.
    for (int side = 0; side < 2; side++) {
        int px = x + 3 + side * 54;
        fill_rect(r, px, y + 26, 12, 41, shadow);
        for (int row = 0; row < 5; row++) {
            int py = y + 27 + row * 8;
            fill_rect(r, px + 1, py, 10, 7, stones[(row + side) % 3]);
            fill_rect(r, px + 1, py, 9, 1, edge);
            fill_rect(r, px + 1, py + 1, 1, 5, worn);
            fill_rect(r, px + 10, py + 1, 1, 6, mortar);
            fill_rect(r, px + 2, py + 6, 8, 1, stones[2]);
            fill_rect(r, px + 3 + row % 3, py + 2, 2, 1, worn);
            if ((row + side) % 3 == 0) {
                fill_rect(r, px + 7, py + 1, 1, 3, mortar);
                fill_rect(r, px + 6, py + 3, 1, 2, mortar);
            }
        }
        fill_rect(r, px - 1, y + 24, 14, 3, stones[1]);
        fill_rect(r, px, y + 24, 12, 1, edge);
        fill_rect(r, px + 4, y + 44, 4, 12, iron);
        fill_rect(r, px + 3, y + 47, 6, 3, brass);
        if (open) {
            fill_rect(r, px + 3, y + 40, 6, 7, (SDL_Color){141, 60, 28, 255});
            fill_rect(r, px + 4, y + 38, 4, 8, (SDL_Color){236, 140, 43, 255});
            fill_rect(r, px + 5, y + 41, 2, 5, (SDL_Color){255, 221, 120, 255});
            fill_rect(r, px + 1, y + 48, 1, 5, brass);
        }
        for (int sprig = 0; sprig < 7; sprig++) {
            int vx = px + (side == 0 ? 1 : 9) + sprig % 3 - 1;
            int vy = y + 16 + sprig * 6 + side * 2;
            fill_rect(r, vx, vy, 2, 9, moss);
            fill_rect(r, vx - 2, vy + 1, 3, 3, leaf);
            fill_rect(r, vx + 2, vy + 4, 3, 2, moss);
            fill_rect(r, vx - 1, vy + 1, 1, 1, worn);
        }
    }
    fill_rect(r, x + 14, y + 66, 44, 3, mortar);
    fill_rect(r, x + 17, y + 66, 38, 1, edge);
    fill_rect(r, x + 12, y + 69, 48, 3, stones[2]);
    fill_rect(r, x + 13, y + 69, 46, 1, worn);
    fill_rect(r, x + 29, y + 69, 1, 3, mortar);
    fill_rect(r, x + 46, y + 69, 1, 3, mortar);
}

void draw_labyrinth_entrance(Renderer *r, int tile_x, int tile_y, int open) {
    if (!r->labyrinth_texture) {
        draw_labyrinth_entrance_fallback(r, tile_x, tile_y, open);
        return;
    }
    // The sheet holds the sealed frame, then the open frame, 3x3 tiles each.
    SDL_Rect source = {open ? 3 * TILE_SIZE : 0, 0, 3 * TILE_SIZE, 3 * TILE_SIZE};
    SDL_Rect destination = {
        (tile_x - 1) * TILE_SIZE, (tile_y - 2) * TILE_SIZE,
        3 * TILE_SIZE, 3 * TILE_SIZE
    };
    SDL_RenderCopy(r->sdl, r->labyrinth_texture, &source, &destination);
}

void draw_swamp_entrance(Renderer *r, int tile_x, int tile_y) {
    if (!r->swamp_entrance_texture) {
        draw_labyrinth_entrance_fallback(r, tile_x, tile_y, 1);
        return;
    }
    SDL_Rect destination = {(tile_x - 3) * TILE_SIZE, (tile_y - 5) * TILE_SIZE, 7 * TILE_SIZE, 6 * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->swamp_entrance_texture, NULL, &destination);
}

void draw_area_entrance(Renderer *r, TownExitStyle style, int tile_x, int tile_y, int rows) {
    if (!r->area_entrance_textures[style]) {
        draw_labyrinth_entrance_fallback(r, tile_x, tile_y, 1);
        return;
    }
    SDL_Rect destination = {(tile_x - 3) * TILE_SIZE, (tile_y - rows + 1) * TILE_SIZE, 7 * TILE_SIZE, rows * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->area_entrance_textures[style], NULL, &destination);
}

void draw_forest_entrance(Renderer *r, int tile_x, int tile_y) {
    if (!r->forest_entrance_texture) {
        draw_labyrinth_entrance_fallback(r, tile_x, tile_y, 1);
        return;
    }
    SDL_Rect destination = {(tile_x - 3) * TILE_SIZE, (tile_y - 5) * TILE_SIZE, 7 * TILE_SIZE, 6 * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->forest_entrance_texture, NULL, &destination);
}

void draw_mountain_entrance(Renderer *r, int tile_x, int tile_y) {
    if (!r->mountain_entrance_texture) {
        draw_labyrinth_entrance_fallback(r, tile_x, tile_y, 1);
        return;
    }
    SDL_Rect destination = {(tile_x - 3) * TILE_SIZE, (tile_y - 5) * TILE_SIZE, 7 * TILE_SIZE, 6 * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->mountain_entrance_texture, NULL, &destination);
}

void draw_dungeon_entrance(Renderer *r, int tile_x, int tile_y) {
    if (!r->dungeon_entrance_texture) {
        draw_labyrinth_entrance_fallback(r, tile_x, tile_y, 1);
        return;
    }
    SDL_Rect destination = {(tile_x - 1) * TILE_SIZE, (tile_y - 2) * TILE_SIZE, 3 * TILE_SIZE, 3 * TILE_SIZE};
    SDL_RenderCopy(r->sdl, r->dungeon_entrance_texture, NULL, &destination);
}

void draw_labyrinth_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color floor = {(Uint8)(44 + (map_x + map_y) % 3 * 3), 48, 42, 255};
    SDL_Color seam = {31, 35, 31, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, floor);
    fill_rect(r, x, y + 11, TILE_SIZE, 2, seam);
    fill_rect(r, x + ((map_y & 1) ? 6 : 16), y, 2, 11, seam);
    fill_rect(r, x + ((map_y & 1) ? 17 : 7), y + 13, 2, 11, seam);
    if ((map_x * 7 + map_y * 3) % 9 == 0) {
        fill_rect(r, x + 4, y + 5, 5, 2, (SDL_Color){44, 83, 44, 255});
    }
}

void draw_labyrinth_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color dark = {27, 33, 29, 255};
    SDL_Color stone = {59, 68, 58, 255};
    SDL_Color edge = {80, 88, 72, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, dark);
    fill_rect(r, x + 2, y + 2, 20, 20, stone);
    fill_rect(r, x + 2, y + 2, 20, 2, edge);
    fill_rect(r, x + 2, y + 11, 20, 2, dark);
    fill_rect(r, x + ((map_y & 1) ? 7 : 14), y + 3, 2, 8, dark);
    fill_rect(r, x + ((map_y & 1) ? 15 : 8), y + 13, 2, 9, dark);
    if ((map_x + map_y * 2) % 7 == 0) {
        fill_rect(r, x + 3, y + 3, 4, 10, (SDL_Color){39, 91, 43, 255});
    }
}

void draw_labyrinth_exit(Renderer *r, int tile_x, int tile_y) {
    draw_labyrinth_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_EXIT, (SDL_Color){134, 153, 107, 255});
}

void draw_labyrinth_stairs(Renderer *r, int tile_x, int tile_y) {
    draw_labyrinth_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_STAIRS_DOWN, (SDL_Color){160, 150, 106, 255});
}

void draw_labyrinth_switch(Renderer *r, int tile_x, int tile_y, int active) {
    draw_labyrinth_floor(r, tile_x, tile_y, tile_x, tile_y);
    SDL_Color accent = active ? (SDL_Color){72, 224, 192, 255} : (SDL_Color){62, 70, 66, 255};
    draw_world_prop(r, tile_x, tile_y, PROP_SEAL, accent);
}

void draw_labyrinth_gate(Renderer *r, int tile_x, int tile_y) {
    draw_labyrinth_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_GATE, (SDL_Color){116, 102, 75, 255});
}

void draw_labyrinth_relic(Renderer *r, int tile_x, int tile_y) {
    draw_labyrinth_floor(r, tile_x, tile_y, tile_x, tile_y);
    draw_world_prop(r, tile_x, tile_y, PROP_RELIC, (SDL_Color){228, 220, 181, 255});
}

void draw_floor_gold(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_COINS, (SDL_Color){220, 168, 43, 255});
}

void draw_floor_gold_and_item(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_MIXED_LOOT, (SDL_Color){220, 168, 43, 255});
}

void draw_trap_warning(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_WARNING, (SDL_Color){190, 152, 72, 255});
}

void draw_trap_spike(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_SPIKES, (SDL_Color){152, 158, 170, 255});
}

void draw_trap_fire(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_FLAME, (SDL_Color){230, 100, 20, 255});
}

void draw_trap_poison(Renderer *r, int tile_x, int tile_y) {
    draw_world_prop(r, tile_x, tile_y, PROP_POISON, (SDL_Color){54, 174, 70, 255});
}
