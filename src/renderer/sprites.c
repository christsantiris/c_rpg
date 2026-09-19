#include "sprites.h"

static void fill_rect(Renderer *r, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(r->sdl, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r->sdl, &rect);
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

void draw_forest_floor(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    SDL_Color soil[3] = {
        {15, 32, 22, 255},
        {18, 36, 23, 255},
        {20, 34, 23, 255}
    };
    SDL_Color moss = {29, 58, 32, 255};
    SDL_Color moss_light = {42, 76, 39, 255};
    SDL_Color leaf = {70, 65, 34, 255};

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

void draw_forest_wall(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    unsigned int seed = forest_tile_seed(map_x, map_y);
    int trunk_x = 7 + (int)((seed >> 4) % 7u);
    unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
        ((seed >> 16) & 1u)) & 1u;
    int canopy_shift = (seed >> 20) % 3u == 0u ? (int)frame : 0;
    SDL_Color shadow = {6, 21, 13, 255};
    SDL_Color foliage = {22, 51, 28, 255};
    SDL_Color foliage_light = {37, 75, 37, 255};
    SDL_Color bark = {64, 54, 35, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, shadow);
    fill_rect(r, x + 1, y + 5, 9, 10, foliage);
    fill_rect(r, x + 14, y + 9, 9, 10, foliage);
    fill_rect(r, x + 3 + canopy_shift, y + 15, 7, 5, foliage_light);
    fill_rect(r, x + 17 + canopy_shift, y + 4, 5, 4, foliage_light);
    int shape = (int)((seed >> 11) % 4u);
    if (shape == 0) {
        fill_rect(r, x + 2, y + 7, 20, 13, foliage);
        fill_rect(r, x + 4, y + 4, 11, 9,
            (SDL_Color){29, 66, 33, 255});
        fill_rect(r, x + 12, y + 11, 10, 10,
            (SDL_Color){30, 61, 31, 255});
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
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 5, y + 17, 15, 5, (SDL_Color){45, 58, 48, 255});
    fill_rect(r, x + 8, y + 5, 9, 14, (SDL_Color){84, 103, 91, 255});
    fill_rect(r, x + 10, y + 2, 5, 5, (SDL_Color){109, 132, 113, 255});
    fill_rect(r, x + 11, y + 8, 3, 7, (SDL_Color){94, 224, 126, 255});
    fill_rect(r, x + 9, y + 10, 7, 3, (SDL_Color){94, 224, 126, 255});
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
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 5, y + 17, 15, 5, (SDL_Color){45, 58, 48, 255});
    fill_rect(r, x + 8, y + 6, 8, 13, (SDL_Color){75, 88, 79, 255});
    fill_rect(r, x + 10, y + 3, 5, 5, (SDL_Color){96, 109, 98, 255});
    fill_rect(r, x + 10, y + 9, 3, 3, (SDL_Color){64, 124, 76, 255});
    fill_rect(r, x + 13, y + 12, 3, 2, (SDL_Color){64, 124, 76, 255});
}

static unsigned int mountain_tile_seed(int map_x, int map_y) {
    unsigned int seed = (unsigned int)map_x * 2654435761u ^
        (unsigned int)map_y * 2246822519u;
    seed ^= seed >> 16;
    seed *= 3266489917u;
    return seed ^ (seed >> 15);
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
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {87, 71, 64, 255};
    SDL_Color rim = {139, 103, 72, 255};
    SDL_Color timber = {85, 48, 32, 255};
    SDL_Color iron = {105, 101, 103, 255};

    draw_mountain_wall(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 3, y + 1, 18, 23, (SDL_Color){16, 14, 19, 255});
    fill_rect(r, x + 5, y + 3, 14, 21, timber);
    fill_rect(r, x + 11, y + 3, 2, 21, (SDL_Color){45, 28, 25, 255});
    fill_rect(r, x + 1, y + 2, 4, 22, stone);
    fill_rect(r, x + 19, y + 2, 4, 22, stone);
    fill_rect(r, x + 1, y + 2, 22, 3, rim);
    fill_rect(r, x + 5, y + 7, 14, 2, iron);
    fill_rect(r, x + 5, y + 17, 14, 2, iron);
    fill_rect(r, x + 10, y + 10, 4, 5, (SDL_Color){167, 60, 35, 255});
    fill_rect(r, x + 11, y + 11, 2, 3, (SDL_Color){231, 139, 61, 255});
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
    SDL_Color ripple = deep ? (SDL_Color){29, 96, 122, 255} :
        (SDL_Color){68, 152, 154, 255};
    SDL_Color glint = deep ? (SDL_Color){51, 120, 138, 255} :
        (SDL_Color){111, 191, 180, 255};

    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE,
        deep ? deep_water[seed % 3u] : shallow[seed % 3u]);
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

void draw_coast_deep_water(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    draw_coast_water(r, tile_x, tile_y, map_x, map_y, 1);
}

void draw_coast_shore(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, unsigned int edges, TileType water) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    int shift = (int)((coast_tile_seed(map_x, map_y) >> 7) % 4u);
    int deep = water == TILE_COAST_DEEP_WATER ||
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
        draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
        fill_rect(r, x + 3, y + 9, 18, 5,
            (SDL_Color){15, 47, 52, 255});
        fill_rect(r, x + 5, y + 12, 12, 1,
            (SDL_Color){88, 86, 67, 255});
    }
    SDL_Color rim = amber ? (SDL_Color){183, 127, 51, 255} : (SDL_Color){48, 157, 179, 255};
    fill_rect(r, x, y, 3, 3, rim);
    fill_rect(r, x + TILE_SIZE - 3, y + TILE_SIZE - 3, 3, 3, rim);
}

void draw_coast_sluice(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {43, 91, 89, 255};
    SDL_Color rim = {91, 151, 132, 255};
    SDL_Color brass = {213, 157, 61, 255};

    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x, y + 12, 5, 5, stone);
    fill_rect(r, x + 4, y + 11, 16, 12, stone);
    fill_rect(r, x + 5, y + 11, 14, 2, rim);
    fill_rect(r, x + 7, y + 4, 10, 12, (SDL_Color){14, 45, 52, 255});
    fill_rect(r, x + 8, y + 5, 8, 10, rim);
    fill_rect(r, x + 10, y + 7, 4, 6, (SDL_Color){14, 45, 52, 255});
    fill_rect(r, x + 11, y + 2, 2, 5, brass);
    fill_rect(r, x + 9, y + 1, 6, 2, brass);
    fill_rect(r, x + 8, y + 18, 8, 2, brass);
}

void draw_coast_sluice_intake(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color rim = {91, 151, 132, 255};
    SDL_Color iron = {78, 91, 88, 255};

    fill_rect(r, x + 2, y + 6, 20, 13, (SDL_Color){9, 33, 44, 255});
    fill_rect(r, x + 2, y + 6, 20, 2, rim);
    fill_rect(r, x + 2, y + 18, 20, 2, rim);
    fill_rect(r, x + 5, y + 8, 2, 10, iron);
    fill_rect(r, x + 11, y + 8, 2, 10, iron);
    fill_rect(r, x + 17, y + 8, 2, 10, iron);
    fill_rect(r, x + 21, y + 11, 3, 5, (SDL_Color){213, 157, 61, 255});
}

void draw_coast_sluice_conduit(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;

    fill_rect(r, x, y + 9, TILE_SIZE, 9, (SDL_Color){9, 33, 44, 255});
    fill_rect(r, x, y + 9, TILE_SIZE, 2, (SDL_Color){91, 151, 132, 255});
    fill_rect(r, x, y + 16, TILE_SIZE, 2, (SDL_Color){43, 91, 89, 255});
    fill_rect(r, x, y + 12, TILE_SIZE, 3, (SDL_Color){154, 116, 59, 255});
    fill_rect(r, x + 3, y + 12, 5, 1, (SDL_Color){213, 157, 61, 255});
    fill_rect(r, x + 17, y + 12, 4, 1, (SDL_Color){213, 157, 61, 255});
}

void draw_coast_cache(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 4, y + 8, 17, 12, (SDL_Color){102, 83, 46, 255});
    fill_rect(r, x + 4, y + 8, 17, 3, (SDL_Color){191, 157, 66, 255});
    fill_rect(r, x + 11, y + 11, 3, 5, (SDL_Color){243, 201, 91, 255});
    fill_rect(r, x + 3, y + 19, 7, 2, (SDL_Color){42, 136, 119, 255});
}

void draw_coast_tide_control(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_floor(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 5, y + 15, 14, 7, (SDL_Color){43, 91, 89, 255});
    fill_rect(r, x + 8, y + 7, 8, 10, (SDL_Color){91, 151, 132, 255});
    fill_rect(r, x + 10, y + 3, 4, 8, (SDL_Color){196, 158, 69, 255});
    fill_rect(r, x + 12, y + 2, 7, 3, (SDL_Color){90, 231, 207, 255});
}

void draw_coast_beacon(Renderer *r, int tile_x, int tile_y, int map_x, int map_y, int lit) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_coast_shallow_water(r, tile_x, tile_y, map_x, map_y);
    fill_rect(r, x + 5, y + 17, 14, 5, (SDL_Color){34, 65, 67, 255});
    fill_rect(r, x + 8, y + 9, 8, 9, (SDL_Color){67, 104, 97, 255});
    fill_rect(r, x + 6, y + 7, 12, 4, (SDL_Color){116, 126, 98, 255});
    if (lit) {
        unsigned int frame = (SDL_GetTicks() / AMBIENT_FRAME_MS +
            ((coast_tile_seed(map_x, map_y) >> 20) & 1u)) & 1u;
        if (frame == 0u) {
            fill_rect(r, x + 9, y + 3, 7, 6,
                (SDL_Color){47, 221, 212, 255});
            fill_rect(r, x + 11, y, 4, 7,
                (SDL_Color){255, 211, 77, 255});
            fill_rect(r, x + 12, y + 2, 3, 4,
                (SDL_Color){255, 244, 156, 255});
        } else {
            fill_rect(r, x + 9, y + 4, 7, 5,
                (SDL_Color){47, 221, 212, 255});
            fill_rect(r, x + 10, y + 1, 5, 7,
                (SDL_Color){255, 211, 77, 255});
            fill_rect(r, x + 11, y + 3, 3, 3,
                (SDL_Color){255, 244, 156, 255});
        }
    } else {
        fill_rect(r, x + 9, y + 5, 7, 3, (SDL_Color){34, 42, 45, 255});
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
        case ITEM_VISUAL_MAGIC_LONGBOW:
        case ITEM_VISUAL_MAGIC_DAGGER:
        case ITEM_VISUAL_MAGIC_GREATSWORD:
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
    if (weapon->visual_id == ITEM_VISUAL_RUSTY_SWORD) {
        blade = (SDL_Color){151, 77, 45, 255};
        edge = (SDL_Color){190, 105, 59, 255};
    }

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
    draw_weapon_line(r, bx, by, dx, dy, -4, 0, 9, 0, handle);
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
    draw_weapon_line(r, bx, by, dx, dy, -5, 0, 8, 0, wood);
    draw_weapon_line(r, bx, by, dx, dy, 8, -2, 11, 0, gem);
    draw_weapon_line(r, bx, by, dx, dy, 8, 2, 11, 0, gem);
    draw_weapon_line(r, bx, by, dx, dy, 9, -1, 9, 1,
        magic || runed ? core : gem);
}

