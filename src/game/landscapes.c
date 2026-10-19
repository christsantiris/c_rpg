#include "landscapes.h"
#include <stdlib.h>
#include <string.h>

static void init_map(Map *m, TileType wall) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = wall;
        }
    }
    m->room_count = 9;
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

static void trail(Map *m, int ax, int ay, int bx, int by, int radius, TileType wall, TileType floor) {
    int dx = abs(bx - ax);
    int dy = -abs(by - ay);
    int sx = ax < bx ? 1 : -1;
    int sy = ay < by ? 1 : -1;
    int error = dx + dy;
    while (1) {
        brush(m, ax, ay, radius, wall, floor);
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

static void clearing(Map *m, int i, int cx, int cy, int w, int h, TileType floor) {
    Room *r = &m->rooms[i];
    *r = (Room){cx - w / 2, cy - h / 2, w, h};
    for (int y = r->y; y < r->y + h; y++) {
        for (int x = r->x; x < r->x + w; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (i == 8 || dx * dx * h * h + dy * dy * w * w <= w * w * h * h / 4) {
                m->tiles[y][x] = floor;
            }
        }
    }
}

static void connect(Map *m, int a, int b, int radius, TileType wall, TileType floor) {
    int ax, ay, bx, by;
    map_room_center(&m->rooms[a], &ax, &ay);
    map_room_center(&m->rooms[b], &bx, &by);
    trail(m, ax, ay, bx, by, radius, wall, floor);
}

static void exits(Map *m, TileType wall, TileType floor, TileType entrance, TileType exit) {
    int ax, ay, bx, by;
    map_room_center(&m->rooms[0], &ax, &ay);
    map_room_center(&m->rooms[8], &bx, &by);
    m->stairs_up_x = SWAMP_MAP_W - 1;
    m->stairs_up_y = ay;
    m->stairs_down_x = 0;
    m->stairs_down_y = by;
    trail(m, ax, ay, SWAMP_MAP_W - 2, ay, 1, wall, floor);
    trail(m, bx, by, 1, by, 1, wall, floor);
    m->tiles[ay][m->stairs_up_x] = entrance;
    m->tiles[by][0] = exit;
}

void map_generate_frost_ridges(Map *m, int level) {
    static const int centers[9][2] = {{59, 11}, {38, 11}, {16, 11}, {16, 31}, {38, 31}, {59, 31}, {59, 51}, {38, 51}, {16, 51}};
    init_map(m, TILE_FROST_WALL);
    for (int i = 0; i < 9; i++) {
        int y = centers[i][1] + (i == 8 ? 0 : (level + i) % 3 - 1);
        clearing(m, i, centers[i][0], y, 17 + rand() % 3, 11, TILE_FROST_FLOOR);
    }
    // Three long ridges turn at opposite ends; brittle crosslinks skip a turn.
    for (int i = 1; i < 9; i++) {
        connect(m, i - 1, i, 1, TILE_FROST_WALL, TILE_FROST_FLOOR);
    }
    exits(m, TILE_FROST_WALL, TILE_FROST_FLOOR, TILE_FROST_ENTRANCE, TILE_FROST_EXIT);
}

static void oasis(Map *m, int room) {
    const Room *r = &m->rooms[room];
    int cx, cy;
    map_room_center(r, &cx, &cy);
    int best = MAP_W + MAP_H;
    int x = -1;
    int y = -1;
    for (int ty = r->y + 1; ty < r->y + r->h - 1; ty++) {
        for (int tx = r->x + 1; tx < r->x + r->w - 1; tx++) {
            int distance = abs(tx - cx) + abs(ty - cy - 2);
            if (m->tiles[ty][tx] == TILE_DESERT_FLOOR && distance < best) {
                best = distance;
                x = tx;
                y = ty;
            }
        }
    }
    if (x >= 0) {
        m->tiles[y][x] = TILE_DESERT_OASIS;
    }
}

static int sunscar_connected(const Map *m) {
    unsigned char seen[SWAMP_MAP_H][SWAMP_MAP_W] = {{0}};
    int queue[SWAMP_MAP_W * SWAMP_MAP_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = m->stairs_up_y * SWAMP_MAP_W + m->stairs_up_x;
    seen[m->stairs_up_y][m->stairs_up_x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % SWAMP_MAP_W + offsets[i][0];
            int y = cell / SWAMP_MAP_W + offsets[i][1];
            if (x < 0 || y < 0 || x >= SWAMP_MAP_W || y >= SWAMP_MAP_H || seen[y][x] || !map_is_walkable(m, x, y)) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * SWAMP_MAP_W + x;
        }
    }
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            if (map_is_walkable(m, x, y) && !seen[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

static int sunscar_protected(const Map *m, const GameState *g, const Enemy *enemies, int count, int level, int active, int x, int y) {
    const Room *last = &m->rooms[m->room_count - 1];
    if (x >= last->x && y >= last->y && x < last->x + last->w && y < last->y + last->h) {
        return 1;
    }
    for (int i = 0; i < m->room_count; i++) {
        int cx, cy;
        map_room_center(&m->rooms[i], &cx, &cy);
        if (abs(x - cx) + abs(y - cy) <= 2) {
            return 1;
        }
    }
    for (int i = 0; i < count; i++) {
        if (enemies[i].x == x && enemies[i].y == y) {
            return 1;
        }
    }
    if (g) {
        if (g->portal_active && g->portal_location == LOCATION_DESERT && g->portal_level == level && g->portal_x == x && g->portal_y == y) {
            return 1;
        }
        if (active) {
            if (g->player.x == x && g->player.y == y) {
                return 1;
            }
            for (int i = 0; i < g->floor_item_count; i++) {
                if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void sunscar_obstacles(Map *m, const GameState *g, const Enemy *enemies, int count, int level, int active) {
    if (!m->room_count) {
        return;
    }
    unsigned int seed = 2166136261u + level;
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            if (m->tiles[y][x] == TILE_DESERT_MESA) {
                return;
            }
            if (m->tiles[y][x] == TILE_DESERT_WALL) {
                seed = (seed ^ (unsigned int)(x + y * SWAMP_MAP_W)) * 16777619u;
            }
        }
    }
    // Broad, staggered mesas and narrow rock spines leave several sandy routes.
    for (int cy = 11; cy <= 55; cy += 11) {
        for (int cx = 14; cx <= 58; cx += 11) {
            seed = seed * 1664525u + 1013904223u;
            int px = cx + (int)(seed % 5) - 2;
            int py = cy + (int)(seed / 7 % 3) - 1;
            int rx = 3 + seed / 13 % 3;
            int ry = 3 + seed / 19 % 2;
            if (seed % 4 == 0) {
                rx = 2;
                ry = 5;
            }
            int cells[128];
            int changed = 0;
            for (int dy = -ry; dy <= ry; dy++) {
                for (int dx = -rx - 1; dx <= rx + 1; dx++) {
                    int x = px + dx;
                    int y = py + dy;
                    int bend = (dy + ry) / 3 - ry / 3;
                    if (x <= 1 || y <= 1 || x >= SWAMP_MAP_W - 2 || y >= SWAMP_MAP_H - 2 ||
                        abs(dx - bend) * ry + abs(dy) * rx > rx * ry + 2 || m->tiles[y][x] != TILE_DESERT_FLOOR ||
                        sunscar_protected(m, g, enemies, count, level, active, x, y)) {
                        continue;
                    }
                    cells[changed++] = y * SWAMP_MAP_W + x;
                    m->tiles[y][x] = TILE_DESERT_MESA;
                }
            }
            if (changed && !sunscar_connected(m)) {
                for (int i = 0; i < changed; i++) {
                    m->tiles[cells[i] / SWAMP_MAP_W][cells[i] % SWAMP_MAP_W] = TILE_DESERT_FLOOR;
                }
            }
        }
    }
}

void map_generate_desert(Map *m, int level) {
    static const int centers[9][2] = {{60, 32}, {54, 15}, {35, 11}, {18, 16}, {37, 31}, {55, 49}, {33, 51}, {15, 47}, {10, 32}};
    init_map(m, TILE_DESERT_WALL);
    // A continuous dune basin, rather than chambers joined by trails.
    for (int y = 2; y < SWAMP_MAP_H - 2; y++) {
        for (int x = 2; x < SWAMP_MAP_W - 2; x++) {
            int dx = x - 36;
            int dy = y - 32;
            int radius = 33 + ((y / 7 + level) % 3 == 0 ? -2 : 0);
            if (dx * dx * 28 * 28 + dy * dy * radius * radius < radius * radius * 28 * 28) {
                m->tiles[y][x] = TILE_DESERT_FLOOR;
            }
        }
    }
    for (int i = 0; i < 9; i++) {
        clearing(m, i, centers[i][0], centers[i][1], i == 8 ? 14 : 9, i == 8 ? 13 : 9, TILE_DESERT_FLOOR);
    }
    // Isolated sandstone ribs break lines of fire without funneling every route.
    static const int ribs[8][2] = {{26, 20}, {43, 22}, {57, 30}, {25, 36}, {43, 40}, {25, 46}, {48, 49}, {33, 20}};
    for (int i = 0; i < 8; i++) {
        int cx = ribs[i][0] + rand() % 5 - 2;
        int cy = ribs[i][1] + rand() % 3 - 1;
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (abs(dx) + abs(dy) <= 2) {
                    m->tiles[cy + dy][cx + dx] = TILE_DESERT_WALL;
                }
            }
        }
    }
    // All landmarks remain connected around the ribs by broad sandy approaches.
    for (int i = 0; i < 9; i++) {
        connect(m, 4, i, 1, TILE_DESERT_WALL, TILE_DESERT_FLOOR);
    }
    oasis(m, 2);
    exits(m, TILE_DESERT_WALL, TILE_DESERT_FLOOR, TILE_DESERT_ENTRANCE, TILE_DESERT_EXIT);
    sunscar_obstacles(m, NULL, NULL, 0, level, 0);
}

static int waystone_site(const Map *m, int room, TileType kind, int *x, int *y) {
    const Room *r = &m->rooms[room];
    int cx, cy;
    map_room_center(r, &cx, &cy);
    int best = MAP_W + MAP_H;
    for (int ty = r->y + 1; ty < r->y + r->h - 1; ty++) {
        for (int tx = r->x + 1; tx < r->x + r->w - 1; tx++) {
            TileType tile = m->tiles[ty][tx];
            if (tile == kind) {
                *x = tx;
                *y = ty;
                return 1;
            }
            if (tile != TILE_MOONVEIL_FLOOR && tile != TILE_MOONVEIL_CIRCLE) {
                continue;
            }
            int distance = abs(tx - cx) + abs(ty - cy - 2);
            if (distance < best) {
                best = distance;
                *x = tx;
                *y = ty;
            }
        }
    }
    return best < MAP_W + MAP_H;
}

static void waystone_pair(Map *m, int a, int b, TileType kind) {
    int ax, ay, bx, by;
    // Place both ends together, finding alternate ground around existing loot.
    if (waystone_site(m, a, kind, &ax, &ay) && waystone_site(m, b, kind, &bx, &by)) {
        m->tiles[ay][ax] = kind;
        m->tiles[by][bx] = kind;
    }
}

void map_generate_moonveil(Map *m, int level) {
    static const int centers[9][2] = {{61, 32}, {51, 14}, {33, 10}, {16, 18}, {15, 45}, {33, 53}, {53, 49}, {49, 32}, {9, 32}};
    static const int edges[][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 0}, {0, 7}, {7, 1}, {7, 6}, {3, 8}, {4, 8}};
    init_map(m, TILE_MOONVEIL_WALL);
    for (int i = 0; i < 9; i++) {
        int shift = i == 0 || i == 8 ? 0 : rand() % 3 - 1;
        clearing(m, i, centers[i][0] + shift, centers[i][1], i == 8 ? 14 : 13, i == 8 ? 13 : 11, TILE_MOONVEIL_FLOOR);
    }
    for (int i = 0; i < (int)(sizeof(edges) / sizeof(edges[0])); i++) {
        connect(m, edges[i][0], edges[i][1], 2, TILE_MOONVEIL_WALL, TILE_MOONVEIL_FLOOR);
    }
    // Pools fill hedge interiors; the curving garden walks stay on their banks.
    for (int y = 3; y < SWAMP_MAP_H - 3; y++) {
        for (int x = 3; x < SWAMP_MAP_W - 3; x++) {
            int dx = x - 33;
            int dy = y - 32;
            if (dx * dx + dy * dy <= 100 + level * 2 && m->tiles[y][x] == TILE_MOONVEIL_WALL) {
                m->tiles[y][x] = TILE_MOONVEIL_POOL;
            }
        }
    }
    for (int i = 1; i < 9; i++) {
        int x, y;
        map_room_center(&m->rooms[i], &x, &y);
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                int d = dx * dx + dy * dy;
                if (d >= 7 && d <= 10 && m->tiles[y + dy][x + dx] == TILE_MOONVEIL_FLOOR) {
                    m->tiles[y + dy][x + dx] = TILE_MOONVEIL_CIRCLE;
                }
            }
        }
    }
    waystone_pair(m, 1, 5, TILE_MOONVEIL_WAYSTONE_A);
    waystone_pair(m, 3, 6, TILE_MOONVEIL_WAYSTONE_B);
    exits(m, TILE_MOONVEIL_WALL, TILE_MOONVEIL_FLOOR, TILE_MOONVEIL_ENTRANCE, TILE_MOONVEIL_EXIT);
}

int landscape_tile(TileType tile) {
    return tile >= TILE_DESERT_OASIS && tile <= TILE_MOONVEIL_WAYSTONE_B;
}

static TileType terrain(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_WALL;
    }
    if (g->map.tiles[y][x] == TILE_PORTAL && g->portal_active && g->portal_location == g->location &&
        g->portal_level == g->level && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    if (g->map.tiles[y][x] == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                return g->floor_items[i].underlying_tile;
            }
        }
    }
    return g->map.tiles[y][x];
}

