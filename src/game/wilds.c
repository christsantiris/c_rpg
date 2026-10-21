#include "wilds.h"
#include <stdlib.h>
#include <string.h>

static int boulder(TileType tile) {
    return tile == TILE_DRAGON_BOULDER || tile == TILE_DRAGON_ASH_BOULDER;
}

int wilds_tile(TileType tile) {
    return tile >= TILE_SWAMP_WATER && tile <= TILE_DRAGON_ASH_BOULDER;
}

static TileType terrain(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_WALL;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_PORTAL && g->portal_active && g->portal_location == g->location &&
        g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                return g->floor_items[i].underlying_tile;
            }
        }
    }
    return tile;
}

static void set_terrain(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_PORTAL && g->portal_active && g->portal_location == g->location && g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        g->portal_origin_tile = tile;
    } else if (g->map.tiles[y][x] != TILE_ITEM) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
            g->floor_items[i].underlying_tile = tile;
        }
    }
}

static void init_map(Map *m, TileType wall) {
    memset(m, 0, sizeof(*m));
    m->room_count = 9;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = wall;
        }
    }
}

static void brush(Map *m, int cx, int cy, int radius, TileType wall, TileType floor) {
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x > 0 && y > 0 && x < SWAMP_MAP_W - 1 && y < SWAMP_MAP_H - 1 && m->tiles[y][x] == wall) {
                m->tiles[y][x] = floor;
            }
        }
    }
}

static void path(Map *m, int ax, int ay, int bx, int by, TileType wall, TileType floor) {
    int dx = abs(bx - ax);
    int dy = -abs(by - ay);
    int sx = ax < bx ? 1 : -1;
    int sy = ay < by ? 1 : -1;
    int error = dx + dy;
    while (1) {
        brush(m, ax, ay, 2, wall, floor);
        if (ax == bx && ay == by) {
            break;
        }
        int twice = error * 2;
        if (twice >= dy) {
            error += dy;
            ax += sx;
        }
        if (twice <= dx) {
            error += dx;
            ay += sy;
        }
    }
}

static void clearing(Map *m, int i, int cx, int cy, TileType floor) {
    int w = i == 8 ? 14 : 12 + rand() % 4;
    int h = i == 8 ? 12 : 9 + rand() % 3;
    Room *room = &m->rooms[i];
    *room = (Room){cx - w / 2, cy - h / 2, w, h};
    for (int y = room->y; y < room->y + h; y++) {
        for (int x = room->x; x < room->x + w; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (i == 8 || dx * dx * h * h + dy * dy * w * w < w * w * h * h / 4) {
                m->tiles[y][x] = floor;
            }
        }
    }
}

static void link(Map *m, int a, int b, TileType wall, TileType floor) {
    int ax, ay, bx, by;
    map_room_center(&m->rooms[a], &ax, &ay);
    map_room_center(&m->rooms[b], &bx, &by);
    path(m, ax, ay, bx, by, wall, floor);
}

static void edges(Map *m, TileType wall, TileType floor, TileType entrance, TileType exit) {
    int ax, ay, bx, by;
    map_room_center(&m->rooms[0], &ax, &ay);
    map_room_center(&m->rooms[8], &bx, &by);
    path(m, ax, ay, 1, ay, wall, floor);
    path(m, bx, by, SWAMP_MAP_W - 2, by, wall, floor);
    if (wall == TILE_SWAMP_WATER) {
        path(m, ax, ay, 1, ay, TILE_SWAMP_WALL, floor);
        path(m, bx, by, SWAMP_MAP_W - 2, by, TILE_SWAMP_WALL, floor);
    }
    m->stairs_up_x = 0;
    m->stairs_up_y = ay;
    m->stairs_down_x = SWAMP_MAP_W - 1;
    m->stairs_down_y = by;
    m->tiles[ay][0] = entrance;
    m->tiles[by][SWAMP_MAP_W - 1] = exit;
}

static void reeds(Map *m) {
    const int islands[3] = {1, 4, 6};
    for (int i = 0; i < 3; i++) {
        int x, y;
        map_room_center(&m->rooms[islands[i]], &x, &y);
        for (int dy = 1; dy <= 3; dy++) {
            for (int dx = -3; dx <= 2; dx++) {
                if (m->tiles[y + dy][x + dx] == TILE_SWAMP_FLOOR) {
                    m->tiles[y + dy][x + dx] = TILE_SWAMP_REEDS;
                }
            }
        }
    }
}