static void draw_player_bow(Renderer *r, const Item *weapon, int bx, int by, int dx, int dy) {
    int magic = weapon_is_magic(weapon);
    int length = weapon->visual_id == ITEM_VISUAL_BOW ? 8 : 10;
    SDL_Color wood = magic ? (SDL_Color){151, 101, 215, 255}
        : (SDL_Color){151, 100, 48, 255};
    SDL_Color string = magic ? (SDL_Color){208, 248, 255, 255}
        : (SDL_Color){217, 214, 191, 255};
    SDL_Color arrow = magic ? (SDL_Color){80, 228, 245, 255}
        : (SDL_Color){190, 201, 204, 255};
    draw_weapon_line(r, bx, by, dx, dy, -length / 2, 3, 0, -1, wood);
    draw_weapon_line(r, bx, by, dx, dy, 0, -1, length / 2, 3, wood);
    draw_weapon_line(r, bx, by, dx, dy, -length / 2, 3,
        length / 2, 3, string);
    draw_weapon_line(r, bx, by, dx, dy, -4, 0, length, 0, arrow);
    draw_weapon_line(r, bx, by, dx, dy, length - 2, -2,
        length, 0, arrow);
    draw_weapon_line(r, bx, by, dx, dy, length - 2, 2,
        length, 0, arrow);
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

    int px = -dy;
    int py = dx;
    int bx = x + TILE_SIZE / 2 + px * 6 * hand_side;
    int by = y + TILE_SIZE / 2 + 1 + py * 6 * hand_side;
    if (weapon->weapon_family == WEAPON_FAMILY_AXE) {
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
    int px = -dy;
    int py = dx;
    int bx = x + TILE_SIZE / 2 - px * 7;
    int by = y + TILE_SIZE / 2 + 1 - py * 7;
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
    } else if (shield->visual_id == ITEM_VISUAL_MAGIC_SHIELD) {
        rim = (SDL_Color){88, 230, 247, 255};
        face = (SDL_Color){79, 65, 153, 255};
        mark = (SDL_Color){221, 250, 255, 255};
    }
    fill_rect(r, bx - 5, by - 6, 10, 12, rim);
    fill_rect(r, bx - 3, by - 4, 6, 9, face);
    fill_rect(r, bx - 1, by - 4, 2, 8, mark);
    fill_rect(r, bx - 3, by - 1, 6, 2, mark);
}

void draw_player(Renderer *r, int tile_x, int tile_y, PlayerClass player_class, const Item *main_hand, const Item *off_hand, const Item *armor, int facing_dx, int facing_dy) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outline = {16, 18, 30, 255};
    SDL_Color skin = {210, 158, 112, 255};
    SDL_Color leather = {78, 48, 30, 255};
    SDL_Color steel = {184, 194, 204, 255};

    if (player_class == CLASS_MAGE) {
        SDL_Color robe = {42, 72, 150, 255};
        SDL_Color robe_hi = {83, 133, 222, 255};
        fill_rect(r, x + 11, y + 1, 3, 3, robe_hi);
        fill_rect(r, x + 9, y + 3, 7, 4, robe);
        fill_rect(r, x + 7, y + 6, 11, 3, robe);
        fill_rect(r, x + 5, y + 8, 14, 2, outline);
        fill_rect(r, x + 7, y + 8, 10, 1, robe_hi);
        fill_rect(r, x + 9, y + 10, 6, 4, skin);
        fill_rect(r, x + 10, y + 11, 1, 1, outline);
        fill_rect(r, x + 14, y + 11, 1, 1, outline);
        fill_rect(r, x + 5, y + 13, 14, 9, robe);
        fill_rect(r, x + 3, y + 14, 4, 6, robe_hi);
        fill_rect(r, x + 17, y + 14, 4, 6, robe_hi);
        fill_rect(r, x + 9, y + 15, 6, 6, robe_hi);
        fill_rect(r, x + 11, y + 16, 2, 4, (SDL_Color){166, 230, 239, 255});
        fill_rect(r, x + 4, y + 21, 16, 2, outline);
        fill_rect(r, x + 7, y + 21, 10, 1, robe_hi);
    } else if (player_class == CLASS_ROGUE) {
        SDL_Color hood = {43, 81, 51, 255};
        SDL_Color cloth = {83, 119, 69, 255};
        fill_rect(r, x + 11, y + 1, 3, 3, hood);
        fill_rect(r, x + 8, y + 3, 9, 4, hood);
        fill_rect(r, x + 6, y + 6, 12, 6, hood);
        fill_rect(r, x + 9, y + 8, 6, 4, skin);
        fill_rect(r, x + 8, y + 10, 8, 2, outline);
        fill_rect(r, x + 10, y + 9, 1, 1, (SDL_Color){207, 222, 176, 255});
        fill_rect(r, x + 14, y + 9, 1, 1, (SDL_Color){207, 222, 176, 255});
        fill_rect(r, x + 6, y + 12, 12, 9, cloth);
        fill_rect(r, x + 4, y + 12, 4, 8, hood);
        fill_rect(r, x + 17, y + 13, 3, 9, hood);
        fill_rect(r, x + 8, y + 13, 3, 7, leather);
        fill_rect(r, x + 10, y + 16, 7, 2, leather);
        fill_rect(r, x + 7, y + 20, 4, 3, outline);
        fill_rect(r, x + 14, y + 20, 4, 3, outline);
        fill_rect(r, x + 7, y + 20, 3, 1, cloth);
        fill_rect(r, x + 15, y + 20, 3, 1, cloth);
    } else {
        SDL_Color blue = {47, 82, 176, 255};
        SDL_Color gold = {212, 168, 71, 255};
        fill_rect(r, x + 4, y + 10, 16, 11, outline);
        fill_rect(r, x + 6, y + 11, 12, 9, blue);
        fill_rect(r, x + 7, y + 2, 10, 4, steel);
        fill_rect(r, x + 6, y + 5, 12, 2, steel);
        fill_rect(r, x + 8, y + 7, 8, 4, skin);
        fill_rect(r, x + 6, y + 7, 2, 5, steel);
        fill_rect(r, x + 16, y + 7, 2, 5, steel);
        fill_rect(r, x + 9, y + 8, 1, 1, outline);
        fill_rect(r, x + 14, y + 8, 1, 1, outline);
        fill_rect(r, x + 11, y, 3, 3, gold);
        fill_rect(r, x + 3, y + 10, 6, 6, steel);
        fill_rect(r, x + 15, y + 10, 6, 6, steel);
        fill_rect(r, x + 5, y + 11, 3, 2, (SDL_Color){226, 231, 232, 255});
        fill_rect(r, x + 17, y + 11, 3, 2, (SDL_Color){226, 231, 232, 255});
        fill_rect(r, x + 11, y + 13, 2, 6, gold);
        fill_rect(r, x + 7, y + 20, 5, 3, outline);
        fill_rect(r, x + 14, y + 20, 5, 3, outline);
        fill_rect(r, x + 8, y + 20, 3, 1, steel);
        fill_rect(r, x + 15, y + 20, 3, 1, steel);
    }
    if (armor && armor->armor_family == ARMOR_FAMILY_HEAVY) {
        SDL_Color plate = {139, 150, 162, 255};
        if (armor->visual_id == ITEM_VISUAL_MAGIC_PLATE) {
            plate = (SDL_Color){92, 190, 218, 255};
        } else if (armor->visual_id == ITEM_VISUAL_SCALE_MAIL) {
            plate = (SDL_Color){165, 125, 70, 255};
        } else if (armor->visual_id == ITEM_VISUAL_PLATE_ARMOR) {
            plate = (SDL_Color){190, 197, 205, 255};
        }
        fill_rect(r, x + 4, y + 10, 16, 3, plate);
        fill_rect(r, x + 6, y + 13, 12, 6, plate);
        fill_rect(r, x + 3, y + 10, 3, 6, plate);
        fill_rect(r, x + 18, y + 10, 3, 6, plate);
    } else if (armor && armor->armor_family == ARMOR_FAMILY_LIGHT) {
        SDL_Color cloak = {102, 66, 39, 255};
        if (armor->visual_id == ITEM_VISUAL_SHADOW_ARMOR) {
            cloak = (SDL_Color){67, 39, 88, 255};
        } else if (armor->visual_id == ITEM_VISUAL_RANGER_CLOAK) {
            cloak = (SDL_Color){58, 112, 55, 255};
        } else if (armor->visual_id == ITEM_VISUAL_STUDDED_LEATHER) {
            cloak = (SDL_Color){128, 88, 48, 255};
        }
        fill_rect(r, x + 5, y + 5, 14, 3, cloak);
        fill_rect(r, x + 5, y + 12, 14, 7, cloak);
        fill_rect(r, x + 8, y + 13, 2, 6, leather);
    } else if (armor && armor->armor_family == ARMOR_FAMILY_ROBE) {
        SDL_Color robe = {46, 88, 168, 255};
        if (armor->visual_id == ITEM_VISUAL_ARCHMAGE_ROBES) {
            robe = (SDL_Color){112, 68, 178, 255};
        } else if (armor->visual_id == ITEM_VISUAL_ENCHANTER_ROBES) {
            robe = (SDL_Color){78, 66, 163, 255};
        } else if (armor->visual_id == ITEM_VISUAL_RUNED_ROBES) {
            robe = (SDL_Color){38, 112, 133, 255};
        }
        SDL_Color rune = armor->rarity == ITEM_RARITY_COMMON
            ? (SDL_Color){91, 132, 201, 255}
            : (SDL_Color){91, 231, 225, 255};
        fill_rect(r, x + 5, y + 12, 14, 8, robe);
        fill_rect(r, x + 11, y + 13, 2, 6, rune);
        fill_rect(r, x + 8, y + 16, 8, 2, rune);
    }
    draw_equipped_player_weapon(r, main_hand, x, y, facing_dx, facing_dy, 1);
    if (off_hand && off_hand->type == ITEM_SHIELD) {
        draw_equipped_player_shield(r, off_hand, x, y, facing_dx,
            facing_dy);
    } else {
        draw_equipped_player_weapon(r, off_hand, x, y, facing_dx,
            facing_dy, -1);
    }
}

