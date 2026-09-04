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

    // Turn the final room into a real, sealed boss chamber. Rebuilding the
    // perimeter closes any incidental corridors that crossed the randomly
    // generated room, leaving exactly one entrance from the intended path.
    if (level == DUNGEON_DEPTH && m->room_count > 1) {
        Room *boss_room = &m->rooms[m->room_count - 1];
        int previous_x, previous_y;
        map_room_center(&m->rooms[m->room_count - 2], &previous_x, &previous_y);
        int door_x = dx;
        int door_y = dy;
        if (previous_y < boss_room->y) {
            door_y = boss_room->y;
        } else if (previous_y >= boss_room->y + boss_room->h) {
            door_y = boss_room->y + boss_room->h - 1;
        } else if (previous_x < boss_room->x) {
            door_x = boss_room->x;
            door_y = previous_y;
        } else {
            door_x = boss_room->x + boss_room->w - 1;
            door_y = previous_y;
        }
        for (int y = boss_room->y; y < boss_room->y + boss_room->h; y++) {
            for (int x = boss_room->x; x < boss_room->x + boss_room->w; x++) {
                int perimeter = x == boss_room->x ||
                    x == boss_room->x + boss_room->w - 1 ||
                    y == boss_room->y ||
                    y == boss_room->y + boss_room->h - 1;
                m->tiles[y][x] = perimeter ? TILE_WALL : TILE_FLOOR;
            }
        }
        m->tiles[dy][dx] = TILE_STAIRS_DOWN;
        m->tiles[door_y][door_x] = TILE_LOCKED_DOOR;
    }

    // Place traps in rooms (skip room 0 — player spawn)
    int num_traps = 2 + level;
    if (num_traps > 12) num_traps = 12;
    for (int t = 0; t < num_traps; t++) {
        int room_idx = 1 + rand() % (m->room_count - 1);
        Room *room = &m->rooms[room_idx];
        int tx = room->x + 1 + rand() % (room->w - 2);
        int ty = room->y + 1 + rand() % (room->h - 2);
        if (m->tiles[ty][tx] != TILE_FLOOR) continue;
        m->tiles[ty][tx] = TILE_TRAP_HIDDEN;
    }

    if (level == DUNGEON_DEPTH && m->room_count > 2) {
        // Put the key at the center of the penultimate room. This makes it a
        // guaranteed landmark on the critical path instead of a tiny object
        // hidden at a random coordinate in a large floor.
        Room *key_room = &m->rooms[m->room_count - 2];
        int key_x, key_y;
        int key_placed = 0;
        map_room_center(key_room, &key_x, &key_y);
        // The key takes precedence over a randomly placed hidden trap.
        m->tiles[key_y][key_x] = TILE_DUNGEON_KEY;
        key_placed = 1;
        for (int room_idx = 1; !key_placed &&
            room_idx < m->room_count - 1; room_idx++) {
            Room *room = &m->rooms[room_idx];
            for (int key_y = room->y + 1; !key_placed &&
                key_y < room->y + room->h - 1; key_y++) {
                for (int key_x = room->x + 1;
                    key_x < room->x + room->w - 1; key_x++) {
                    if (m->tiles[key_y][key_x] == TILE_FLOOR) {
                        m->tiles[key_y][key_x] = TILE_DUNGEON_KEY;
                        key_placed = 1;
                        break;
                    }
                }
            }
        }
    }
}

int map_is_walkable(const Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
    return m->tiles[y][x] != TILE_WALL &&
        m->tiles[y][x] != TILE_FOREST_WALL &&
        m->tiles[y][x] != TILE_MOUNTAIN_WALL &&
        m->tiles[y][x] != TILE_COAST_WALL &&
        m->tiles[y][x] != TILE_TAVERN &&
        m->tiles[y][x] != TILE_LOCKED_DOOR;
}

typedef struct {
    int room_count;
    int x[MAX_ROOMS];
    int y[MAX_ROOMS];
    int edge_count;
    int edges[16][2];
} ForestTemplate;