void map_generate_swamp(Map *m, int level) {
    // Bank loop, open lagoon, branching islets, and winding southern channel.
    static const int centers[4][9][2] = {
        {{9, 32}, {21, 13}, {41, 10}, {59, 20}, {36, 30}, {53, 49}, {30, 53}, {15, 47}, {61, 36}},
        {{9, 16}, {27, 10}, {48, 11}, {58, 24}, {29, 32}, {57, 51}, {37, 53}, {12, 48}, {61, 37}},
        {{9, 46}, {14, 19}, {35, 12}, {57, 12}, {34, 33}, {55, 51}, {32, 53}, {13, 34}, {61, 33}},
        {{9, 13}, {28, 11}, {49, 11}, {23, 31}, {47, 32}, {53, 52}, {30, 52}, {11, 46}, {61, 32}}
    };
    // The first eight links connect every clearing; the last two add loops.
    static const int links[4][10][2] = {
        {{0, 1}, {1, 2}, {2, 3}, {3, 8}, {0, 7}, {7, 6}, {6, 5}, {5, 4}, {4, 1}, {5, 8}},
        {{0, 1}, {1, 2}, {2, 3}, {3, 8}, {0, 4}, {4, 7}, {7, 6}, {6, 5}, {5, 8}, {4, 6}},
        {{0, 7}, {7, 1}, {1, 2}, {2, 4}, {4, 3}, {3, 8}, {4, 6}, {6, 5}, {5, 8}, {7, 4}},
        {{0, 1}, {1, 2}, {2, 4}, {4, 3}, {3, 7}, {7, 6}, {6, 5}, {5, 8}, {0, 3}, {4, 8}}
    };
    int layout = (level - 1) % 4;
    int flip = ((level - 1) / 4 + rand() % 2) % 2;
    init_map(m, TILE_SWAMP_WALL);
    for (int y = 1; y < SWAMP_MAP_H - 1; y++) {
        for (int x = 1; x < SWAMP_MAP_W - 1; x++) {
            m->tiles[y][x] = TILE_SWAMP_WATER;
        }
    }
    for (int i = 0; i < 9; i++) {
        int x = centers[layout][i][0];
        int y = centers[layout][i][1] + rand() % 7 - 3;
        if (i != 0 && i != 8) {
            x += rand() % 7 - 3;
        }
        if (flip) {
            y = SWAMP_MAP_H - 1 - y;
        }
        clearing(m, i, x, y, TILE_SWAMP_FLOOR);
    }
    for (int i = 0; i < 10; i++) {
        if (i < 8 || rand() % 2) {
            link(m, links[layout][i][0], links[layout][i][1], TILE_SWAMP_WATER, TILE_SWAMP_FLOOR);
        }
    }
    // Rounded cypress islets grow along the far banks of the blackwater.
    for (int by = 8; by <= 56; by += 16) {
        for (int bx = 9; bx <= 63; bx += 18) {
            int cx = bx + rand() % 5 - 2;
            int cy = by + rand() % 5 - 2;
            int rx = 3 + (level + rand() % 3) % 3;
            int ry = 3 + rand() % 3;
            for (int dy = -ry; dy <= ry; dy++) {
                for (int dx = -rx; dx <= rx; dx++) {
                    int x = cx + dx;
                    int y = cy + dy;
                    if (x > 1 && y > 1 && x < SWAMP_MAP_W - 2 && y < SWAMP_MAP_H - 2 &&
                        dx * dx * ry * ry + dy * dy * rx * rx <= rx * rx * ry * ry && m->tiles[y][x] == TILE_SWAMP_WATER) {
                        m->tiles[y][x] = TILE_SWAMP_WALL;
                    }
                }
            }
        }
    }
    edges(m, TILE_SWAMP_WATER, TILE_SWAMP_FLOOR, TILE_SWAMP_ENTRANCE, TILE_SWAMP_EXIT);
    reeds(m);
}

static int connected(const Map *m, int x, int y) {
    unsigned char seen[SWAMP_MAP_H][SWAMP_MAP_W] = {{0}};
    int queue[SWAMP_MAP_W * SWAMP_MAP_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = y * SWAMP_MAP_W + x;
    seen[y][x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int nx = cell % SWAMP_MAP_W + offsets[i][0];
            int ny = cell / SWAMP_MAP_W + offsets[i][1];
            if (nx < 0 || ny < 0 || nx >= SWAMP_MAP_W || ny >= SWAMP_MAP_H || seen[ny][nx] || !map_is_walkable(m, nx, ny)) {
                continue;
            }
            seen[ny][nx] = 1;
            queue[tail++] = ny * SWAMP_MAP_W + nx;
        }
    }
    for (int ty = 0; ty < SWAMP_MAP_H; ty++) {
        for (int tx = 0; tx < SWAMP_MAP_W; tx++) {
            if (map_is_walkable(m, tx, ty) && !seen[ty][tx]) {
                return 0;
            }
        }
    }
    return 1;
}