void draw_stairs_up(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = {18,  18,  35,  255};
    SDL_Color step   = {180, 140, 60,  255};
    SDL_Color edge   = {220, 180, 80,  255};
    fill_rect(r, x,         y,         TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3,     y + 14,    18,        3,         step);
    fill_rect(r, x + 6,     y + 10,    12,        3,         step);
    fill_rect(r, x + 9,     y + 6,     6,         3,         step);
    fill_rect(r, x + 3,     y + 14,    18,        1,         edge);
    fill_rect(r, x + 6,     y + 10,    12,        1,         edge);
    fill_rect(r, x + 9,     y + 6,     6,         1,         edge);
}

void draw_stairs_down(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base   = {18,  18,  35,  255};
    SDL_Color step   = {100, 80,  160, 255};
    SDL_Color edge   = {140, 110, 200, 255};
    fill_rect(r, x,         y,         TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3,     y + 4,     18,        3,         step);
    fill_rect(r, x + 6,     y + 8,     12,        3,         step);
    fill_rect(r, x + 9,     y + 12,    6,         3,         step);
    fill_rect(r, x + 3,     y + 4,     18,        1,         edge);
    fill_rect(r, x + 6,     y + 8,     12,        1,         edge);
    fill_rect(r, x + 9,     y + 12,    6,         1,         edge);
}

void draw_return_exit(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = {12, 14, 28, 255};
    SDL_Color glow = {96, 210, 190, 255};
    SDL_Color core = {190, 255, 232, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x + 3, y + 3, 18, 18, glow);
    fill_rect(r, x + 6, y + 5, 12, 16, base);
    fill_rect(r, x + 9, y + 7, 6, 12, core);
}

void draw_broken_burial_seal(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x + 3, y + 3, 18, 18, (SDL_Color){42, 38, 52, 255});
    fill_rect(r, x + 5, y + 5, 14, 14, (SDL_Color){91, 83, 99, 255});
    fill_rect(r, x + 8, y + 8, 8, 8, (SDL_Color){54, 46, 67, 255});
    fill_rect(r, x + 11, y + 5, 2, 6, (SDL_Color){185, 55, 192, 255});
    fill_rect(r, x + 8, y + 10, 5, 2, (SDL_Color){185, 55, 192, 255});
    fill_rect(r, x + 12, y + 11, 2, 8, (SDL_Color){28, 24, 36, 255});
    fill_rect(r, x + 14, y + 14, 5, 2, (SDL_Color){28, 24, 36, 255});
}

void draw_restored_burial_seal(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x + 3, y + 3, 18, 18, (SDL_Color){63, 59, 66, 255});
    fill_rect(r, x + 5, y + 5, 14, 14, (SDL_Color){118, 111, 112, 255});
    fill_rect(r, x + 8, y + 8, 8, 8, (SDL_Color){72, 67, 72, 255});
    fill_rect(r, x + 11, y + 6, 2, 12, (SDL_Color){226, 191, 82, 255});
    fill_rect(r, x + 7, y + 11, 10, 2, (SDL_Color){246, 220, 126, 255});
    fill_rect(r, x + 10, y + 10, 4, 4, (SDL_Color){255, 236, 158, 255});
}

void draw_locked_door(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color stone = {50, 45, 80, 255};
    SDL_Color wood = {66, 38, 54, 255};
    SDL_Color edge = {116, 72, 94, 255};
    SDL_Color iron = {146, 142, 164, 255};
    fill_rect(r, x, y, TILE_SIZE, TILE_SIZE, stone);
    fill_rect(r, x+3, y+2, 18, 22, edge);
    fill_rect(r, x+5, y+4, 14, 20, wood);
    fill_rect(r, x+7, y+4, 2, 20, edge);
    fill_rect(r, x+15, y+4, 2, 20, edge);
    fill_rect(r, x+10, y+11, 5, 6, iron);
    fill_rect(r, x+11, y+9, 3, 4, iron);
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
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color gold = {226, 184, 54, 255};
    SDL_Color shine = {255, 232, 126, 255};
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x+4, y+7, 8, 8, gold);
    fill_rect(r, x+6, y+9, 4, 4, (SDL_Color){18, 18, 35, 255});
    fill_rect(r, x+11, y+10, 10, 3, gold);
    fill_rect(r, x+17, y+13, 3, 4, gold);
    fill_rect(r, x+5, y+7, 3, 2, shine);
}

void draw_crypt_door(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_locked_door(r, tile_x, tile_y);
    fill_rect(r, x+10, y+11, 5, 6, (SDL_Color){202, 164, 58, 255});
    fill_rect(r, x+11, y+9, 3, 4, (SDL_Color){242, 214, 112, 255});
}

void draw_crypt_key(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color iron = {146, 166, 184, 255};
    SDL_Color shine = {220, 234, 240, 255};
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x+4, y+7, 8, 8, iron);
    fill_rect(r, x+6, y+9, 4, 4, (SDL_Color){18, 18, 35, 255});
    fill_rect(r, x+11, y+10, 10, 3, iron);
    fill_rect(r, x+17, y+13, 3, 4, iron);
    fill_rect(r, x+5, y+7, 3, 2, shine);
}

void draw_crypt_cache(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x+3, y+9, 18, 12, (SDL_Color){74, 42, 28, 255});
    fill_rect(r, x+4, y+6, 16, 6, (SDL_Color){112, 66, 34, 255});
    fill_rect(r, x+5, y+7, 14, 2, (SDL_Color){166, 102, 42, 255});
    fill_rect(r, x+10, y+11, 5, 6, (SDL_Color){226, 184, 54, 255});
}

void draw_dungeon_gate(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    SDL_Color iron = {102, 106, 132, 255};
    SDL_Color shine = {174, 180, 204, 255};
    fill_rect(r, x+2, y+3, 20, 3, iron);
    fill_rect(r, x+2, y+18, 20, 3, iron);
    for (int bar = 4; bar <= 19; bar += 5) {
        fill_rect(r, x+bar, y+2, 3, 21, iron);
        fill_rect(r, x+bar, y+2, 1, 19, shine);
    }
}

void draw_dungeon_switch(Renderer *r, int tile_x, int tile_y, int active) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x+5, y+6, 14, 13, (SDL_Color){50, 48, 68, 255});
    fill_rect(r, x+7, y+8, 10, 9, (SDL_Color){90, 86, 108, 255});
    SDL_Color rune = active ?
        (SDL_Color){80, 220, 150, 255} :
        (SDL_Color){220, 92, 62, 255};
    fill_rect(r, x+10, y+10, 4, 5, rune);
    fill_rect(r, x+8, y+12, 8, 2, rune);
}

void draw_portal(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color outer = {68, 72, 218, 255};
    SDL_Color glow = {86, 214, 244, 255};
    SDL_Color core = {188, 252, 255, 255};
    draw_floor(r, tile_x, tile_y);
    fill_rect(r, x+3, y+9, 18, 7, outer);
    fill_rect(r, x+6, y+7, 12, 10, glow);
    fill_rect(r, x+9, y+9, 6, 6, core);
    fill_rect(r, x+7, y+11, 10, 3, (SDL_Color){24, 30, 82, 255});
}

void draw_goblin(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {78, 142, 38, 255};
    SDL_Color dark = {28, 48, 26, 255};
    SDL_Color cloth = {92, 58, 30, 255};
    fill_rect(r, x+4, y+6, 16, 3, skin);
    fill_rect(r, x+7, y+4, 10, 9, skin);
    fill_rect(r, x+8, y+7, 2, 2, (SDL_Color){232, 186, 38, 255});
    fill_rect(r, x+14,y+7, 2, 2, (SDL_Color){232, 186, 38, 255});
    fill_rect(r, x+7, y+13, 10, 7, cloth);
    fill_rect(r, x+4, y+14, 3, 5, skin);
    fill_rect(r, x+17,y+13, 3, 6, skin);
    fill_rect(r, x+7, y+20, 4, 2, dark);
    fill_rect(r, x+14,y+20, 4, 2, dark);
    fill_rect(r, x+19,y+10, 2, 8, (SDL_Color){174, 184, 188, 255});
}

void draw_skeleton(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color bone = {210, 210, 190, 255};
    SDL_Color shade = {145, 151, 145, 255};
    SDL_Color dark = {18, 18, 35, 255};
    SDL_Color iron = {174, 184, 192, 255};
    fill_rect(r, x + 2, y + 5, 2, 12, iron);
    fill_rect(r, x + 1, y + 4, 4, 3, (SDL_Color){220, 226, 227, 255});
    fill_rect(r, x + 1, y + 16, 5, 2, (SDL_Color){111, 72, 48, 255});
    fill_rect(r, x + 7, y + 2, 10, 8, bone);
    fill_rect(r, x + 7, y + 3, 2, 6, shade);
    fill_rect(r, x + 8, y + 5, 3, 3, dark);
    fill_rect(r, x + 14, y + 5, 3, 3, dark);
    fill_rect(r, x + 9, y + 9, 6, 3, shade);
    fill_rect(r, x + 11, y + 9, 3, 1, dark);
    fill_rect(r, x + 8, y + 12, 9, 7, dark);
    fill_rect(r, x + 11, y + 12, 3, 9, bone);
    fill_rect(r, x + 8, y + 13, 9, 2, bone);
    fill_rect(r, x + 8, y + 16, 9, 2, shade);
    fill_rect(r, x + 5, y + 13, 3, 3, bone);
    fill_rect(r, x + 17, y + 13, 2, 4, bone);
    fill_rect(r, x + 7, y + 19, 3, 4, bone);
    fill_rect(r, x + 15, y + 19, 3, 4, bone);
    fill_rect(r, x + 19, y + 12, 5, 8, (SDL_Color){101, 69, 48, 255});
    fill_rect(r, x + 20, y + 13, 3, 5, iron);
}