typedef enum {
    OUTDOOR_SIDE_WEST,
    OUTDOOR_SIDE_EAST,
    OUTDOOR_SIDE_NORTH,
    OUTDOOR_SIDE_SOUTH
} OutdoorSide;

static const ForestTemplate forest_templates[FOREST_DEPTH] = {
    {7, {5,42,42,92,92,132,166}, {42,14,68,12,66,40,42}, 8,
        {{0,1},{0,2},{1,3},{2,4},{3,4},{3,5},{4,5},{5,6}}},
    {8, {5,38,38,78,78,78,125,166}, {42,20,65,10,42,74,42,42}, 10,
        {{0,1},{0,2},{1,3},{1,4},{2,4},{2,5},{3,6},{4,6},{5,6},{6,7}}},
    {9, {5,40,40,40,94,94,94,136,166}, {42,12,42,72,12,42,72,42,42}, 12,
        {{0,1},{0,2},{0,3},{1,4},{2,5},{3,6},{1,5},{3,5},{4,7},{5,7},{6,7},{7,8}}},
    {8, {5,45,86,91,86,132,132,166}, {42,42,12,42,72,24,61,42}, 10,
        {{0,1},{1,2},{1,3},{1,4},{2,5},{3,5},{3,6},{4,6},{5,7},{6,7}}},
    {9, {5,38,38,82,82,122,122,148,176}, {42,16,68,10,72,18,66,42,42}, 10,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{5,7},{6,7},{3,4},{7,8}}},
    {10, {5,34,34,70,70,105,105,140,140,176}, {42,16,68,12,48,18,74,28,68,42}, 10,
        {{0,1},{0,2},{1,3},{2,4},{3,4},{3,5},{4,6},{6,8},{4,7},{7,9}}},
    {10, {5,38,38,76,76,112,112,146,146,176}, {42,12,42,72,25,15,68,28,76,42}, 10,
        {{0,1},{0,2},{0,3},{1,4},{2,4},{3,6},{4,5},{4,7},{6,8},{7,9}}},
    {10, {5,36,36,72,72,108,108,142,142,176}, {42,14,70,18,66,10,76,24,66,42}, 11,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{3,4},{5,7},{6,8},{7,9},{8,9}}}
};