static int interaction(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_DESERT && g->location != LOCATION_MOONVEIL) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        int tx = g->player.x + offsets[i][0];
        int ty = g->player.y + offsets[i][1];
        TileType tile = terrain(g, tx, ty);
        if ((g->location == LOCATION_DESERT && (tile == TILE_DESERT_OASIS || tile == TILE_DESERT_OASIS_DRY)) ||
            (g->location == LOCATION_MOONVEIL && (tile == TILE_MOONVEIL_WAYSTONE_A || tile == TILE_MOONVEIL_WAYSTONE_B))) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

int landscape_has_interaction(const GameState *g) {
    int x, y;
    return interaction(g, &x, &y);
}

int landscape_interact(GameState *g) {
    int x, y;
    if (!interaction(g, &x, &y)) {
        return 0;
    }
    TileType tile = terrain(g, x, y);
    if (g->location == LOCATION_DESERT) {
        if (tile == TILE_DESERT_OASIS_DRY) {
            push_message(g, "The oasis well is dry. It refills on a new expedition from town.");
            return 1;
        }
        if (g->player.hp == g->player.max_hp && g->player.mp == g->player.max_mp) {
            push_message(g, "You need no water. The oasis well remains full.");
            return 1;
        }
        int hp = g->player.max_hp / 4;
        int mp = g->player.max_mp / 4;
        g->player.hp += hp > 0 ? hp : 1;
        g->player.mp += mp > 0 ? mp : 1;
        if (g->player.hp > g->player.max_hp) {
            g->player.hp = g->player.max_hp;
        }
        if (g->player.mp > g->player.max_mp) {
            g->player.mp = g->player.max_mp;
        }
        if (g->map.tiles[y][x] == TILE_PORTAL) {
            g->portal_origin_tile = TILE_DESERT_OASIS_DRY;
        } else if (g->map.tiles[y][x] != TILE_ITEM) {
            g->map.tiles[y][x] = TILE_DESERT_OASIS_DRY;
        }
        for (int i = 0; i < g->floor_item_count; i++) {
            if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
                g->floor_items[i].underlying_tile = TILE_DESERT_OASIS_DRY;
            }
        }
        push_message(g, "Oasis water restores a quarter of your health and mana. The well runs dry.");
        return 1;
    }
    for (int ty = 1; ty < SWAMP_MAP_H - 1; ty++) {
        for (int tx = 1; tx < SWAMP_MAP_W - 1; tx++) {
            if ((tx == x && ty == y) || terrain(g, tx, ty) != tile) {
                continue;
            }
            for (int i = 0; i < g->enemy_count; i++) {
                if (g->enemies[i].active && g->enemies[i].x == tx && g->enemies[i].y == ty) {
                    push_message(g, "A creature blocks the matching waystone. Clear its landing first.");
                    return 1;
                }
            }
            game_move_player(g, tx - g->player.x, ty - g->player.y);
            map_mark_explored(&g->map, tx, ty);
            push_message(g, "Moonlight carries you to the matching garden waystone.");
            return 1;
        }
    }
    push_message(g, "This waystone has no matching stone on this stage.");
    return 1;
}