void draw_orc(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {92, 116, 48, 255};
    SDL_Color dark = {30, 38, 26, 255};
    SDL_Color armor = {82, 82, 86, 255};
    fill_rect(r, x+6, y+3, 12, 10, skin);
    fill_rect(r, x+5, y+5, 2, 5, skin);
    fill_rect(r, x+17,y+5, 2, 5, skin);
    fill_rect(r, x+8, y+7, 2, 2, (SDL_Color){232, 166, 34, 255});
    fill_rect(r, x+14,y+7, 2, 2, (SDL_Color){232, 166, 34, 255});
    fill_rect(r, x+8, y+11,2, 3, (SDL_Color){224, 210, 170, 255});
    fill_rect(r, x+15,y+11,2, 3, (SDL_Color){224, 210, 170, 255});
    fill_rect(r, x+4, y+13,16, 7, armor);
    fill_rect(r, x+8, y+14,8, 6, (SDL_Color){84, 48, 28, 255});
    fill_rect(r, x+5, y+20,5, 3, dark);
    fill_rect(r, x+15,y+20,5, 3, dark);
    fill_rect(r, x+20,y+7, 3, 13, (SDL_Color){172, 180, 182, 255});
}

void draw_troll(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {72, 94, 58, 255};
    SDL_Color shade = {42, 56, 40, 255};
    fill_rect(r, x+5, y+3, 14, 10, skin);
    fill_rect(r, x+7, y+6, 3, 2, (SDL_Color){224, 154, 36, 255});
    fill_rect(r, x+14,y+6, 3, 2, (SDL_Color){224, 154, 36, 255});
    fill_rect(r, x+3, y+12,18, 8, skin);
    fill_rect(r, x+2, y+13,4, 8, skin);
    fill_rect(r, x+18,y+12,4, 9, skin);
    fill_rect(r, x+8, y+13,8, 7, (SDL_Color){90, 58, 32, 255});
    fill_rect(r, x+4, y+20,6, 3, shade);
    fill_rect(r, x+15,y+20,6, 3, shade);
    fill_rect(r, x+20,y+5, 3, 15, (SDL_Color){104, 76, 48, 255});
    fill_rect(r, x+19,y+4, 5, 6, (SDL_Color){126, 126, 116, 255});
}

void draw_zombie(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {132, 146, 108, 255};
    SDL_Color cloth = {100, 48, 54, 255};
    SDL_Color dark = {42, 34, 42, 255};
    fill_rect(r, x + 11, y + 2, 9, 8, dark);
    fill_rect(r, x + 10, y + 4, 9, 8, skin);
    fill_rect(r, x + 17, y + 7, 3, 4, (SDL_Color){91, 105, 78, 255});
    fill_rect(r, x + 12, y + 6, 2, 2, (SDL_Color){226, 207, 91, 255});
    fill_rect(r, x + 17, y + 6, 2, 2, dark);
    fill_rect(r, x + 6, y + 11, 12, 9, cloth);
    fill_rect(r, x + 10, y + 12, 7, 3, (SDL_Color){69, 39, 47, 255});
    fill_rect(r, x + 11, y + 15, 5, 2, (SDL_Color){150, 48, 48, 255});
    fill_rect(r, x + 4, y + 12, 4, 3, cloth);
    fill_rect(r, x + 1, y + 13, 5, 3, skin);
    fill_rect(r, x + 17, y + 13, 3, 6, cloth);
    fill_rect(r, x + 19, y + 18, 3, 3, skin);
    fill_rect(r, x + 7, y + 20, 4, 3, dark);
    fill_rect(r, x + 15, y + 19, 4, 4, dark);
    fill_rect(r, x + 5, y + 22, 6, 2, dark);
}

void draw_crypt_bat(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color wing = {92, 42, 126, 255};
    SDL_Color edge = {46, 22, 70, 255};
    SDL_Color body = {54, 32, 72, 255};
    SDL_Color eye = {244, 48, 62, 255};
    fill_rect(r, x + 1, y + 5, 6, 3, edge);
    fill_rect(r, x + 2, y + 8, 7, 4, wing);
    fill_rect(r, x + 4, y + 12, 5, 3, wing);
    fill_rect(r, x + 6, y + 15, 2, 3, edge);
    fill_rect(r, x + 17, y + 5, 6, 3, edge);
    fill_rect(r, x + 15, y + 8, 7, 4, wing);
    fill_rect(r, x + 15, y + 12, 5, 3, wing);
    fill_rect(r, x + 16, y + 15, 2, 3, edge);
    fill_rect(r, x + 8, y + 7, 8, 11, body);
    fill_rect(r, x + 8, y + 6, 3, 3, edge);
    fill_rect(r, x + 13, y + 6, 3, 3, edge);
    fill_rect(r, x + 9, y + 11, 2, 2, eye);
    fill_rect(r, x + 13, y + 11, 2, 2, eye);
    fill_rect(r, x + 11, y + 14, 2, 3, (SDL_Color){207, 197, 188, 255});
    fill_rect(r, x + 10, y + 17, 4, 3, edge);
}

void draw_wraith(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color glow = {112, 224, 232, 255};
    SDL_Color pale = {176, 242, 238, 255};
    SDL_Color shade = {54, 112, 132, 255};
    SDL_Color voidc = {12, 20, 38, 255};
    fill_rect(r, x + 8, y + 2, 8, 3, pale);
    fill_rect(r, x + 6, y + 5, 12, 8, glow);
    fill_rect(r, x + 8, y + 6, 8, 7, voidc);
    fill_rect(r, x + 9, y + 9, 2, 2, pale);
    fill_rect(r, x + 14, y + 9, 2, 2, pale);
    fill_rect(r, x + 5, y + 13, 14, 5, shade);
    fill_rect(r, x + 3, y + 12, 4, 3, glow);
    fill_rect(r, x + 1, y + 15, 4, 3, pale);
    fill_rect(r, x + 18, y + 12, 4, 3, glow);
    fill_rect(r, x + 20, y + 15, 3, 3, pale);
    fill_rect(r, x + 7, y + 18, 10, 2, glow);
    fill_rect(r, x + 9, y + 20, 6, 2, shade);
    fill_rect(r, x + 11, y + 22, 3, 2, glow);
    fill_rect(r, x + 6, y + 20, 2, 2, pale);
    fill_rect(r, x + 17, y + 19, 2, 2, pale);
}

void draw_necromancer(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color robe = {62, 28, 92, 255};
    SDL_Color trim = {116, 54, 148, 255};
    SDL_Color skin = {172, 166, 134, 255};
    SDL_Color magic = {78, 232, 76, 255};
    SDL_Color bone = {210, 204, 170, 255};
    fill_rect(r, x + 2, y + 5, 2, 17, bone);
    fill_rect(r, x + 1, y + 3, 4, 4, bone);
    fill_rect(r, x + 2, y + 2, 2, 2, magic);
    fill_rect(r, x + 10, y + 1, 5, 3, trim);
    fill_rect(r, x + 8, y + 4, 9, 3, trim);
    fill_rect(r, x + 6, y + 7, 13, 7, robe);
    fill_rect(r, x + 8, y + 7, 9, 6, skin);
    fill_rect(r, x + 9, y + 8, 3, 3, (SDL_Color){31, 17, 48, 255});
    fill_rect(r, x + 14, y + 8, 3, 3, (SDL_Color){31, 17, 48, 255});
    fill_rect(r, x + 10, y + 9, 2, 2, magic);
    fill_rect(r, x + 14, y + 9, 2, 2, magic);
    fill_rect(r, x + 5, y + 14, 15, 8, robe);
    fill_rect(r, x + 4, y + 20, 4, 3, robe);
    fill_rect(r, x + 17, y + 20, 4, 3, robe);
    fill_rect(r, x + 9, y + 15, 7, 2, trim);
    fill_rect(r, x + 11, y + 17, 3, 3, magic);
    fill_rect(r, x + 17, y + 13, 4, 4, skin);
    fill_rect(r, x + 19, y + 11, 4, 4, magic);
}

void draw_giant(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {166, 122, 82, 255};
    SDL_Color shade = {104, 72, 50, 255};
    fill_rect(r, x+5, y+2, 14, 10, skin);
    fill_rect(r, x+8, y+5, 2, 2, (SDL_Color){34, 28, 28, 255});
    fill_rect(r, x+15,y+5, 2, 2, (SDL_Color){34, 28, 28, 255});
    fill_rect(r, x+3, y+11,18, 10, skin);
    fill_rect(r, x+1, y+12,4, 9, skin);
    fill_rect(r, x+19,y+11,4, 10, skin);
    fill_rect(r, x+7, y+13,10, 8, (SDL_Color){92, 62, 38, 255});
    fill_rect(r, x+4, y+20,6, 3, shade);
    fill_rect(r, x+15,y+20,6, 3, shade);
    fill_rect(r, x+20,y+3, 3, 17, (SDL_Color){92, 58, 30, 255});
    fill_rect(r, x+19,y+2, 5, 6, (SDL_Color){116, 76, 40, 255});
}

void draw_goblin_king(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color skin = {82, 128, 38, 255};
    SDL_Color crown = {230, 184, 42, 255};
    SDL_Color cloak = {132, 34, 42, 255};
    fill_rect(r, x+5, y+1, 14, 4, crown);
    fill_rect(r, x+6, y, 3, 3, crown);
    fill_rect(r, x+11,y, 3, 3, crown);
    fill_rect(r, x+16,y, 3, 3, crown);
    fill_rect(r, x+4, y+7, 16, 3, skin);
    fill_rect(r, x+7, y+4, 10, 9, skin);
    fill_rect(r, x+8, y+7, 2, 2, (SDL_Color){250, 56, 36, 255});
    fill_rect(r, x+14,y+7, 2, 2, (SDL_Color){250, 56, 36, 255});
    fill_rect(r, x+3, y+12,18, 9, cloak);
    fill_rect(r, x+7, y+13,10, 8, (SDL_Color){72, 50, 28, 255});
    fill_rect(r, x+5, y+20,5, 3, (SDL_Color){26, 34, 24, 255});
    fill_rect(r, x+15,y+20,5, 3, (SDL_Color){26, 34, 24, 255});
    fill_rect(r, x+20,y+8, 2, 12, (SDL_Color){194, 202, 204, 255});
}