static void map_generate_outdoor(Map *m, int level, OutdoorSide entrance_side, OutdoorSide exit_side) {
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = TILE_FOREST_WALL;

    int template_index = level - 1;
    if (template_index < 0) template_index = 0;
    if (template_index >= FOREST_DEPTH) template_index = FOREST_DEPTH - 1;
    const ForestTemplate *layout = &forest_templates[template_index];
    m->room_count = layout->room_count;
    for (int i = 0; i < m->room_count; i++) {
        Room r;
        r.w = random_range(10, 17);
        r.h = random_range(8, 13);
        r.x = layout->x[i] + random_range(-3, 3);
        r.y = layout->y[i] + random_range(-3, 3);
        fill_rect(m, r.x, r.y, r.w, r.h, TILE_FOREST_FLOOR);
        m->rooms[i] = r;
    }

    for (int i = 0; i < layout->edge_count; i++) {
        int ax, ay, bx, by;
        map_room_center(&m->rooms[layout->edges[i][0]], &ax, &ay);
        map_room_center(&m->rooms[layout->edges[i][1]], &bx, &by);
        carve_corridor(m, ax, ay, bx, by);
    }
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            if (m->tiles[y][x] == TILE_FLOOR)
                m->tiles[y][x] = TILE_FOREST_FLOOR;

    int first_x, first_y, last_x, last_y;
    map_room_center(&m->rooms[0], &first_x, &first_y);
    map_room_center(&m->rooms[m->room_count - 1], &last_x, &last_y);
    if (entrance_side == OUTDOOR_SIDE_NORTH) {
        for (int y = first_y; y >= 0; y--) {
            m->tiles[y][first_x] = TILE_FOREST_FLOOR;
        }
        m->tiles[0][first_x] = TILE_FOREST_ENTRANCE;
        m->stairs_up_x = first_x;
        m->stairs_up_y = 1;
    } else if (entrance_side == OUTDOOR_SIDE_SOUTH) {
        for (int y = first_y; y < MAP_H; y++) {
            m->tiles[y][first_x] = TILE_FOREST_FLOOR;
        }
        m->tiles[MAP_H - 1][first_x] = TILE_FOREST_ENTRANCE;
        m->stairs_up_x = first_x;
        m->stairs_up_y = MAP_H - 2;
    } else if (entrance_side == OUTDOOR_SIDE_EAST) {
        for (int x = first_x; x < MAP_W; x++) {
            m->tiles[first_y][x] = TILE_FOREST_FLOOR;
        }
        m->tiles[first_y][MAP_W - 1] = TILE_FOREST_ENTRANCE;
        m->stairs_up_x = MAP_W - 2;
        m->stairs_up_y = first_y;
    } else {
        for (int x = 0; x <= first_x; x++) {
            m->tiles[first_y][x] = TILE_FOREST_FLOOR;
        }
        m->tiles[first_y][0] = TILE_FOREST_ENTRANCE;
        m->stairs_up_x = 1;
        m->stairs_up_y = first_y;
    }
    if (exit_side == OUTDOOR_SIDE_NORTH) {
        for (int y = last_y; y >= 0; y--) {
            m->tiles[y][last_x] = TILE_FOREST_FLOOR;
        }
        m->tiles[0][last_x] = TILE_FOREST_EXIT;
        m->stairs_down_x = last_x;
        m->stairs_down_y = 1;
    } else if (exit_side == OUTDOOR_SIDE_SOUTH) {
        for (int y = last_y; y < MAP_H; y++) {
            m->tiles[y][last_x] = TILE_FOREST_FLOOR;
        }
        m->tiles[MAP_H - 1][last_x] = TILE_FOREST_EXIT;
        m->stairs_down_x = last_x;
        m->stairs_down_y = MAP_H - 2;
    } else if (exit_side == OUTDOOR_SIDE_WEST) {
        for (int x = 0; x <= last_x; x++) {
            m->tiles[last_y][x] = TILE_FOREST_FLOOR;
        }
        m->tiles[last_y][0] = TILE_FOREST_EXIT;
        m->stairs_down_x = 1;
        m->stairs_down_y = last_y;
    } else {
        for (int x = last_x; x < MAP_W; x++) {
            m->tiles[last_y][x] = TILE_FOREST_FLOOR;
        }
        m->tiles[last_y][MAP_W - 1] = TILE_FOREST_EXIT;
        m->stairs_down_x = MAP_W - 2;
        m->stairs_down_y = last_y;
    }

    int num_traps = 2 + level;
    for (int i = 0; i < num_traps; i++) {
        Room *room = &m->rooms[1 + rand() % (m->room_count - 1)];
        int x = room->x + 1 + rand() % (room->w - 2);
        int y = room->y + 1 + rand() % (room->h - 2);
        if (m->tiles[y][x] == TILE_FOREST_FLOOR)
            m->tiles[y][x] = TILE_TRAP_HIDDEN;
    }
}

void map_generate_forest(Map *m, int level) {
    static const OutdoorSide entrances[FOREST_DEPTH] = {
        OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_WEST
    };
    static const OutdoorSide exits[FOREST_DEPTH] = {
        OUTDOOR_SIDE_EAST,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_EAST,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_EAST
    };
    int index = level - 1;
    if (index < 0) {
        index = 0;
    }
    if (index >= FOREST_DEPTH) {
        index = FOREST_DEPTH - 1;
    }
    map_generate_outdoor(m, level, entrances[index], exits[index]);
}

