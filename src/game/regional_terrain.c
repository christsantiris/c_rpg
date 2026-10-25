#include "regional_terrain.h"
#include "stonewake.h"
#include <stdlib.h>
#include <string.h>

int regional_terrain_tile(TileType tile) {
    return (tile >= TILE_ASHEN_COLD_6 && tile <= TILE_ASHEN_VALVE_DIVERTED) ||
        (tile >= TILE_STONEWAKE_BOUNDARY && tile <= TILE_STONEWAKE_REMOTE);
}

int regional_terrain_heat(TileType tile) {
    return tile >= TILE_ASHEN_COLD_6 && tile <= TILE_ASHEN_ACTIVE_1;
}

static int face(TileType tile) {
    return tile >= TILE_GLASS_FACE_N && tile <= TILE_GLASS_FACE_W;
}

static TileType terrain_at(const GameState *g, int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return TILE_WALL;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_PORTAL && g->portal_active && g->portal_location == g->location &&
        g->portal_level == g->level && x == g->portal_x && y == g->portal_y) {
        return g->portal_origin_tile;
    }
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                return item->underlying_tile;
            }
        }
    }
    return tile;
}

static void set_terrain(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_PORTAL && g->portal_active &&
        x == g->portal_x && y == g->portal_y && g->portal_location == g->location && g->portal_level == g->level) {
        g->portal_origin_tile = tile;
    } else if (g->map.tiles[y][x] != TILE_ITEM) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
        }
    }
}

static int region_plain(TileType tile, int glass) {
    return glass ? tile == TILE_GLASSDEEP_FLOOR || tile == TILE_GLASSDEEP_RUIN :
        tile == TILE_ASHEN_FLOOR || tile == TILE_ASHEN_RUIN;
}

static void carve(Map *m, int cx, int cy, int radius, int glass) {
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x <= 0 || y <= 0 || x >= SWAMP_MAP_W - 1 || y >= SWAMP_MAP_H - 1) {
                continue;
            }
            TileType tile = m->tiles[y][x];
            if (glass ? tile == TILE_GLASSDEEP_WALL || tile == TILE_GLASSDEEP_POOL :
                tile == TILE_ASHEN_WALL || tile == TILE_ASHEN_LAVA) {
                m->tiles[y][x] = glass ? TILE_GLASSDEEP_FLOOR : TILE_ASHEN_FLOOR;
            }
        }
    }
}

static void path(Map *m, int ax, int ay, int bx, int by, int radius, int glass) {
    // Angular basalt terraces and crystal galleries have deliberate sightlines.
    while (ax != bx || ay != by) {
        carve(m, ax, ay, radius, glass);
        if (ax != bx) {
            ax += ax < bx ? 1 : -1;
        } else {
            ay += ay < by ? 1 : -1;
        }
    }
    carve(m, bx, by, radius, glass);
}

static void link_rooms(Map *m, int a, int b, int radius, int glass) {
    int ax, ay, bx, by;
    map_room_center(&m->rooms[a], &ax, &ay);
    map_room_center(&m->rooms[b], &bx, &by);
    path(m, ax, ay, bx, by, radius, glass);
}

static void init_region(Map *m, int glass) {
    memset(m, 0, sizeof(*m));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = glass ? TILE_GLASSDEEP_WALL : TILE_ASHEN_WALL;
        }
    }
    m->room_count = 9;
}

static void chamber(Map *m, int i, int cx, int cy, int w, int h, int glass) {
    Room *room = &m->rooms[i];
    *room = (Room){cx - w / 2, cy - h / 2, w, h};
    for (int y = room->y; y < room->y + h; y++) {
        for (int x = room->x; x < room->x + w; x++) {
            int dx = abs(x - cx);
            int dy = abs(y - cy);
            // Broad, stepped basalt shelves versus rounded crystal chambers.
            int inside = glass ? dx * dx * h * h + dy * dy * w * w <= w * w * h * h / 4 :
                dx + dy < (w + h) / 2 - 3;
            if (inside || i == 8) {
                m->tiles[y][x] = glass ? TILE_GLASSDEEP_FLOOR : TILE_ASHEN_FLOOR;
            }
        }
    }
    for (int x = cx - 2; x <= cx + 2; x++) {
        m->tiles[cy + 2][x] = glass ? TILE_GLASSDEEP_RUIN : TILE_ASHEN_RUIN;
    }
}