void draw_lich_king(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color robe = {42, 24, 76, 255};
    SDL_Color bone = {208, 204, 170, 255};
    SDL_Color glow = {142, 80, 248, 255};
    fill_rect(r, x+7, y+1, 10, 4, (SDL_Color){92, 76, 120, 255});
    fill_rect(r, x+8, y, 2, 3, bone);
    fill_rect(r, x+14,y, 2, 3, bone);
    fill_rect(r, x+7, y+4, 10, 8, bone);
    fill_rect(r, x+9, y+6, 2, 2, glow);
    fill_rect(r, x+14,y+6, 2, 2, glow);
    fill_rect(r, x+5, y+12,14, 9, robe);
    fill_rect(r, x+3, y+14,4, 8, robe);
    fill_rect(r, x+17,y+14,4, 8, robe);
    fill_rect(r, x+4, y+21,5, 2, robe);
    fill_rect(r, x+15,y+21,5, 2, robe);
    fill_rect(r, x+2, y+7, 2, 14, bone);
    fill_rect(r, x+1, y+3, 4, 5, glow);
    fill_rect(r, x+2, y+1, 2, 3, (SDL_Color){206, 166, 255, 255});
}

void draw_demon_lord(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = {142, 38, 34, 255};
    SDL_Color dark = {54, 20, 26, 255};
    SDL_Color horn = {126, 116, 94, 255};
    fill_rect(r, x, y+7, 5, 12, dark);
    fill_rect(r, x+19,y+7,5, 12, dark);
    fill_rect(r, x+5, y+2, 4, 6, horn);
    fill_rect(r, x+15,y+2,4, 6, horn);
    fill_rect(r, x+7, y, 2, 4, horn);
    fill_rect(r, x+15,y, 2, 4, horn);
    fill_rect(r, x+6, y+5,12, 9, body);
    fill_rect(r, x+8, y+8,2, 2, (SDL_Color){248, 160, 28, 255});
    fill_rect(r, x+14,y+8,2, 2, (SDL_Color){248, 160, 28, 255});
    fill_rect(r, x+4, y+13,16, 8, body);
    fill_rect(r, x+8, y+14,8, 6, dark);
    fill_rect(r, x+4, y+20,6, 3, dark);
    fill_rect(r, x+15,y+20,6, 3, dark);
}

void draw_red_dragon(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = {174, 42, 24, 255};
    SDL_Color dark = {92, 24, 20, 255};
    SDL_Color gold = {224, 154, 42, 255};
    fill_rect(r, x, y+6, 5, 12, dark);
    fill_rect(r, x+19,y+6,5, 12, dark);
    fill_rect(r, x+6, y+3,12, 9, body);
    fill_rect(r, x+7, y+1,3, 4, (SDL_Color){196, 174, 126, 255});
    fill_rect(r, x+14,y+1,3, 4, (SDL_Color){196, 174, 126, 255});
    fill_rect(r, x+8, y+6,2, 2, gold);
    fill_rect(r, x+14,y+6,2, 2, gold);
    fill_rect(r, x+5, y+11,14, 10, body);
    fill_rect(r, x+9, y+12,6, 9, gold);
    fill_rect(r, x+4, y+20,6, 3, dark);
    fill_rect(r, x+15,y+20,6, 3, dark);
    fill_rect(r, x+19,y+18,4, 2, body);
    fill_rect(r, x+21,y+16,2, 3, body);
    fill_rect(r, x+1, y+18,3, 3, (SDL_Color){255, 110, 16, 255});
    fill_rect(r, x, y+20,2, 2, (SDL_Color){255, 210, 54, 255});
}

void draw_tarrasque(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color body = {78, 58, 36, 255};
    SDL_Color shell = {112, 88, 52, 255};
    SDL_Color spike = {176, 156, 112, 255};
    fill_rect(r, x+3, y+4, 18, 15, shell);
    fill_rect(r, x+5, y+2, 4, 5, spike);
    fill_rect(r, x+10,y, 4, 6, spike);
    fill_rect(r, x+16,y+2,4, 5, spike);
    fill_rect(r, x+5, y+9, 14, 11, body);
    fill_rect(r, x+7, y+9, 3, 2, (SDL_Color){238, 48, 26, 255});
    fill_rect(r, x+15,y+9, 3, 2, (SDL_Color){238, 48, 26, 255});
    fill_rect(r, x+10,y+12,5, 5, (SDL_Color){48, 36, 28, 255});
    fill_rect(r, x, y+14,6, 7, body);
    fill_rect(r, x+18,y+14,6, 7, body);
    fill_rect(r, x+1, y+20,7, 3, spike);
    fill_rect(r, x+16,y+20,7, 3, spike);
}

static void draw_forest_pixie(Renderer *r, int x, int y) {
    SDL_Color outline = {19, 28, 34, 255};
    SDL_Color wing = {127, 181, 177, 255};
    SDL_Color light = {213, 238, 203, 255};
    SDL_Color robe = {100, 72, 133, 255};
    SDL_Color gold = {238, 189, 84, 255};
    fill_rect(r, x + 1, y + 6, 8, 12, outline);
    fill_rect(r, x + 15, y + 6, 8, 12, outline);
    fill_rect(r, x + 2, y + 7, 6, 8, wing);
    fill_rect(r, x + 4, y + 5, 4, 4, light);
    fill_rect(r, x + 3, y + 15, 4, 4, wing);
    fill_rect(r, x + 16, y + 7, 6, 8, wing);
    fill_rect(r, x + 16, y + 5, 4, 4, light);
    fill_rect(r, x + 17, y + 15, 4, 4, wing);
    fill_rect(r, x + 9, y + 3, 6, 16, outline);
    fill_rect(r, x + 10, y + 4, 4, 5, gold);
    fill_rect(r, x + 10, y + 10, 4, 8, robe);
    fill_rect(r, x + 11, y + 6, 1, 2, outline);
    fill_rect(r, x + 13, y + 6, 1, 2, outline);
    fill_rect(r, x + 8, y + 17, 2, 3, gold);
    fill_rect(r, x + 14, y + 17, 2, 3, gold);
    fill_rect(r, x + 5, y + 21, 2, 2, light);
    fill_rect(r, x + 18, y + 21, 2, 2, light);
}

static void draw_forest_wolf(Renderer *r, int x, int y) {
    SDL_Color outline = {18, 22, 30, 255};
    SDL_Color fur = {83, 87, 108, 255};
    SDL_Color light = {133, 139, 153, 255};
    SDL_Color blight = {125, 210, 102, 255};
    fill_rect(r, x + 1, y + 7, 5, 4, outline);
    fill_rect(r, x + 2, y + 8, 5, 3, fur);
    fill_rect(r, x + 4, y + 9, 14, 10, outline);
    fill_rect(r, x + 6, y + 10, 11, 7, fur);
    fill_rect(r, x + 7, y + 10, 8, 2, light);
    fill_rect(r, x + 16, y + 6, 7, 10, outline);
    fill_rect(r, x + 17, y + 7, 5, 7, fur);
    fill_rect(r, x + 16, y + 3, 3, 5, outline);
    fill_rect(r, x + 20, y + 4, 3, 5, outline);
    fill_rect(r, x + 18, y + 7, 2, 3, light);
    fill_rect(r, x + 20, y + 10, 4, 4, light);
    fill_rect(r, x + 21, y + 13, 3, 2, outline);
    fill_rect(r, x + 20, y + 8, 2, 2, blight);
    fill_rect(r, x + 8, y + 13, 4, 2, blight);
    fill_rect(r, x + 5, y + 17, 4, 6, outline);
    fill_rect(r, x + 6, y + 18, 2, 4, fur);
    fill_rect(r, x + 15, y + 17, 4, 6, outline);
    fill_rect(r, x + 16, y + 18, 2, 4, fur);
}

static void draw_forest_spider(Renderer *r, int x, int y) {
    SDL_Color outline = {19, 18, 30, 255};
    SDL_Color shell = {106, 64, 103, 255};
    SDL_Color light = {171, 105, 146, 255};
    SDL_Color venom = {239, 117, 88, 255};
    for (int side = 0; side < 2; side++) {
        int lx = side == 0 ? 1 : 17;
        int joint = side == 0 ? 5 : 16;
        fill_rect(r, x + lx, y + 4, 6, 2, outline);
        fill_rect(r, x + lx, y + 6, 2, 5, outline);
        fill_rect(r, x + joint, y + 9, 4, 2, outline);
        fill_rect(r, x + lx, y + 11, 7, 2, outline);
        fill_rect(r, x + lx, y + 13, 2, 5, outline);
        fill_rect(r, x + joint, y + 14, 4, 2, outline);
        fill_rect(r, x + lx, y + 18, 7, 2, outline);
        fill_rect(r, x + lx, y + 19, 2, 4, outline);
        fill_rect(r, x + lx + 2, y + 5, 4, 1, light);
        fill_rect(r, x + lx + 2, y + 12, 4, 1, shell);
        fill_rect(r, x + lx + 2, y + 19, 4, 1, shell);
    }
    fill_rect(r, x + 7, y + 9, 11, 10, outline);
    fill_rect(r, x + 8, y + 10, 9, 7, shell);
    fill_rect(r, x + 10, y + 10, 5, 3, light);
    fill_rect(r, x + 9, y + 5, 7, 7, outline);
    fill_rect(r, x + 10, y + 6, 5, 4, shell);
    fill_rect(r, x + 10, y + 8, 2, 2, venom);
    fill_rect(r, x + 14, y + 8, 2, 2, venom);
    fill_rect(r, x + 11, y + 12, 3, 2, venom);
}

static void draw_forest_elf(Renderer *r, int x, int y) {
    SDL_Color outline = {20, 19, 32, 255};
    SDL_Color cloak = {83, 63, 109, 255};
    SDL_Color light = {139, 112, 153, 255};
    SDL_Color face = {185, 166, 178, 255};
    SDL_Color bow = {190, 139, 70, 255};
    fill_rect(r, x + 1, y + 4, 3, 16, bow);
    fill_rect(r, x + 3, y + 2, 3, 3, bow);
    fill_rect(r, x + 3, y + 19, 3, 3, bow);
    fill_rect(r, x + 4, y + 11, 7, 1, face);
    fill_rect(r, x + 9, y + 10, 10, 2, bow);
    fill_rect(r, x + 17, y + 9, 4, 1, face);
    fill_rect(r, x + 6, y + 3, 12, 18, outline);
    fill_rect(r, x + 8, y + 4, 9, 7, cloak);
    fill_rect(r, x + 10, y + 6, 6, 6, face);
    fill_rect(r, x + 6, y + 7, 4, 3, outline);
    fill_rect(r, x + 16, y + 7, 4, 3, outline);
    fill_rect(r, x + 11, y + 8, 2, 2, (SDL_Color){219, 111, 95, 255});
    fill_rect(r, x + 8, y + 13, 9, 7, cloak);
    fill_rect(r, x + 9, y + 13, 3, 6, light);
    fill_rect(r, x + 7, y + 20, 4, 3, outline);
    fill_rect(r, x + 15, y + 20, 4, 3, outline);
}

