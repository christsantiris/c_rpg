#include "map.h"
#include <stdlib.h>

static void fill_rect(Map *m, int x, int y, int w, int h, TileType t) {
    for (int ry = y; ry < y + h; ry++)
        for (int rx = x; rx < x + w; rx++)
            if (rx >= 0 && rx < MAP_W && ry >= 0 && ry < MAP_H)
                m->tiles[ry][rx] = t;
}

static void carve_corridor(Map *m, int x1, int y1, int x2, int y2) {
    // Horizontal then vertical L-shaped corridor
    int x = x1;
    while (x != x2) {
        if (x >= 0 && x < MAP_W && y1 >= 0 && y1 < MAP_H)
            m->tiles[y1][x] = TILE_FLOOR;
        x += (x2 > x1) ? 1 : -1;
    }
    int y = y1;
    while (y != y2) {
        if (x2 >= 0 && x2 < MAP_W && y >= 0 && y < MAP_H)
            m->tiles[y][x2] = TILE_FLOOR;
        y += (y2 > y1) ? 1 : -1;
    }
    m->tiles[y2][x2] = TILE_FLOOR;
}

static int rooms_overlap(const Room *a, const Room *b) {
    return !(a->x + a->w + 1 < b->x ||
             b->x + b->w + 1 < a->x ||
             a->y + a->h + 1 < b->y ||
             b->y + b->h + 1 < a->y);
}

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

void map_room_center(const Room *r, int *cx, int *cy) {
    *cx = r->x + r->w / 2;
    *cy = r->y + r->h / 2;
}

void map_generate(Map *m, int level) {
    (void)level;

    // Fill with walls
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = TILE_WALL;

    m->room_count = 0;

    int target_rooms = random_range(MIN_ROOMS, MAX_ROOMS);
    int attempts = 0;

    while (m->room_count < target_rooms && attempts < 200) {
        attempts++;

        Room r;
        r.w = random_range(MIN_ROOM_W, MAX_ROOM_W);
        r.h = random_range(MIN_ROOM_H, MAX_ROOM_H);
        r.x = random_range(1, MAP_W / 2); // hallway size
        r.y = random_range(1, MAP_H / 2); // hallway size
        // r.x = random_range(1, MAP_W - r.w - 2);
        // r.y = random_range(1, MAP_H - r.h - 2);

        // Check overlap with existing rooms
        int overlaps = 0;
        for (int i = 0; i < m->room_count; i++) {
            if (rooms_overlap(&r, &m->rooms[i])) {
                overlaps = 1;
                break;
            }
        }
        if (overlaps) continue;

        // Carve room
        fill_rect(m, r.x, r.y, r.w, r.h, TILE_FLOOR);

        // Connect to previous room
        if (m->room_count > 0) {
            int cx1, cy1, cx2, cy2;
            map_room_center(&m->rooms[m->room_count - 1], &cx1, &cy1);
            map_room_center(&r, &cx2, &cy2);
            carve_corridor(m, cx1, cy1, cx2, cy2);
        }

        m->rooms[m->room_count++] = r;
    }

    // Place stairs up in first room
    int ux, uy;
    map_room_center(&m->rooms[0], &ux, &uy);
    m->stairs_up_x = ux;
    m->stairs_up_y = uy;
    m->tiles[uy][ux] = TILE_STAIRS_UP;

    // Place stairs down in last room, not on stairs up
    int dx, dy;
    map_room_center(&m->rooms[m->room_count - 1], &dx, &dy);
    if (dx == ux && dy == uy) dx++;
    m->stairs_down_x = dx;
    m->stairs_down_y = dy;
    m->tiles[dy][dx] = TILE_STAIRS_DOWN;
}

int map_is_walkable(const Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
    return m->tiles[y][x] != TILE_WALL;
}

void map_generate_town(Map *m, int *spawn_x, int *spawn_y) {
    m->room_count = 0;

    // Fill with walls
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = TILE_WALL;

    // Town floor
    for (int y = 1; y < TOWN_H - 1; y++)
        for (int x = 1; x < TOWN_W - 1; x++)
            m->tiles[y][x] = TILE_TOWN_FLOOR;

    // Vertical path (center column)
    for (int y = 1; y < TOWN_H - 1; y++)
        m->tiles[y][20] = TILE_TOWN_PATH;

    // Horizontal path (mid row)
    for (int x = 1; x < TOWN_W - 1; x++)
        m->tiles[12][x] = TILE_TOWN_PATH;

    // Exit at north edge
    for (int x = 18; x <= 22; x++)
        m->tiles[0][x] = TILE_TOWN_EXIT;

    // Blacksmith at (7, 7) — 5x4 tiles
    for (int dy = 0; dy < 4; dy++)
        for (int dx = 0; dx < 5; dx++)
            m->tiles[7 + dy][7 + dx] = TILE_SHOP_BLACKSMITH;

    // Alchemist at (28, 7) — 5x4 tiles
    for (int dy = 0; dy < 4; dy++)
        for (int dx = 0; dx < 5; dx++)
            m->tiles[7 + dy][28 + dx] = TILE_SHOP_ALCHEMIST;

    // Spawn at south end of vertical path
    *spawn_x = 20;
    *spawn_y = TOWN_H - 2;
}