static int add_crossing(Map *m, int x, int y, int fresh) {
    if (x < 6 || y < 4 || x >= SWAMP_MAP_W - 6 || y >= SWAMP_MAP_H - 4) {
        return 0;
    }
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (!region_plain(m->tiles[y + dy][x + dx], 0)) {
                return 0;
            }
        }
    }
    if (!region_plain(m->tiles[y + 3][x - 3], 0)) {
        return 0;
    }
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            m->tiles[y + dy][x + dx] = TILE_ASHEN_COLD_6;
        }
    }
    if (fresh) {
        // The short bridge spans a real fissure; its flanks cannot be sidestepped.
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -5; dx <= 5; dx++) {
                if (abs(dx) >= 2) {
                    m->tiles[y + dy][x + dx] = TILE_ASHEN_LAVA;
                }
            }
        }
    }
    m->tiles[y + 3][x - 3] = TILE_ASHEN_VALVE;
    return 1;
}

static int add_optics(Map *m, int room_index, int legacy) {
    const Room *room = &m->rooms[room_index];
    int x = room->x + room->w / 2;
    int y = room->y + 4;
    if (x < 6 || y < 7 || x >= SWAMP_MAP_W - 6 || y >= SWAMP_MAP_H - 4) {
        return 0;
    }
    TileType gate = m->tiles[y - 5][x];
    if (legacy && gate != TILE_GLASSDEEP_WALL && gate != TILE_GLASSDEEP_POOL) {
        return 0;
    }
    for (int step = 0; step <= 4; step++) {
        if (!region_plain(m->tiles[y][x - step], 1) || !region_plain(m->tiles[y - step][x], 1)) {
            return 0;
        }
    }
    if (legacy) {
        int tx;
        int ty;
        map_room_center(&m->rooms[room_index > 1 ? room_index - 2 : 0], &tx, &ty);
        path(m, x, y - 6, tx, ty, 0, 1);
    }
    m->tiles[y][x] = TILE_GLASS_FACE_E;
    m->tiles[y][x - 4] = TILE_GLASS_SOURCE;
    m->tiles[y - 4][x] = TILE_GLASS_RECEIVER;
    m->tiles[y - 5][x] = TILE_GLASS_SEAL_CLOSED;
    if (!legacy) {
        m->tiles[y - 5][x - 1] = TILE_GLASSDEEP_WALL;
        m->tiles[y - 5][x + 1] = TILE_GLASSDEEP_WALL;
    }
    return 1;
}

void map_generate_ashen(Map *m, int level) {
    static const int centers[9][2] = {{16, 55}, {51, 53}, {33, 43}, {16, 34}, {53, 34}, {35, 25}, {16, 15}, {53, 15}, {35, 7}};
    static const int edges[][2] = {{0, 1}, {0, 2}, {1, 2}, {2, 3}, {2, 4}, {3, 5}, {4, 5}, {5, 6}, {5, 7}, {6, 8}, {7, 8}};
    init_region(m, 0);
    // Lava rivers separate the terraces; permanent basalt bridges remain open.
    for (int y = 1; y < SWAMP_MAP_H - 1; y++) {
        for (int x = 1; x < SWAMP_MAP_W - 1; x++) {
            int bend = (x / 8 + level) % 3 - 1;
            if (abs(y - 45 - bend) <= 2 || abs(y - 29 + bend) <= 2 || abs(y - 20 - bend) <= 1) {
                m->tiles[y][x] = TILE_ASHEN_LAVA;
            }
        }
    }
    for (int i = 0; i < 9; i++) {
        int jitter = i == 8 ? 0 : rand() % 3 - 1;
        chamber(m, i, centers[i][0] + jitter, centers[i][1], 14 + i % 3 * 2, 9 + i % 2, 0);
    }
    for (int i = 0; i < (int)(sizeof(edges) / sizeof(edges[0])); i++) {
        link_rooms(m, edges[i][0], edges[i][1], 2, 0);
    }
    for (int n = 0; n < level && n < 3; n++) {
        int a = n == 1 ? 3 : 1;
        int b = n == 1 ? 6 : n == 2 ? 7 : 4;
        int ax, ay, bx, by;
        map_room_center(&m->rooms[a], &ax, &ay);
        map_room_center(&m->rooms[b], &bx, &by);
        path(m, ax, ay, bx, by, 1, 0);
        int bend = (bx / 8 + level) % 3 - 1;
        int y = n == 0 ? 45 + bend : n == 1 ? 29 - bend : 20 + bend;
        path(m, bx, y - 3, bx, y + 3, 1, 0);
        path(m, bx - 3, y + 3, bx, y + 3, 1, 0);
        add_crossing(m, bx, y, 1);
    }
    m->stairs_up_x = centers[0][0];
    m->stairs_up_y = SWAMP_MAP_H - 1;
    m->stairs_down_x = centers[8][0];
    m->stairs_down_y = 0;
    path(m, m->stairs_up_x, 55, m->stairs_up_x, SWAMP_MAP_H - 2, 1, 0);
    path(m, m->stairs_down_x, 7, m->stairs_down_x, 1, 1, 0);
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_ASHEN_ENTRANCE;
    m->tiles[0][m->stairs_down_x] = TILE_ASHEN_EXIT;
}