static void draw_forest_wurm(Renderer *r, int x, int y) {
    SDL_Color outline = {29, 29, 25, 255};
    SDL_Color hide = {127, 103, 65, 255};
    SDL_Color plate = {182, 151, 90, 255};
    SDL_Color mouth = {74, 30, 41, 255};
    fill_rect(r, x + 1, y + 16, 17, 7, outline);
    fill_rect(r, x + 3, y + 17, 13, 5, hide);
    fill_rect(r, x + 5, y + 12, 13, 9, outline);
    fill_rect(r, x + 6, y + 13, 10, 7, hide);
    fill_rect(r, x + 12, y + 5, 9, 11, outline);
    fill_rect(r, x + 13, y + 6, 7, 8, hide);
    fill_rect(r, x + 16, y + 3, 6, 5, plate);
    fill_rect(r, x + 19, y + 8, 5, 6, outline);
    fill_rect(r, x + 20, y + 9, 4, 3, mouth);
    fill_rect(r, x + 21, y + 12, 3, 2, plate);
    fill_rect(r, x + 17, y + 7, 2, 2, (SDL_Color){238, 98, 75, 255});
    fill_rect(r, x + 13, y + 9, 2, 5, plate);
    fill_rect(r, x + 9, y + 14, 2, 6, plate);
    fill_rect(r, x + 5, y + 18, 2, 4, plate);
    fill_rect(r, x + 1, y + 19, 3, 3, plate);
}

static void draw_forest_troll(Renderer *r, int x, int y) {
    SDL_Color outline = {22, 27, 28, 255};
    SDL_Color skin = {92, 111, 98, 255};
    SDL_Color light = {143, 158, 125, 255};
    SDL_Color bark = {105, 73, 47, 255};
    fill_rect(r, x + 3, y + 7, 17, 14, outline);
    fill_rect(r, x + 5, y + 8, 14, 11, skin);
    fill_rect(r, x + 7, y + 4, 10, 9, outline);
    fill_rect(r, x + 8, y + 5, 8, 7, skin);
    fill_rect(r, x + 9, y + 7, 2, 2, (SDL_Color){239, 175, 74, 255});
    fill_rect(r, x + 14, y + 7, 2, 2, (SDL_Color){239, 175, 74, 255});
    fill_rect(r, x + 10, y + 11, 5, 2, outline);
    fill_rect(r, x + 10, y + 12, 2, 2, light);
    fill_rect(r, x + 14, y + 12, 2, 2, light);
    fill_rect(r, x + 3, y + 11, 4, 10, skin);
    fill_rect(r, x + 17, y + 10, 4, 11, skin);
    fill_rect(r, x + 7, y + 14, 10, 4, bark);
    fill_rect(r, x + 6, y + 19, 5, 4, outline);
    fill_rect(r, x + 15, y + 19, 5, 4, outline);
    fill_rect(r, x + 21, y + 2, 2, 20, bark);
    fill_rect(r, x + 19, y + 2, 5, 6, outline);
    fill_rect(r, x + 20, y + 3, 4, 4, bark);
    fill_rect(r, x + 8, y + 5, 5, 2, light);
}

static void draw_forest_necromancer(Renderer *r, int x, int y) {
    SDL_Color outline = {20, 19, 32, 255};
    SDL_Color robe = {69, 53, 94, 255};
    SDL_Color trim = {132, 103, 145, 255};
    SDL_Color bone = {208, 195, 163, 255};
    SDL_Color magic = {151, 235, 110, 255};
    fill_rect(r, x + 3, y + 2, 3, 9, bone);
    fill_rect(r, x + 18, y + 2, 3, 9, bone);
    fill_rect(r, x + 5, y + 1, 3, 4, bone);
    fill_rect(r, x + 16, y + 1, 3, 4, bone);
    fill_rect(r, x + 5, y + 4, 14, 18, outline);
    fill_rect(r, x + 7, y + 5, 10, 8, robe);
    fill_rect(r, x + 8, y + 8, 8, 6, bone);
    fill_rect(r, x + 9, y + 9, 2, 2, magic);
    fill_rect(r, x + 14, y + 9, 2, 2, magic);
    fill_rect(r, x + 11, y + 12, 3, 2, outline);
    fill_rect(r, x + 6, y + 14, 12, 7, robe);
    fill_rect(r, x + 8, y + 15, 7, 2, trim);
    fill_rect(r, x + 4, y + 19, 5, 4, outline);
    fill_rect(r, x + 16, y + 19, 5, 4, outline);
    fill_rect(r, x + 21, y + 6, 2, 17, bone);
    fill_rect(r, x + 19, y + 2, 5, 5, magic);
    fill_rect(r, x + 20, y + 1, 3, 2, bone);
}

static void draw_forest_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    switch (type) {
        case ENEMY_PIXIE: draw_forest_pixie(r, x, y); break;
        case ENEMY_BLIGHTED_WOLF: draw_forest_wolf(r, x, y); break;
        case ENEMY_GIANT_SPIDER: draw_forest_spider(r, x, y); break;
        case ENEMY_DARK_ELF: draw_forest_elf(r, x, y); break;
        case ENEMY_GIANT_WURM: draw_forest_wurm(r, x, y); break;
        case ENEMY_FOREST_TROLL: draw_forest_troll(r, x, y); break;
        case ENEMY_FOREST_NECROMANCER:
            draw_forest_necromancer(r, x, y);
            break;
        default: break;
    }
}

static void draw_mountain_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int x=tx*TILE_SIZE, y=ty*TILE_SIZE;
    SDL_Color skin={91,121,45,255}, dark={23,17,19,255};
    SDL_Color rust={128,48,30,255}, iron={103,91,88,255};
    SDL_Color ember={244,67,20,255};
    if (type == ENEMY_TUNNEL_SPIDER) {
        fill_rect(r,x+7,y+7,11,12,iron); fill_rect(r,x+9,y+9,7,7,dark);
        for(int i=0;i<4;i++){fill_rect(r,x+1,y+4+i*5,7,2,rust);fill_rect(r,x+17,y+4+i*5,7,2,rust);}
        fill_rect(r,x+10,y+10,2,2,ember); fill_rect(r,x+14,y+10,2,2,ember);
        return;
    }
    if (type == ENEMY_CAVE_TROLL) {
        fill_rect(r,x+4,y+5,16,17,iron); fill_rect(r,x+7,y+8,3,2,ember);
        fill_rect(r,x+15,y+8,3,2,ember); fill_rect(r,x+1,y+10,5,12,dark);
        fill_rect(r,x+19,y+2,4,21,(SDL_Color){85,62,45,255}); return;
    }
    fill_rect(r,x+6,y+5,13,8,skin); fill_rect(r,x+4,y+7,4,3,skin);
    fill_rect(r,x+17,y+7,4,3,skin); fill_rect(r,x+8,y+8,2,2,ember);
    fill_rect(r,x+15,y+8,2,2,ember); fill_rect(r,x+6,y+13,13,9,rust);
    if (type == ENEMY_GOBLIN_ARCHER) {
        fill_rect(r,x+20,y+4,2,18,(SDL_Color){101,67,34,255});
        fill_rect(r,x+18,y+4,5,2,iron); fill_rect(r,x+18,y+20,5,2,iron);
    } else if (type == ENEMY_GOBLIN_BOMBER) {
        fill_rect(r,x+18,y+2,6,6,dark); fill_rect(r,x+20,y,2,3,ember);
    } else if (type == ENEMY_HOBGOBLIN_GUARD) {
        fill_rect(r,x+1,y+8,7,14,iron); fill_rect(r,x+20,y+1,2,22,iron);
    } else if (type == ENEMY_GOBLIN_SHAMAN) {
        fill_rect(r,x+20,y+2,3,21,(SDL_Color){86,51,27,255});
        fill_rect(r,x+18,y,7,6,ember); fill_rect(r,x+5,y+3,15,3,dark);
    } else if (type == ENEMY_MOUNTAIN_GOBLIN_KING) {
        fill_rect(r,x+4,y+12,17,11,dark); fill_rect(r,x+6,y+1,13,5,iron);
        fill_rect(r,x+7,y,3,4,ember); fill_rect(r,x+15,y,3,4,ember);
        fill_rect(r,x+20,y+5,4,18,(SDL_Color){92,58,32,255});
    } else {
        fill_rect(r,x+20,y+5,3,17,iron); fill_rect(r,x+19,y+3,5,5,iron);
    }
}

static void draw_coast_enemy(Renderer *r, int tx, int ty, EnemyType type) {
    int x = tx * TILE_SIZE;
    int y = ty * TILE_SIZE;
    SDL_Color deep = {10, 33, 48, 255};
    SDL_Color aqua = {55, 192, 187, 255};
    SDL_Color pale = {139, 231, 213, 255};
    SDL_Color coral = {214, 78, 70, 255};
    SDL_Color stone = {75, 125, 116, 255};
    if (type == ENEMY_ILLUSION) {
        fill_rect(r, x + 8, y + 4, 9, 7, pale);
        fill_rect(r, x + 6, y + 11, 13, 9, (SDL_Color){91, 81, 181, 255});
        fill_rect(r, x + 4, y + 20, 17, 2, aqua);
    } else if (type == ENEMY_MERFOLK) {
        fill_rect(r, x + 7, y + 4, 11, 9, aqua);
        fill_rect(r, x + 8, y + 13, 9, 7, stone);
        fill_rect(r, x + 4, y + 20, 7, 3, aqua);
        fill_rect(r, x + 15, y + 20, 7, 3, aqua);
        fill_rect(r, x + 20, y + 2, 2, 21, pale);
    } else if (type == ENEMY_SIREN) {
        fill_rect(r, x + 7, y + 4, 11, 8, pale);
        fill_rect(r, x + 5, y + 2, 15, 5, deep);
        fill_rect(r, x + 7, y + 12, 11, 10, (SDL_Color){36, 107, 145, 255});
        fill_rect(r, x + 20, y + 5, 3, 3, aqua);
        fill_rect(r, x + 21, y + 11, 2, 2, aqua);
    } else if (type == ENEMY_GIANT_CRAB) {
        fill_rect(r, x + 5, y + 8, 15, 11, coral);
        fill_rect(r, x, y + 5, 7, 6, coral);
        fill_rect(r, x + 18, y + 5, 6, 6, coral);
        fill_rect(r, x + 2, y + 19, 6, 3, pale);
        fill_rect(r, x + 16, y + 19, 6, 3, pale);
    } else if (type == ENEMY_ANIMATED_STATUE) {
        fill_rect(r, x + 7, y + 3, 11, 8, stone);
        fill_rect(r, x + 5, y + 11, 15, 11, stone);
        fill_rect(r, x + 9, y + 7, 2, 2, pale);
        fill_rect(r, x + 15, y + 7, 2, 2, pale);
        fill_rect(r, x + 12, y + 11, 2, 8, deep);
    } else if (type == ENEMY_WATER_ELEMENTAL) {
        fill_rect(r, x + 8, y + 3, 9, 7, pale);
        fill_rect(r, x + 5, y + 10, 15, 10, aqua);
        fill_rect(r, x + 2, y + 18, 20, 4, (SDL_Color){38, 121, 169, 255});
        fill_rect(r, x + 10, y + 6, 2, 2, deep);
        fill_rect(r, x + 15, y + 6, 2, 2, deep);
    } else if (type == ENEMY_SEA_SERPENT) {
        fill_rect(r, x + 5, y + 15, 17, 7, aqua);
        fill_rect(r, x + 13, y + 5, 8, 13, aqua);
        fill_rect(r, x + 16, y + 3, 7, 7, pale);
        fill_rect(r, x + 20, y + 5, 2, 2, coral);
    } else {
        fill_rect(r, x + 5, y + 9, 15, 13, (SDL_Color){27, 91, 112, 255});
        fill_rect(r, x + 7, y + 4, 11, 8, pale);
        fill_rect(r, x + 6, y + 1, 3, 5, coral);
        fill_rect(r, x + 11, y, 3, 5, coral);
        fill_rect(r, x + 16, y + 1, 3, 5, coral);
        fill_rect(r, x + 21, y + 3, 2, 20, stone);
    }
}