static void place_boulders(Map *m) {
    for (int i = 1; i < m->room_count - 1; i += 2) {
        int x, y;
        map_room_center(&m->rooms[i], &x, &y);
        x += 2;
        if (m->tiles[y][x] != TILE_DRAGON_FLOOR && m->tiles[y][x] != TILE_DRAGON_ASH) {
            continue;
        }
        TileType ground = m->tiles[y][x];
        m->tiles[y][x] = ground == TILE_DRAGON_ASH ? TILE_DRAGON_ASH_BOULDER : TILE_DRAGON_BOULDER;
        if (!connected(m, m->stairs_up_x, m->stairs_up_y)) {
            m->tiles[y][x] = ground;
        }
    }
}

void map_generate_dragonspine(Map *m, int level) {
    static const int centers[9][2] = {{9, 51}, {31, 54}, {55, 45}, {59, 25}, {10, 29}, {22, 11}, {43, 10}, {37, 30}, {59, 9}};
    static const int links[][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 8}, {0, 4}, {4, 5}, {5, 6}, {6, 8}, {4, 7}, {7, 3}, {1, 7}};
    init_map(m, TILE_DRAGON_WALL);
    for (int i = 0; i < 9; i++) {
        int jitter = i == 0 || i == 8 ? 0 : rand() % 3 - 1;
        clearing(m, i, centers[i][0] + jitter, centers[i][1], TILE_DRAGON_FLOOR);
    }
    for (int i = 0; i < (int)(sizeof(links) / sizeof(links[0])); i++) {
        link(m, links[i][0], links[i][1], TILE_DRAGON_WALL, TILE_DRAGON_FLOOR);
    }
    for (int y = 2; y < SWAMP_MAP_H - 2; y++) {
        for (int x = 2; x < SWAMP_MAP_W - 2; x++) {
            int dx = x - 34;
            int dy = y - 32;
            if (m->tiles[y][x] == TILE_DRAGON_WALL && dx * dx * 169 + dy * dy * 256 < 43264) {
                m->tiles[y][x] = TILE_DRAGON_RAVINE;
            }
            if (level >= 3 && m->tiles[y][x] == TILE_DRAGON_FLOOR && (x * 3 + y + level) % 5 < level - 2) {
                m->tiles[y][x] = TILE_DRAGON_ASH;
            }
        }
    }
    if (level == DRAGONSPINE_DEPTH) {
        const Room *last = &m->rooms[8];
        for (int y = last->y + 1; y < last->y + last->h - 1; y++) {
            for (int x = last->x + 1; x < last->x + last->w - 1; x++) {
                if ((x + y) % 3 == 0) {
                    m->tiles[y][x] = TILE_DRAGON_HOARD;
                }
            }
        }
    }
    edges(m, TILE_DRAGON_WALL, TILE_DRAGON_FLOOR, TILE_DRAGON_ENTRANCE, TILE_DRAGON_EXIT);
    place_boulders(m);
}