void landscape_new_expedition(Map *m) {
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            if (m->tiles[y][x] == TILE_DESERT_OASIS_DRY) {
                m->tiles[y][x] = TILE_DESERT_OASIS;
            } else if (m->tiles[y][x] == TILE_FROST_BROKEN_ICE) {
                m->tiles[y][x] = TILE_FROST_THIN_ICE;
            }
        }
    }
}

static int modern_map(const Map *m, Location location) {
    if (location != LOCATION_FROSTFELL) {
        for (int y = 0; y < SWAMP_MAP_H; y++) {
            for (int x = 0; x < SWAMP_MAP_W; x++) {
                if (landscape_tile(m->tiles[y][x])) {
                    return 1;
                }
            }
        }
        return 0;
    }
    // The glacier's three fixed ridges identify its new layout without adding
    // persistent state. Old mirrored clearings do not have these nine anchors.
    static const int xs[9] = {59, 38, 16, 16, 38, 59, 59, 38, 16};
    for (int i = 0; i < 9; i++) {
        int x, y;
        map_room_center(&m->rooms[i], &x, &y);
        if (x != xs[i] || abs(y - (11 + i / 3 * 20)) > 1 || m->rooms[i].h != 11 || m->rooms[i].w < 17) {
            return 0;
        }
    }
    return 1;
}