void draw_enemy(Renderer *r, int tile_x, int tile_y, EnemyType type) {
    switch (type) {
        case ENEMY_SKELETON: draw_skeleton(r, tile_x, tile_y); break;
        case ENEMY_GOBLIN:   draw_goblin(r, tile_x, tile_y);   break;
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
        case ENEMY_ORC:      draw_orc(r, tile_x, tile_y);      break;
        case ENEMY_TROLL:    draw_troll(r, tile_x, tile_y);    break;
        case ENEMY_GIANT:    draw_giant(r, tile_x, tile_y);    break;
        case ENEMY_GOBLIN_KING: draw_goblin_king(r, tile_x, tile_y); break;
        case ENEMY_LICH_KING:   draw_lich_king(r, tile_x, tile_y);   break;
        case ENEMY_DEMON_LORD:  draw_demon_lord(r, tile_x, tile_y);  break;
        case ENEMY_RED_DRAGON:  draw_red_dragon(r, tile_x, tile_y);  break;
        case ENEMY_TARRASQUE:   draw_tarrasque(r, tile_x, tile_y);   break;
    }
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
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 3, y, 18, 5, (SDL_Color){38, 23, 18, 255});
    fill_rect(r, x + 5, y + 5, 14, 15, (SDL_Color){92, 50, 27, 255});
    fill_rect(r, x + 8, y + 18, 8, 4, (SDL_Color){151, 46, 34, 255});
    fill_rect(r, x + 16, y + 11, 2, 2, (SDL_Color){235, 186, 70, 255});
}

void draw_tavern_table(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 2, y + 5, 20, 14, (SDL_Color){48, 27, 18, 255});
    fill_rect(r, x + 3, y + 3, 18, 13, (SDL_Color){119, 67, 32, 255});
    fill_rect(r, x + 4, y + 4, 16, 3, (SDL_Color){157, 91, 42, 255});
    fill_rect(r, x + 7, y + 16, 3, 6, (SDL_Color){59, 34, 22, 255});
    fill_rect(r, x + 15, y + 16, 3, 6, (SDL_Color){59, 34, 22, 255});
}

void draw_elowen(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 6, y + 3, 12, 7, (SDL_Color){225, 216, 190, 255});
    fill_rect(r, x + 8, y + 7, 8, 6, (SDL_Color){202, 154, 119, 255});
    fill_rect(r, x + 6, y + 12, 12, 10, (SDL_Color){48, 76, 132, 255});
    fill_rect(r, x + 4, y + 13, 3, 8, (SDL_Color){67, 101, 165, 255});
    fill_rect(r, x + 17, y + 13, 3, 8, (SDL_Color){67, 101, 165, 255});
    fill_rect(r, x + 10, y + 9, 2, 2, (SDL_Color){44, 37, 51, 255});
    fill_rect(r, x + 14, y + 9, 2, 2, (SDL_Color){44, 37, 51, 255});
    fill_rect(r, x + 20, y + 4, 2, 18, (SDL_Color){94, 61, 38, 255});
    fill_rect(r, x + 18, y + 3, 6, 3, (SDL_Color){112, 194, 210, 255});
}

void draw_dain(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    fill_rect(r, x + 5, y + 4, 14, 5, (SDL_Color){72, 67, 62, 255});
    fill_rect(r, x + 7, y + 7, 10, 7, (SDL_Color){189, 132, 91, 255});
    fill_rect(r, x + 6, y + 11, 12, 8, (SDL_Color){116, 67, 35, 255});
    fill_rect(r, x + 8, y + 12, 2, 7, (SDL_Color){173, 105, 50, 255});
    fill_rect(r, x + 14, y + 12, 2, 7, (SDL_Color){173, 105, 50, 255});
    fill_rect(r, x + 5, y + 17, 14, 5, (SDL_Color){65, 77, 91, 255});
    fill_rect(r, x + 9, y + 9, 2, 2, (SDL_Color){35, 29, 27, 255});
    fill_rect(r, x + 14, y + 9, 2, 2, (SDL_Color){35, 29, 27, 255});
    fill_rect(r, x + 2, y + 5, 3, 17, (SDL_Color){87, 55, 34, 255});
    fill_rect(r, x, y + 3, 7, 4, (SDL_Color){151, 157, 164, 255});
}

void draw_alder(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    SDL_Color cloak = {48, 91, 55, 255};
    SDL_Color cloak_light = {76, 126, 69, 255};
    SDL_Color leather = {91, 57, 32, 255};
    SDL_Color skin = {191, 142, 101, 255};
    fill_rect(r, x + 7, y + 3, 10, 8, cloak);
    fill_rect(r, x + 9, y + 6, 7, 6, skin);
    fill_rect(r, x + 6, y + 11, 13, 11, cloak);
    fill_rect(r, x + 4, y + 13, 4, 8, cloak_light);
    fill_rect(r, x + 17, y + 13, 4, 8, cloak_light);
    fill_rect(r, x + 8, y + 16, 10, 3, leather);
    fill_rect(r, x + 10, y + 8, 2, 2, (SDL_Color){30, 39, 27, 255});
    fill_rect(r, x + 15, y + 8, 2, 2, (SDL_Color){30, 39, 27, 255});
    fill_rect(r, x + 21, y + 3, 2, 19, leather);
    fill_rect(r, x + 19, y + 2, 5, 2, (SDL_Color){154, 177, 118, 255});
}

void draw_mara(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_tavern_floor(r, tile_x, tile_y);
    SDL_Color coat = {29, 91, 111, 255};
    SDL_Color coat_light = {47, 137, 147, 255};
    SDL_Color skin = {188, 137, 101, 255};
    SDL_Color brass = {207, 164, 63, 255};
    fill_rect(r, x + 6, y + 4, 13, 5, (SDL_Color){30, 54, 66, 255});
    fill_rect(r, x + 8, y + 7, 9, 6, skin);
    fill_rect(r, x + 6, y + 12, 13, 10, coat);
    fill_rect(r, x + 9, y + 13, 3, 8, coat_light);
    fill_rect(r, x + 10, y + 9, 2, 2, (SDL_Color){25, 35, 42, 255});
    fill_rect(r, x + 15, y + 9, 2, 2, (SDL_Color){25, 35, 42, 255});
    fill_rect(r, x + 20, y + 12, 3, 10, brass);
    fill_rect(r, x + 18, y + 15, 6, 6, (SDL_Color){69, 207, 196, 255});
}