void map_generate_glassdeep(Map *m, int level) {
    stonewake_generate(m, level);
}

static int interaction(const GameState *g, int *x, int *y) {
    if (g->location != LOCATION_ASHEN && g->location != LOCATION_GLASSDEEP) {
        return 0;
    }
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (int i = 0; i < 5; i++) {
        int tx = g->player.x + offsets[i][0];
        int ty = g->player.y + offsets[i][1];
        TileType tile = terrain_at(g, tx, ty);
        if ((g->location == LOCATION_ASHEN && (tile == TILE_ASHEN_VALVE || tile == TILE_ASHEN_VALVE_DIVERTED)) ||
            (g->location == LOCATION_GLASSDEEP && face(tile))) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    return 0;
}

int regional_terrain_has_interaction(const GameState *g) {
    int x, y;
    return stonewake_has_interaction(g) || interaction(g, &x, &y);
}

static int gate_occupied(const GameState *g, int x, int y) {
    if ((g->player.x == x && g->player.y == y) ||
        (g->portal_active && g->portal_location == g->location && g->portal_level == g->level && g->portal_x == x && g->portal_y == y)) {
        return 1;
    }
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].active && g->enemies[i].x == x && g->enemies[i].y == y) {
            return 1;
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        if (g->floor_items[i].active && g->floor_items[i].x == x && g->floor_items[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static int source_lit(const GameState *g, int x, int y) {
    if (terrain_at(g, x - 4, y) != TILE_GLASS_SOURCE) {
        return 0;
    }
    for (int step = 1; step < 4; step++) {
        if (!map_is_walkable(&g->map, x - step, y)) {
            return 0;
        }
    }
    return 1;
}

int regional_terrain_interact(GameState *g) {
    if (stonewake_interact(g)) {
        return 1;
    }
    int x, y;
    if (!interaction(g, &x, &y)) {
        return 0;
    }
    if (g->location == LOCATION_ASHEN) {
        if (terrain_at(g, x, y) == TILE_ASHEN_VALVE_DIVERTED) {
            push_message(g, "The lava is already diverted. This basalt crossing stays safe.");
            return 1;
        }
        // add_crossing places this valve three tiles southwest of its bridge.
        for (int ty = y - 4; ty <= y - 2; ty++) {
            for (int tx = x + 2; tx <= x + 4; tx++) {
                if (regional_terrain_heat(terrain_at(g, tx, ty))) {
                    set_terrain(g, tx, ty, TILE_ASHEN_BASALT_CROSSING);
                }
            }
        }
        set_terrain(g, x, y, TILE_ASHEN_VALVE_DIVERTED);
        push_message(g, "The valve diverts the lava. The crossing hardens into a lasting basalt path.");
        return 1;
    }
    TileType next = terrain_at(g, x, y) == TILE_GLASS_FACE_W ? TILE_GLASS_FACE_N : terrain_at(g, x, y) + 1;
    set_terrain(g, x, y, next);
    int gx = x;
    int gy = y - 5;
    TileType gate = terrain_at(g, gx, gy);
    int open = next == TILE_GLASS_FACE_N && terrain_at(g, x, y - 4) == TILE_GLASS_RECEIVER &&
        regional_terrain_light(g, x, y - 4) == 2;
    if (gate == TILE_GLASS_SEAL_OPEN || gate == TILE_GLASS_SEAL_CLOSED) {
        if (open || !gate_occupied(g, gx, gy)) {
            set_terrain(g, gx, gy, open ? TILE_GLASS_SEAL_OPEN : TILE_GLASS_SEAL_CLOSED);
        }
        map_mark_explored(&g->map, gx, gy);
    }
    for (int i = 0; i < g->enemy_count; i++) {
        Enemy *enemy = &g->enemies[i];
        if (enemy->active && enemy->type == ENEMY_BLIND_STALKER &&
            abs(enemy->x - x) + abs(enemy->y - y) <= 10) {
            enemy->move_timer = 1;
        }
    }
    push_message(g, open ? "Light reaches the receiver: the shortcut opens. Nearby Stalkers stir." :
        "The crystal turns. Aim its light north into the receiver to open the shortcut.");
    return 1;
}

int regional_terrain_tick(GameState *g) {
    if (g->location != LOCATION_ASHEN) {
        return 0;
    }
    int damage = 0;
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            TileType tile = terrain_at(g, x, y);
            if (!regional_terrain_heat(tile)) {
                continue;
            }
            TileType next = tile == TILE_ASHEN_ACTIVE_1 ? TILE_ASHEN_COLD_6 : tile + 1;
            set_terrain(g, x, y, next);
            if (g->player.x == x && g->player.y == y && next >= TILE_ASHEN_ACTIVE_2) {
                damage = 8;
            }
        }
    }
    return damage;
}

void regional_terrain_cancel_landing(GameState *g, int x, int y) {
    if (g->location == LOCATION_ASHEN && regional_terrain_heat(terrain_at(g, x, y))) {
        for (int ty = y - 2; ty <= y + 2; ty++) {
            for (int tx = x - 2; tx <= x + 2; tx++) {
                if (regional_terrain_heat(terrain_at(g, tx, ty))) {
                    set_terrain(g, tx, ty, TILE_ASHEN_COLD_6);
                }
            }
        }
    }
}

int regional_terrain_light(const GameState *g, int x, int y) {
    if (stonewake_map(&g->map) || g->location != LOCATION_GLASSDEEP || !map_is_walkable(&g->map, x, y)) {
        return 0;
    }
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    for (int direction = 0; direction < 4; direction++) {
        for (int step = 1; step <= 7; step++) {
            int tx = x - dx[direction] * step;
            int ty = y - dy[direction] * step;
            TileType tile = terrain_at(g, tx, ty);
            if (face(tile)) {
                if (!source_lit(g, tx, ty)) {
                    break;
                }
                if (tile == TILE_GLASS_FACE_N + direction || (direction == 3 && step <= 4)) {
                    return direction % 2 ? 1 : 2;
                }
                break;
            }
            if (!map_is_walkable(&g->map, tx, ty) || tile == TILE_GLASS_RECEIVER || tile == TILE_GLASS_SOURCE) {
                break;
            }
        }
    }
    return 0;
}

static void migrate_map(Map *m, int glass) {
    if (m->room_count < 7) {
        return;
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (regional_terrain_tile(m->tiles[y][x])) {
                return;
            }
        }
    }
    Map original = *m;
    // Add loops in place. Existing traversable squares and special props stay put.
    link_rooms(m, 1, 4, 1, glass);
    link_rooms(m, 2, 6, 1, glass);
    if (glass) {
        for (int i = 1; i < m->room_count - 1; i++) {
            if (add_optics(m, i, 1)) {
                break;
            }
        }
    } else {
        for (int i = 1; i < m->room_count - 1; i++) {
            int cx;
            int cy;
            map_room_center(&m->rooms[i], &cx, &cy);
            if (add_crossing(m, cx, cy, 0)) {
                break;
            }
        }
    }
    // The final chamber may hold a saved boss warning or a quest objective.
    const Room *last = &m->rooms[m->room_count - 1];
    for (int y = last->y; y < last->y + last->h; y++) {
        for (int x = last->x; x < last->x + last->w; x++) {
            m->tiles[y][x] = original.tiles[y][x];
        }
    }
}

void regional_terrain_migrate(GameState *g) {
    if (g->location == LOCATION_ASHEN || g->location == LOCATION_GLASSDEEP) {
        migrate_map(&g->map, g->location == LOCATION_GLASSDEEP);
    }
    for (int i = 0; i < ASHEN_DEPTH; i++) {
        if (g->ashen_cache[i].valid) {
            migrate_map(&g->ashen_cache[i].map, 0);
        }
        if (g->glassdeep_cache[i].valid) {
            migrate_map(&g->glassdeep_cache[i].map, 1);
        }
    }
}