static int find_boulder(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_DRAGONSPINE) {
        return 0;
    }
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 4; i++) {
        int tx = g->player.x + offsets[i][0];
        int ty = g->player.y + offsets[i][1];
        if (boulder(terrain(g, tx, ty))) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

int wilds_has_interaction(const GameState *g) {
    int x, y;
    return find_boulder(g, &x, &y);
}

int wilds_interact(GameState *g) {
    int x, y;
    if (!find_boulder(g, &x, &y)) {
        return 0;
    }
    int tx = x + x - g->player.x;
    int ty = y + y - g->player.y;
    TileType to = terrain(g, tx, ty);
    int blocked = to != TILE_DRAGON_FLOOR && to != TILE_DRAGON_ASH;
    if (g->portal_active && g->portal_location == g->location && g->portal_level == g->level && g->portal_x == tx && g->portal_y == ty) {
        blocked = 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        blocked |= g->enemies[i].active && g->enemies[i].x == tx && g->enemies[i].y == ty;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        blocked |= g->floor_items[i].active && g->floor_items[i].x == tx && g->floor_items[i].y == ty;
    }
    if (g->map.room_count) {
        const Room *last = &g->map.rooms[g->map.room_count - 1];
        blocked |= tx >= last->x && ty >= last->y && tx < last->x + last->w && ty < last->y + last->h;
    }
    if (blocked) {
        push_message(g, "The boulder needs empty snow or ash beyond it, outside the final clearing.");
        return 1;
    }
    TileType from = terrain(g, x, y);
    set_terrain(g, x, y, from == TILE_DRAGON_ASH_BOULDER ? TILE_DRAGON_ASH : TILE_DRAGON_FLOOR);
    set_terrain(g, tx, ty, to == TILE_DRAGON_ASH ? TILE_DRAGON_ASH_BOULDER : TILE_DRAGON_BOULDER);
    if (!connected(&g->map, g->player.x, g->player.y)) {
        set_terrain(g, x, y, from);
        set_terrain(g, tx, ty, to);
        push_message(g, "That push would seal a route. The boulder stays put.");
        return 1;
    }
    push_message(g, "You push the boulder. Its new position blocks movement and enemy fire.");
    return 1;
}

int wilds_conceals(const GameState *g, const Enemy *e) {
    return g->location == LOCATION_SWAMP && terrain(g, g->player.x, g->player.y) == TILE_SWAMP_REEDS &&
        !e->is_boss && e->hp == e->max_hp && abs(e->x - g->player.x) + abs(e->y - g->player.y) > 3;
}

static void migrate_boulders(Map *m, const Map *old) {
    for (int i = 1; i < 8; i += 2) {
        int cx, cy;
        map_room_center(&m->rooms[i], &cx, &cy);
        int placed = 0;
        for (int radius = 2; radius <= 6 && !placed; radius++) {
            for (int y = cy - radius; y <= cy + radius && !placed; y++) {
                for (int x = cx - radius; x <= cx + radius && !placed; x++) {
                    if (x <= 1 || y <= 1 || x >= SWAMP_MAP_W - 2 || y >= SWAMP_MAP_H - 2 ||
                        old->tiles[y][x] != TILE_DRAGON_WALL || m->tiles[y][x] != TILE_DRAGON_FLOOR ||
                        !map_is_walkable(m, x - 1, y) || !map_is_walkable(m, x + 1, y) ||
                        !map_is_walkable(m, x, y - 1) || !map_is_walkable(m, x, y + 1)) {
                        continue;
                    }
                    m->tiles[y][x] = TILE_DRAGON_BOULDER;
                    if (connected(m, m->stairs_up_x, m->stairs_up_y)) {
                        placed = 1;
                    } else {
                        m->tiles[y][x] = TILE_DRAGON_FLOOR;
                    }
                }
            }
        }
    }
}

static void migrate_map(Map *m, int dragon) {
    if (m->room_count != 9) {
        return;
    }
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            if (wilds_tile(m->tiles[y][x])) {
                return;
            }
        }
    }
    Map original = *m;
    TileType wall = dragon ? TILE_DRAGON_WALL : TILE_SWAMP_WALL;
    TileType floor = dragon ? TILE_DRAGON_FLOOR : TILE_SWAMP_FLOOR;
    static const int links[][2] = {{0, 3}, {1, 4}, {2, 5}, {4, 7}};
    for (int i = 0; i < 4; i++) {
        link(m, links[i][0], links[i][1], wall, floor);
    }
    for (int i = 1; i < 8; i++) {
        int x, y;
        map_room_center(&m->rooms[i], &x, &y);
        brush(m, x, y, dragon ? 6 : 7, wall, floor);
    }
    const Room *last = &m->rooms[8];
    for (int y = last->y; y < last->y + last->h; y++) {
        for (int x = last->x; x < last->x + last->w; x++) {
            m->tiles[y][x] = original.tiles[y][x];
        }
    }
    // New scenery replaces blocked terrain only; old walking routes stay open.
    for (int y = 2; y < SWAMP_MAP_H - 2; y++) {
        for (int x = 2; x < SWAMP_MAP_W - 2; x++) {
            if (x >= last->x && x < last->x + last->w && y >= last->y && y < last->y + last->h) {
                continue;
            }
            if (m->tiles[y][x] == wall && (x / 7 + y / 8) % 3 != 0) {
                m->tiles[y][x] = dragon ? TILE_DRAGON_RAVINE : TILE_SWAMP_WATER;
            }
        }
    }
    if (dragon) {
        migrate_boulders(m, &original);
    } else {
        reeds(m);
    }
}

void wilds_migrate(GameState *g) {
    if (g->location == LOCATION_SWAMP || g->location == LOCATION_DRAGONSPINE) {
        migrate_map(&g->map, g->location == LOCATION_DRAGONSPINE);
    }
    for (int i = 0; i < SWAMP_DEPTH; i++) {
        if (g->swamp_cache[i].valid) {
            migrate_map(&g->swamp_cache[i].map, 0);
        }
    }
    for (int i = 0; i < DRAGONSPINE_DEPTH; i++) {
        if (g->dragonspine_cache[i].valid) {
            migrate_map(&g->dragonspine_cache[i].map, 1);
        }
    }
}