void draw_forest_warden(Renderer *r, int tile_x, int tile_y, int map_x, int map_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    draw_forest_floor(r, tile_x, tile_y, map_x, map_y);
    SDL_Color cloak = {41, 84, 49, 255};
    SDL_Color skin = {199, 151, 108, 255};
    SDL_Color binding = {157, 174, 132, 255};
    SDL_Color root = {80, 57, 31, 255};
    fill_rect(r, x + 7, y + 3, 10, 8, cloak);
    fill_rect(r, x + 9, y + 6, 7, 5, skin);
    fill_rect(r, x + 6, y + 11, 13, 11, cloak);
    fill_rect(r, x + 3, y + 12, 19, 3, binding);
    fill_rect(r, x + 4, y + 17, 17, 3, binding);
    fill_rect(r, x + 2, y + 7, 3, 15, root);
    fill_rect(r, x + 20, y + 5, 3, 17, root);
    fill_rect(r, x + 9, y + 8, 2, 2, (SDL_Color){210, 226, 174, 255});
    fill_rect(r, x + 15, y + 8, 2, 2, (SDL_Color){210, 226, 174, 255});
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

void draw_town_gate(Renderer *r, int tile_x, int tile_y, TownExitStyle style) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    if (style == TOWN_EXIT_DUNGEON) {
        SDL_Color stone = {70, 66, 88, 255};
        SDL_Color rim = {111, 102, 132, 255};
        SDL_Color seam = {28, 27, 44, 255};
        fill_rect(r, x, y, 120, 15, stone);
        fill_rect(r, x, y, 120, 3, rim);
        fill_rect(r, x, y + 13, 120, 3, seam);
        for (int block = 1; block < 6; block++) {
            fill_rect(r, x + block * 20, y + 3, 2, 10, seam);
        }
        fill_rect(r, x, y, 17, 48, stone);
        fill_rect(r, x + 103, y, 17, 48, stone);
        fill_rect(r, x + 2, y + 2, 3, 43, rim);
        fill_rect(r, x + 105, y + 2, 3, 43, rim);
        fill_rect(r, x + 17, y + 15, 15, 9, stone);
        fill_rect(r, x + 88, y + 15, 15, 9, stone);
        fill_rect(r, x + 50, y + 8, 20, 14, rim);
        fill_rect(r, x + 56, y + 11, 8, 8,
            (SDL_Color){191, 152, 64, 255});
        fill_rect(r, x + 59, y + 12, 2, 6, seam);
        fill_rect(r, x + 55, y + 15, 10, 2, seam);
        return;
    }
    if (style == TOWN_EXIT_FOREST) {
        SDL_Color trunk = {83, 57, 33, 255};
        SDL_Color bark = {121, 82, 43, 255};
        SDL_Color leaf = {31, 83, 40, 255};
        fill_rect(r, x + 1, y, 13, 120, trunk);
        fill_rect(r, x + 4, y + 3, 3, 114, bark);
        fill_rect(r, x + 10, y + 1, 28, 11, leaf);
        fill_rect(r, x + 18, y + 7, 30, 11, leaf);
        fill_rect(r, x + 10, y + 108, 28, 11, leaf);
        fill_rect(r, x + 18, y + 102, 30, 11, leaf);
        fill_rect(r, x + 24, y + 3, 13, 3,
            (SDL_Color){53, 113, 51, 255});
        fill_rect(r, x + 33, y + 104, 10, 3,
            (SDL_Color){53, 113, 51, 255});
        fill_rect(r, x + 27, y + 16, 3, 12,
            (SDL_Color){24, 67, 34, 255});
        fill_rect(r, x + 38, y + 97, 2, 9,
            (SDL_Color){24, 67, 34, 255});
        fill_rect(r, x + 13, y + 17, 8, 8,
            (SDL_Color){22, 62, 32, 255});
        fill_rect(r, x + 14, y + 95, 9, 8,
            (SDL_Color){22, 62, 32, 255});
        fill_rect(r, x + 10, y + 30, 6, 4, leaf);
        fill_rect(r, x + 9, y + 77, 7, 5, leaf);
        for (int mark = 0; mark < 4; mark++) {
            fill_rect(r, x + 9, y + 18 + mark * 23, 3, 2,
                (SDL_Color){50, 38, 28, 255});
        }
        fill_rect(r, x + 12, y + 57, 5, 3,
            (SDL_Color){77, 151, 69, 255});
        return;
    }
    if (style == TOWN_EXIT_MOUNTAINS) {
        SDL_Color basalt = {53, 45, 50, 255};
        SDL_Color ridge = {99, 69, 65, 255};
        SDL_Color shadow = {29, 26, 33, 255};
        fill_rect(r, x + 55, y, 17, 120, basalt);
        fill_rect(r, x + 56, y + 2, 3, 116, ridge);
        fill_rect(r, x + 10, y, 48, 12, basalt);
        fill_rect(r, x + 27, y + 11, 31, 9, basalt);
        fill_rect(r, x + 44, y + 19, 14, 7, basalt);
        fill_rect(r, x + 10, y + 108, 48, 12, basalt);
        fill_rect(r, x + 27, y + 100, 31, 9, basalt);
        fill_rect(r, x + 44, y + 94, 14, 7, basalt);
        fill_rect(r, x + 12, y + 1, 44, 3, ridge);
        fill_rect(r, x + 12, y + 109, 44, 3, ridge);
        fill_rect(r, x + 44, y + 16, 14, 10, basalt);
        fill_rect(r, x + 44, y + 94, 14, 10, basalt);
        for (int layer = 0; layer < 5; layer++) {
            int layer_y = y + layer * 24;
            fill_rect(r, x + 59, layer_y + 3, 11, 9,
                layer % 2 == 0 ? (SDL_Color){69, 54, 56, 255} :
                (SDL_Color){62, 50, 53, 255});
            fill_rect(r, x + 60, layer_y + 3, 8, 2, ridge);
            fill_rect(r, x + 58, layer_y + 21, 12, 2, shadow);
        }
        fill_rect(r, x + 59, y + 56, 5, 8,
            (SDL_Color){213, 86, 46, 255});
        return;
    }

    SDL_Color stone = {43, 84, 83, 255};
    SDL_Color rim = {97, 144, 124, 255};
    fill_rect(r, x, y, 120, 14, stone);
    fill_rect(r, x, y, 120, 3, rim);
    for (int block = 1; block < 6; block++) {
        fill_rect(r, x + block * 20, y + 3, 2, 10,
            (SDL_Color){24, 59, 67, 255});
    }
    fill_rect(r, x, y, 17, 48, stone);
    fill_rect(r, x + 103, y, 17, 48, stone);
    fill_rect(r, x + 2, y + 2, 3, 43, rim);
    fill_rect(r, x + 105, y + 2, 3, 43, rim);
    for (int row = 1; row < 4; row++) {
        fill_rect(r, x + 4, y + row * 11, 11, 2,
            (SDL_Color){23, 58, 64, 255});
        fill_rect(r, x + 105, y + row * 11, 11, 2,
            (SDL_Color){23, 58, 64, 255});
    }
    fill_rect(r, x + 17, y + 12, 14, 10, stone);
    fill_rect(r, x + 89, y + 12, 14, 10, stone);
    fill_rect(r, x + 25, y + 30, 70, 5,
        (SDL_Color){81, 79, 66, 255});
    fill_rect(r, x + 25, y + 30, 70, 2, rim);
    fill_rect(r, x + 37, y + 35, 7, 4,
        (SDL_Color){21, 91, 91, 255});
    fill_rect(r, x + 77, y + 35, 8, 4,
        (SDL_Color){21, 91, 91, 255});
    fill_rect(r, x + 56, y + 5, 8, 6,
        (SDL_Color){178, 146, 73, 255});
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

void draw_harbor(Renderer *r, int tile_x, int tile_y) {
    if (!r->harbor_texture) {
        return;
    }
    // Exclude the sample's transparent margins and preserve its proportions.
    SDL_Rect source = {150, 174, 1129, 827};
    int width = TOWN_HARBOR_W * TILE_SIZE;
    int height = width * source.h / source.w;
    int bottom = (tile_y + TOWN_HARBOR_H) * TILE_SIZE;
    SDL_Rect destination = {tile_x * TILE_SIZE, bottom - height, width, height};
    SDL_RenderCopy(r->sdl, r->harbor_texture, &source, &destination);
}

void draw_floor_item(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color glow = {100, 180, 255, 255};
    fill_rect(r, x+8, y+6, 8,           10,          glow);
    fill_rect(r, x+6, y+8, 12,          6,           glow);
}

void draw_floor_gold(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color base = { 18,  18,  35, 255};
    SDL_Color gold = {220, 180,  40, 255};
    fill_rect(r, x,   y,   TILE_SIZE, TILE_SIZE, base);
    fill_rect(r, x+7, y+7, 10,        10,        gold);
    fill_rect(r, x+9, y+9, 6,         6,         gold);
}

void draw_trap_warning(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color edge = {116, 108, 132, 255};
    SDL_Color rune = {190, 152, 72, 255};
    fill_rect(r, x+4, y+4, 16, 2, edge);
    fill_rect(r, x+4, y+18, 16, 2, edge);
    fill_rect(r, x+4, y+6, 2, 12, edge);
    fill_rect(r, x+18, y+6, 2, 12, edge);
    fill_rect(r, x+11, y+8, 2, 7, rune);
    fill_rect(r, x+11, y+17, 2, 2, rune);
}

void draw_trap_spike(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color rim = {64, 62, 82, 255};
    SDL_Color recess = {28, 27, 42, 255};
    SDL_Color steel = {152, 158, 170, 255};
    SDL_Color shine = {214, 218, 220, 255};
    fill_rect(r, x+3, y+4, 18, 17, rim);
    fill_rect(r, x+5, y+6, 14, 13, recess);
    fill_rect(r, x+4, y+19, 16, 2, (SDL_Color){38, 36, 54, 255});

    fill_rect(r, x+6,  y+12, 3, 7, steel);
    fill_rect(r, x+7,  y+8,  1, 4, shine);
    fill_rect(r, x+11, y+9,  3, 10, steel);
    fill_rect(r, x+12, y+5,  1, 4, shine);
    fill_rect(r, x+16, y+12, 3, 7, steel);
    fill_rect(r, x+17, y+8,  1, 4, shine);
    fill_rect(r, x+7,  y+7,  1, 1, shine);
    fill_rect(r, x+12, y+4,  1, 1, shine);
    fill_rect(r, x+17, y+7,  1, 1, shine);
}

void draw_trap_fire(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color iron = {58, 54, 66, 255};
    SDL_Color soot = {34, 25, 30, 255};
    SDL_Color red = {174, 42, 20, 255};
    SDL_Color orange = {244, 98, 20, 255};
    SDL_Color yellow = {255, 204, 62, 255};
    fill_rect(r, x+3, y+15, 18, 6, iron);
    fill_rect(r, x+5, y+17, 14, 3, soot);
    fill_rect(r, x+5, y+14, 14, 2, (SDL_Color){92, 76, 70, 255});
    fill_rect(r, x+7, y+10, 11, 7, red);
    fill_rect(r, x+9, y+6, 7, 11, orange);
    fill_rect(r, x+12,y+3, 3, 8, orange);
    fill_rect(r, x+7, y+8, 3, 7, red);
    fill_rect(r, x+11,y+10,4, 7, yellow);
    fill_rect(r, x+12,y+7, 2, 5, (SDL_Color){255, 238, 150, 255});
    fill_rect(r, x+6, y+4, 2, 2, (SDL_Color){116, 52, 34, 255});
    fill_rect(r, x+18,y+7, 2, 2, (SDL_Color){206, 70, 24, 255});
}

void draw_trap_poison(Renderer *r, int tile_x, int tile_y) {
    int x = tile_x * TILE_SIZE;
    int y = tile_y * TILE_SIZE;
    SDL_Color rim = {54, 62, 70, 255};
    SDL_Color recess = {24, 34, 34, 255};
    SDL_Color toxic = {54, 174, 70, 255};
    SDL_Color glow = {122, 236, 92, 255};
    fill_rect(r, x+3, y+4, 18, 17, rim);
    fill_rect(r, x+5, y+6, 14, 13, recess);
    fill_rect(r, x+6, y+7, 3, 3, toxic);
    fill_rect(r, x+15,y+7, 3, 3, toxic);
    fill_rect(r, x+8, y+9, 8, 7, toxic);
    fill_rect(r, x+9, y+10,2, 2, glow);
    fill_rect(r, x+14,y+10,2, 2, glow);
    fill_rect(r, x+11,y+13,3, 2, recess);
    fill_rect(r, x+8, y+16,3, 3, toxic);
    fill_rect(r, x+12,y+16,2, 3, toxic);
    fill_rect(r, x+15,y+16,3, 3, toxic);
    fill_rect(r, x+4, y+2, 2, 2, glow);
    fill_rect(r, x+18,y+1, 2, 2, toxic);
    fill_rect(r, x+20,y+5, 1, 1, glow);
}