static void migrate_map(Map *m, Location location) {
    if (m->room_count < 9 || modern_map(m, location)) {
        return;
    }
    TileType wall = location == LOCATION_FROSTFELL ? TILE_FROST_WALL : location == LOCATION_DESERT ? TILE_DESERT_WALL : TILE_MOONVEIL_WALL;
    TileType floor = location == LOCATION_FROSTFELL ? TILE_FROST_FLOOR : location == LOCATION_DESERT ? TILE_DESERT_FLOOR : TILE_MOONVEIL_FLOOR;
    // Expand old trails in place; never replace progress, items or open squares.
    Map original = *m;
    for (int i = 1; i < 8; i++) {
        int x, y;
        map_room_center(&m->rooms[i], &x, &y);
        brush(m, x, y, location == LOCATION_DESERT ? 9 : 6, wall, floor);
    }
    static const int links[4][2] = {{0, 3}, {1, 4}, {2, 5}, {4, 7}};
    for (int i = 0; i < 4; i++) {
        connect(m, links[i][0], links[i][1], location == LOCATION_DESERT ? 4 : 1, wall, floor);
    }
    // The last chamber may contain a live boss, a restored quest or its loot.
    const Room *last = &m->rooms[8];
    for (int y = last->y; y < last->y + last->h; y++) {
        for (int x = last->x; x < last->x + last->w; x++) {
            m->tiles[y][x] = original.tiles[y][x];
        }
    }
    if (location == LOCATION_DESERT) {
        oasis(m, 2);
    } else if (location == LOCATION_MOONVEIL) {
        waystone_pair(m, 1, 5, TILE_MOONVEIL_WAYSTONE_A);
        waystone_pair(m, 3, 6, TILE_MOONVEIL_WAYSTONE_B);
    }
}

void landscape_migrate(GameState *g) {
    if (g->location == LOCATION_FROSTFELL || g->location == LOCATION_DESERT || g->location == LOCATION_MOONVEIL) {
        migrate_map(&g->map, g->location);
    }
    for (int i = 0; i < 5; i++) {
        if (g->frostfell_cache[i].valid) {
            migrate_map(&g->frostfell_cache[i].map, LOCATION_FROSTFELL);
        }
        if (g->desert_cache[i].valid) {
            migrate_map(&g->desert_cache[i].map, LOCATION_DESERT);
        }
        if (g->moonveil_cache[i].valid) {
            migrate_map(&g->moonveil_cache[i].map, LOCATION_MOONVEIL);
        }
    }
}

void sunscar_migrate(GameState *g) {
    if (g->location == LOCATION_DESERT) {
        sunscar_obstacles(&g->map, g, g->enemies, g->enemy_count, g->level, 1);
    }
    for (int i = 0; i < DESERT_DEPTH; i++) {
        LevelCache *cache = &g->desert_cache[i];
        if (cache->valid) {
            sunscar_obstacles(&cache->map, g, cache->enemies, cache->enemy_count, i + 1, 0);
        }
    }
}