void map_generate_mountains(Map *m, int level) {
    // Reuse the branching route graphs, then reinterpret their
    // clearings as exposed basalt passes and mine chambers.
    static const OutdoorSide entrances[MOUNTAIN_DEPTH] = {
        OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_WEST
    };
    static const OutdoorSide exits[MOUNTAIN_DEPTH] = {
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_EAST,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_EAST,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH
    };
    int index = level - 1;
    if (index < 0) {
        index = 0;
    }
    if (index >= MOUNTAIN_DEPTH) {
        index = MOUNTAIN_DEPTH - 1;
    }
    map_generate_outdoor(m, level, entrances[index], exits[index]);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_FOREST_FLOOR)
                m->tiles[y][x] = TILE_MOUNTAIN_FLOOR;
            else if (m->tiles[y][x] == TILE_FOREST_WALL)
                m->tiles[y][x] = TILE_MOUNTAIN_WALL;
            else if (m->tiles[y][x] == TILE_FOREST_ENTRANCE)
                m->tiles[y][x] = TILE_MOUNTAIN_ENTRANCE;
            else if (m->tiles[y][x] == TILE_FOREST_EXIT)
                m->tiles[y][x] = TILE_MOUNTAIN_EXIT;
        }
    }
}

void map_generate_coast(Map *m, int level) {
    static const OutdoorSide entrances[COAST_DEPTH] = {
        OUTDOOR_SIDE_NORTH, OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_EAST, OUTDOOR_SIDE_NORTH, OUTDOOR_SIDE_EAST,
        OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_SOUTH
    };
    static const OutdoorSide exits[COAST_DEPTH] = {
        OUTDOOR_SIDE_EAST, OUTDOOR_SIDE_SOUTH, OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_NORTH, OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH, OUTDOOR_SIDE_EAST
    };
    int index = level - 1;
    if (index < 0) {
        index = 0;
    }
    if (index >= COAST_DEPTH) {
        index = COAST_DEPTH - 1;
    }
    map_generate_outdoor(m, level, entrances[index], exits[index]);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_FOREST_FLOOR) {
                m->tiles[y][x] = TILE_COAST_FLOOR;
            } else if (m->tiles[y][x] == TILE_FOREST_WALL) {
                m->tiles[y][x] = TILE_COAST_WALL;
            } else if (m->tiles[y][x] == TILE_FOREST_ENTRANCE) {
                m->tiles[y][x] = TILE_COAST_ENTRANCE;
            } else if (m->tiles[y][x] == TILE_FOREST_EXIT) {
                m->tiles[y][x] = TILE_COAST_EXIT;
            }
        }
    }
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

    // Forest exit at the west end of the crossroad. East and south remain
    // available for future regions.
    for (int y = 10; y <= 14; y++)
        m->tiles[y][0] = TILE_TOWN_EXIT;

    // Goblin Mountains exit at the east end of the crossroad.
    for (int y = 10; y <= 14; y++)
        m->tiles[y][TOWN_W - 1] = TILE_TOWN_EXIT;

    // Sunken Coast exit at the south end of the crossroad.
    for (int x = 18; x <= 22; x++)
        m->tiles[TOWN_H - 1][x] = TILE_TOWN_EXIT;

    // Blacksmith at (7, 7) — 5x4 tiles
    for (int dy = 0; dy < 4; dy++)
        for (int dx = 0; dx < 5; dx++)
            m->tiles[7 + dy][7 + dx] = TILE_SHOP_BLACKSMITH;

    // Alchemist at (28, 7) — 5x4 tiles
    for (int dy = 0; dy < 4; dy++)
        for (int dx = 0; dx < 5; dx++)
            m->tiles[7 + dy][28 + dx] = TILE_SHOP_ALCHEMIST;

    // Tavern at (5, 16) — 7x5 tiles. It remains decorative until quests and
    // NPC interaction are added.
    for (int dy = 0; dy < 5; dy++) {
        for (int dx = 0; dx < 7; dx++) {
            m->tiles[16 + dy][5 + dx] = TILE_TAVERN;
        }
    }

    // Spawn at the central crossroads so the south road remains unobstructed
    // for a future region.
    *spawn_x = 20;
    *spawn_y = 12;
}
