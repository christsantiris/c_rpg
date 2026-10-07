#include "map.h"
#include <stdlib.h>
#include <string.h>

void map_clear_exploration(Map *m) {
    memset(m->explored, 0, sizeof(m->explored));
}

void map_mark_explored(Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        return;
    }
    int index = y * MAP_W + x;
    m->explored[index / 8] |= (unsigned char)(1u << (index % 8));
}

int map_is_explored(const Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        return 0;
    }
    int index = y * MAP_W + x;
    return (m->explored[index / 8] & (1u << (index % 8))) != 0;
}

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

static int crypt_space_is_clear(const Map *m, int x, int y, int w, int h) {
    if (x < 1 || y < 1 || x + w >= MAP_W || y + h >= MAP_H) {
        return 0;
    }
    for (int cy = y; cy < y + h; cy++) {
        for (int cx = x; cx < x + w; cx++) {
            if (m->tiles[cy][cx] != TILE_WALL) {
                return 0;
            }
        }
    }
    return 1;
}

static int place_locked_crypt(Map *m) {
    for (int room_index = 1; room_index < m->room_count - 1; room_index++) {
        Room *room = &m->rooms[room_index];
        for (int door_y = room->y + 2;
            door_y < room->y + room->h - 2; door_y++) {
            int door_x = room->x + room->w;
            int crypt_x = door_x + 1;
            int crypt_y = door_y - 2;
            if (!crypt_space_is_clear(m, door_x, crypt_y - 1, 7, 7)) {
                continue;
            }
            fill_rect(m, crypt_x, crypt_y, 5, 5, TILE_FLOOR);
            m->tiles[door_y][door_x] = TILE_CRYPT_DOOR;
            m->tiles[crypt_y + 2][crypt_x + 2] = TILE_CRYPT_CACHE;
            for (int key_y = room->y + 1;
                key_y < room->y + room->h - 1; key_y++) {
                for (int key_x = room->x + 1;
                    key_x < room->x + room->w - 1; key_x++) {
                    if (m->tiles[key_y][key_x] == TILE_FLOOR) {
                        m->tiles[key_y][key_x] = TILE_CRYPT_KEY;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void map_generate(Map *m, int level) {
    (void)level;
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));

    // Fill with walls
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = TILE_WALL;

    m->room_count = 0;

    int target_rooms = random_range(MIN_ROOMS, MAX_ROOMS);
    int attempts = 0;

    while (m->room_count < target_rooms && attempts < 400) {
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

        // The last room holds the exit (and, on the final floor, the boss).
        if (m->room_count == target_rooms - 1) {
            int start_x, start_y, exit_x, exit_y;
            map_room_center(&m->rooms[0], &start_x, &start_y);
            map_room_center(&r, &exit_x, &exit_y);
            if (abs(exit_x - start_x) + abs(exit_y - start_y) < 45) {
                continue;
            }
        }

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

    if (level >= 2 && level < DUNGEON_DEPTH) {
        place_locked_crypt(m);
    }
}

int map_is_walkable(const Map *m, int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        return 0;
    }
    return m->tiles[y][x] != TILE_NPC_STEWARD &&
        m->tiles[y][x] != TILE_NPC_SHARPENER &&
        m->tiles[y][x] != TILE_CASTLE_WALL &&
        m->tiles[y][x] != TILE_CASTLE_TABLE &&
        m->tiles[y][x] != TILE_CASTLE_BOOKCASE &&
        m->tiles[y][x] != TILE_CASTLE_THRONE &&
        m->tiles[y][x] != TILE_CASTLE_PILLAR &&
        m->tiles[y][x] != TILE_CASTLE_BANNER &&
        m->tiles[y][x] != TILE_CASTLE_GATE &&
        m->tiles[y][x] != TILE_CATACOMBS_WALL &&
        m->tiles[y][x] != TILE_OSSUARY_BRAZIER &&
        m->tiles[y][x] != TILE_OSSUARY_COLD &&
        m->tiles[y][x] != TILE_CATACOMBS_SARCOPHAGUS &&
        m->tiles[y][x] != TILE_WALL &&
        m->tiles[y][x] != TILE_FOREST_WALL &&
        m->tiles[y][x] != TILE_FOREST_HIDDEN_TRAIL &&
        m->tiles[y][x] != TILE_MOUNTAIN_WALL &&
        m->tiles[y][x] != TILE_MOUNTAIN_CHASM &&
        m->tiles[y][x] != TILE_MOUNTAIN_GATE &&
        m->tiles[y][x] != TILE_MOUNTAIN_ROCKFALL &&
        m->tiles[y][x] != TILE_MOUNTAIN_HIDDEN_CAVE &&
        m->tiles[y][x] != TILE_COAST_WALL &&
        m->tiles[y][x] != TILE_SWAMP_WALL &&
        m->tiles[y][x] != TILE_DESERT_WALL &&
        m->tiles[y][x] != TILE_MOONVEIL_WALL &&
        m->tiles[y][x] != TILE_MOONVEIL_POOL &&
        m->tiles[y][x] != TILE_ASHEN_WALL &&
        m->tiles[y][x] != TILE_ASHEN_LAVA &&
        m->tiles[y][x] != TILE_GLASSDEEP_WALL &&
        m->tiles[y][x] != TILE_GLASSDEEP_POOL &&
        m->tiles[y][x] != TILE_FROST_WALL &&
        m->tiles[y][x] != TILE_FROST_LAKE_HOLE &&
        m->tiles[y][x] != TILE_FROST_BROKEN_ICE &&
        m->tiles[y][x] != TILE_DRAGON_WALL &&
        m->tiles[y][x] != TILE_COAST_DEEP_WATER &&
        m->tiles[y][x] != TILE_COAST_CHANNEL_WATER &&
        m->tiles[y][x] != TILE_TAVERN &&
        m->tiles[y][x] != TILE_SHOP_BLACKSMITH &&
        m->tiles[y][x] != TILE_SHOP_ALCHEMIST &&
        m->tiles[y][x] != TILE_HEALER &&
        m->tiles[y][x] != TILE_WITCH &&
        m->tiles[y][x] != TILE_WATCHTOWER &&
        m->tiles[y][x] != TILE_TAVERN_WALL &&
        m->tiles[y][x] != TILE_TAVERN_TABLE &&
        m->tiles[y][x] != TILE_NPC_ELOWEN &&
        m->tiles[y][x] != TILE_NPC_DAIN &&
        m->tiles[y][x] != TILE_NPC_ALDER &&
        m->tiles[y][x] != TILE_NPC_MARA &&
        m->tiles[y][x] != TILE_NPC_BRENNA &&
        m->tiles[y][x] != TILE_NPC_LIORA &&
        m->tiles[y][x] != TILE_NPC_ORIN &&
        m->tiles[y][x] != TILE_NPC_FROST_SURVIVOR &&
        m->tiles[y][x] != TILE_NPC_ROOK &&
        m->tiles[y][x] != TILE_NPC_INNKEEPER &&
        m->tiles[y][x] != TILE_SWAMP_DAUGHTER &&
        m->tiles[y][x] != TILE_NPC_CAIN &&
        m->tiles[y][x] != TILE_NPC_ROWAN &&
        m->tiles[y][x] != TILE_NPC_DRAGON_SEEKER &&
        m->tiles[y][x] != TILE_NPC_GUILD_SEEKER &&
        m->tiles[y][x] != TILE_NPC_ROYAL_GUARD &&
        m->tiles[y][x] != TILE_FOREST_WARDEN &&
        m->tiles[y][x] != TILE_LOCKED_DOOR &&
        m->tiles[y][x] != TILE_CRYPT_DOOR &&
        m->tiles[y][x] != TILE_ISLAND_WATER &&
        m->tiles[y][x] != TILE_ISLAND_JUNGLE &&
        m->tiles[y][x] != TILE_ISLAND_SHIP &&
        m->tiles[y][x] != TILE_ISLAND_CAMP &&
        m->tiles[y][x] != TILE_ISLAND_MARKER &&
        m->tiles[y][x] != TILE_ISLAND_STATUE &&
        m->tiles[y][x] != TILE_ISLAND_LAGOON &&
        m->tiles[y][x] != TILE_ISLAND_TEMPLE_GATE &&
        m->tiles[y][x] != TILE_NPC_ISLAND_CAPTAIN &&
        m->tiles[y][x] != TILE_NPC_ISLAND_NAHLA &&
        m->tiles[y][x] != TILE_TEMPLE_WALL &&
        m->tiles[y][x] != TILE_TEMPLE_MOON_DOOR_CLOSED &&
        m->tiles[y][x] != TILE_TEMPLE_DORMANT_SENTINEL &&
        m->tiles[y][x] != TILE_TEMPLE_VAULT_DOOR &&
        m->tiles[y][x] != TILE_LABYRINTH_WALL &&
        m->tiles[y][x] != TILE_LABYRINTH_GATE;
}

// The dungeon's portcullis shortcut is retired. Older saves can still hold its
// gate and floor switches, so they turn into plain floor.
void map_remove_dungeon_gates(Map *m) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_DUNGEON_GATE ||
                m->tiles[y][x] == TILE_DUNGEON_SWITCH_OFF ||
                m->tiles[y][x] == TILE_DUNGEON_SWITCH_ON) {
                m->tiles[y][x] = TILE_FLOOR;
            }
        }
    }
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

// The coast also uses selected clearing templates.
static const ForestTemplate forest_templates[] = {
    {7, {5,42,42,92,92,132,166}, {42,14,68,12,66,40,42}, 8,
        {{0,1},{0,2},{1,3},{2,4},{3,4},{3,5},{4,5},{5,6}}},
    {9, {5,38,38,78,78,78,125,105,166}, {42,20,65,10,42,74,42,82,42}, 11,
        {{0,1},{0,2},{1,3},{1,4},{2,4},{2,5},{3,6},{4,6},{5,6},{5,7},{6,8}}},
    {9, {5,40,40,40,94,94,94,136,166}, {42,12,42,72,12,42,72,42,42}, 12,
        {{0,1},{0,2},{0,3},{1,4},{2,5},{3,6},{1,5},{3,5},{4,7},{5,7},{6,7},{7,8}}},
    {8, {5,45,86,91,86,132,132,166}, {42,42,12,42,72,24,61,42}, 10,
        {{0,1},{1,2},{1,3},{1,4},{2,5},{3,5},{3,6},{4,6},{5,7},{6,7}}},
    {10, {5,38,38,82,82,122,122,148,145,176}, {42,16,68,10,72,18,66,42,78,42}, 11,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{5,7},{6,7},{3,4},{6,8},{7,9}}},
    {10, {5,34,34,70,70,105,105,140,140,176}, {42,16,68,12,48,18,74,28,68,42}, 10,
        {{0,1},{0,2},{1,3},{2,4},{3,4},{3,5},{4,6},{6,8},{4,7},{7,9}}},
    {10, {5,38,38,76,76,112,112,146,146,176}, {42,12,42,72,25,15,68,28,76,42}, 10,
        {{0,1},{0,2},{0,3},{1,4},{2,4},{3,6},{4,5},{4,7},{6,8},{7,9}}},
    {10, {5,36,36,72,72,108,108,142,142,176}, {42,14,70,18,66,10,76,24,66,42}, 11,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{3,4},{5,7},{6,8},{7,9},{8,9}}}
};

static const ForestTemplate mountain_templates[8] = {
    {7, {8,34,61,88,116,145,172}, {70,52,70,43,58,31,12}, 6,
        {{0,1},{1,2},{2,3},{3,4},{4,5},{5,6}}},
    {8, {18,42,42,78,78,112,145,172}, {76,20,58,20,58,40,40,40}, 9,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,5},{3,4},{5,6},{6,7}}},
    {8, {20,48,38,75,82,112,140,158}, {8,24,56,17,53,68,38,76}, 8,
        {{0,1},{0,2},{1,3},{2,4},{3,4},{4,5},{5,6},{6,7}}},
    {9, {20,48,48,82,82,116,116,150,100}, {76,18,62,18,62,18,62,40,8}, 11,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{3,4},{5,7},{6,7},{7,8},{5,6}}},
    {9, {8,38,38,72,72,108,108,144,170}, {40,14,66,14,66,14,66,40,76}, 10,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{3,4},{5,7},{6,7},{7,8}}},
    {10, {170,143,143,110,110,78,78,46,46,12}, {18,18,65,30,76,16,61,30,78,46}, 10,
        {{0,1},{1,2},{1,3},{2,4},{3,5},{4,6},{5,7},{6,8},{7,8},{8,9}}},
    {10, {18,43,43,76,76,108,108,140,140,172}, {78,25,68,12,52,25,72,12,52,8}, 11,
        {{0,1},{0,2},{1,3},{1,4},{2,4},{3,5},{4,6},{5,7},{6,8},{7,9},{8,9}}},
    {10, {8,38,38,72,72,108,108,142,142,174}, {42,12,72,12,72,12,72,25,60,76}, 12,
        {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},{3,4},{5,7},{6,8},{7,8},{7,9},{8,9}}}
};

static void carve_forest_trail(Map *m, int x, int y, int target_x, int target_y) {
    int horizontal = rand() % 2;
    while (x != target_x || y != target_y) {
        fill_rect(m, x - 1, y - 1, 3, 3, TILE_FOREST_FLOOR);
        if ((horizontal && x != target_x) || y == target_y) {
            x += target_x > x ? 1 : -1;
        } else {
            y += target_y > y ? 1 : -1;
        }
        if (rand() % 5 == 0) {
            horizontal = !horizontal;
        }
    }
    fill_rect(m, x - 1, y - 1, 3, 3, TILE_FOREST_FLOOR);
}

static void mark_hidden_forest_trail(Map *m, int x, int y, int target_x, int target_y) {
    int horizontal = 1;
    while (x != target_x || y != target_y) {
        for (int trail_y = y - 1; trail_y <= y + 1; trail_y++) {
            for (int trail_x = x - 1; trail_x <= x + 1; trail_x++) {
                if (m->tiles[trail_y][trail_x] == TILE_FOREST_WALL) {
                    m->tiles[trail_y][trail_x] = TILE_FOREST_HIDDEN_TRAIL;
                }
            }
        }
        if ((horizontal && x != target_x) || y == target_y) {
            x += target_x > x ? 1 : -1;
        } else {
            y += target_y > y ? 1 : -1;
        }
        if (rand() % 5 == 0) {
            horizontal = !horizontal;
        }
    }
}

static void carve_forest_clearing(Map *m, Room *room) {
    int center_x;
    int center_y;
    map_room_center(room, &center_x, &center_y);
    int radius_x = room->w / 2;
    int radius_y = room->h / 2;
    for (int y = room->y; y < room->y + room->h; y++) {
        for (int x = room->x; x < room->x + room->w; x++) {
            int dx = x - center_x;
            int dy = y - center_y;
            int inside = dx * dx * radius_y * radius_y +
                dy * dy * radius_x * radius_x <=
                radius_x * radius_x * radius_y * radius_y;
            if (inside) {
                m->tiles[y][x] = TILE_FOREST_FLOOR;
            }
        }
    }
}

static void map_generate_outdoor(Map *m, int level, OutdoorSide entrance_side, OutdoorSide exit_side, int organic, const ForestTemplate *layout) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++)
            m->tiles[y][x] = TILE_FOREST_WALL;

    m->room_count = layout->room_count;
    for (int i = 0; i < m->room_count; i++) {
        Room r;
        r.w = random_range(10, 17);
        r.h = random_range(8, 13);
        r.x = layout->x[i] + random_range(-3, 3);
        r.y = layout->y[i] + random_range(-3, 3);
        if (organic) {
            carve_forest_clearing(m, &r);
        } else {
            fill_rect(m, r.x, r.y, r.w, r.h, TILE_FOREST_FLOOR);
        }
        m->rooms[i] = r;
    }

    for (int i = 0; i < layout->edge_count; i++) {
        int ax, ay, bx, by;
        map_room_center(&m->rooms[layout->edges[i][0]], &ax, &ay);
        map_room_center(&m->rooms[layout->edges[i][1]], &bx, &by);
        if (organic) {
            carve_forest_trail(m, ax, ay, bx, by);
        } else {
            carve_corridor(m, ax, ay, bx, by);
        }
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

int map_forest_difficulty(int level) {
    static const int difficulty[FOREST_DEPTH] = {1, 2, 4, 8, 4, 2, 1};
    if (level < 1 || level > FOREST_DEPTH) {
        return 1;
    }
    return difficulty[level - 1];
}

void map_generate_forest(Map *m, int level) {
    static const int templates[FOREST_DEPTH] = {0, 1, 2, 7, 4, 5, 3};
    static const OutdoorSide entrances[] = {
        OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_WEST
    };
    static const OutdoorSide exits[] = {
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
    index = templates[index];
    map_generate_outdoor(m, map_forest_difficulty(level), entrances[index], exits[index], 1,
        &forest_templates[index]);
    int hidden_start_x;
    int hidden_start_y;
    int hidden_end_x;
    int hidden_end_y;
    map_room_center(&m->rooms[1], &hidden_start_x, &hidden_start_y);
    map_room_center(&m->rooms[m->room_count - 2],
        &hidden_end_x, &hidden_end_y);
    mark_hidden_forest_trail(m, hidden_start_x, hidden_start_y,
        hidden_end_x, hidden_end_y);
    int landmark_room = level == FOREST_BOSS_LEVEL ? m->room_count - 2 :
        m->room_count - 1;
    int landmark_x;
    int landmark_y;
    map_room_center(&m->rooms[landmark_room], &landmark_x, &landmark_y);
    m->tiles[landmark_y][landmark_x] = TILE_FOREST_LANDMARK;
    if (level > 1 && level < FOREST_DEPTH) {
        int false_x;
        int false_y;
        map_room_center(&m->rooms[m->room_count / 2], &false_x, &false_y);
        m->tiles[false_y][false_x] = TILE_FOREST_FALSE_MARKER;
    }
    if (m->stairs_down_x == 1) {
        m->tiles[m->stairs_down_y][0] = TILE_FOREST_WALL;
    } else if (m->stairs_down_x == MAP_W - 2) {
        m->tiles[m->stairs_down_y][MAP_W - 1] = TILE_FOREST_WALL;
    } else if (m->stairs_down_y == 1) {
        m->tiles[0][m->stairs_down_x] = TILE_FOREST_WALL;
    } else {
        m->tiles[MAP_H - 1][m->stairs_down_x] = TILE_FOREST_WALL;
    }
}

void map_reveal_forest_entrance(Map *m) {
    int x = m->stairs_up_x;
    int y = m->stairs_up_y;
    if (x == 1) {
        x = 0;
    } else if (x == MAP_W - 2) {
        x = MAP_W - 1;
    } else if (y == 1) {
        y = 0;
    } else {
        y = MAP_H - 1;
    }
    m->tiles[y][x] = TILE_FOREST_ENTRANCE;
}

void map_reveal_forest_exit(Map *m) {
    if (m->stairs_down_x == 1) {
        m->tiles[m->stairs_down_y][0] = TILE_FOREST_EXIT;
    } else if (m->stairs_down_x == MAP_W - 2) {
        m->tiles[m->stairs_down_y][MAP_W - 1] = TILE_FOREST_EXIT;
    } else if (m->stairs_down_y == 1) {
        m->tiles[0][m->stairs_down_x] = TILE_FOREST_EXIT;
    } else {
        m->tiles[MAP_H - 1][m->stairs_down_x] = TILE_FOREST_EXIT;
    }
}

static int mountain_tile_in_room(const Map *m, int x, int y) {
    for (int i = 0; i < m->room_count; i++) {
        const Room *room = &m->rooms[i];
        if (x >= room->x && x < room->x + room->w &&
            y >= room->y && y < room->y + room->h) {
            return 1;
        }
    }
    return 0;
}

static void place_mountain_cache_passage(Map *m) {
    Room *room = &m->rooms[1];
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    // Optional buried treasure can be uncovered from either end.
    for (int x = cx - 2; x <= cx + 2; x++) {
        m->tiles[cy + 2][x] = TILE_MOUNTAIN_HIDDEN_CAVE;
    }
    m->tiles[cy + 2][cx - 3] = TILE_MOUNTAIN_ROCKFALL;
    m->tiles[cy + 2][cx + 3] = TILE_MOUNTAIN_ROCKFALL;
}

int map_mountain_difficulty(int level) {
    static const int difficulty[MOUNTAIN_DEPTH] = {1, 2, 3, 8, 3, 2, 1};
    if (level < 1 || level > MOUNTAIN_DEPTH) {
        return 1;
    }
    return difficulty[level - 1];
}

void map_generate_mountains(Map *m, int level) {
    static const int templates[MOUNTAIN_DEPTH] = {0, 1, 2, 7, 4, 5, 6};
    static const OutdoorSide entrances[MOUNTAIN_DEPTH] = {
        OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_SOUTH, OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_NORTH,
        OUTDOOR_SIDE_SOUTH
    };
    static const OutdoorSide exits[MOUNTAIN_DEPTH] = {
        OUTDOOR_SIDE_NORTH, OUTDOOR_SIDE_EAST, OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_SOUTH, OUTDOOR_SIDE_SOUTH, OUTDOOR_SIDE_EAST,
        OUTDOOR_SIDE_NORTH
    };
    int index = level - 1;
    if (index < 0) {
        index = 0;
    }
    if (index >= MOUNTAIN_DEPTH) {
        index = MOUNTAIN_DEPTH - 1;
    }
    map_generate_outdoor(m, map_mountain_difficulty(level), entrances[index], exits[index], 0,
        &mountain_templates[templates[index]]);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == TILE_FOREST_FLOOR) {
                m->tiles[y][x] = TILE_MOUNTAIN_FLOOR;
            } else if (m->tiles[y][x] == TILE_FOREST_WALL) {
                m->tiles[y][x] = TILE_MOUNTAIN_WALL;
            } else if (m->tiles[y][x] == TILE_FOREST_ENTRANCE) {
                m->tiles[y][x] = TILE_MOUNTAIN_ENTRANCE;
            } else if (m->tiles[y][x] == TILE_FOREST_EXIT) {
                m->tiles[y][x] = TILE_MOUNTAIN_EXIT;
            }
        }
    }
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] != TILE_MOUNTAIN_FLOOR) {
                continue;
            }
            int in_room = mountain_tile_in_room(m, x, y);
            if ((level == 2 || level == 7) && !in_room) {
                m->tiles[y][x] = TILE_MOUNTAIN_BRIDGE;
            } else if (level == 3 || level == 6) {
                m->tiles[y][x] = TILE_MOUNTAIN_CAVE_FLOOR;
            } else if (level == 4 || level == 5) {
                if (in_room) {
                    m->tiles[y][x] = TILE_MOUNTAIN_FORTRESS_FLOOR;
                }
            }
        }
    }
    if (level == 2 || level == 7) {
        int placed_weak_bridge = 0;
        for (int y = 3; y < MAP_H - 3 && !placed_weak_bridge; y++) {
            for (int x = 1; x < MAP_W - 1; x++) {
                if (m->tiles[y][x] != TILE_MOUNTAIN_BRIDGE) {
                    continue;
                }
                int horizontal = m->tiles[y][x - 1] == TILE_MOUNTAIN_BRIDGE &&
                    m->tiles[y][x + 1] == TILE_MOUNTAIN_BRIDGE &&
                    m->tiles[y - 1][x] == TILE_MOUNTAIN_WALL &&
                    m->tiles[y + 1][x] == TILE_MOUNTAIN_WALL;
                int vertical = m->tiles[y - 1][x] == TILE_MOUNTAIN_BRIDGE &&
                    m->tiles[y + 1][x] == TILE_MOUNTAIN_BRIDGE &&
                    m->tiles[y][x - 1] == TILE_MOUNTAIN_WALL &&
                    m->tiles[y][x + 1] == TILE_MOUNTAIN_WALL;
                if ((horizontal || vertical) && x > 2 && x < MAP_W - 3) {
                    m->tiles[y][x] = TILE_MOUNTAIN_WEAK_BRIDGE;
                    placed_weak_bridge = 1;
                    break;
                }
            }
        }
    }
    if (level == 3 || level == 4 || level == 5 || level == 6) {
        place_mountain_cache_passage(m);
    }
}

int map_is_coast_tidal_tile(TileType tile) {
    return tile == TILE_COAST_DEEP_WATER || tile == TILE_COAST_DRAINED_WATER ||
        tile == TILE_COAST_CHANNEL_WATER || tile == TILE_COAST_CHANNEL_DRY;
}

int map_is_coast_object(TileType tile) {
    return tile == TILE_COAST_TIDE_CONTROL || tile == TILE_COAST_SLUICE_CONTROL ||
        tile == TILE_COAST_CACHE || tile == TILE_COAST_BEACON_UNLIT ||
        tile == TILE_COAST_BEACON_LIT;
}

TileType map_coast_swapped_tile(TileType tile) {
    switch (tile) {
        case TILE_COAST_DEEP_WATER: return TILE_COAST_DRAINED_WATER;
        case TILE_COAST_DRAINED_WATER: return TILE_COAST_DEEP_WATER;
        case TILE_COAST_CHANNEL_WATER: return TILE_COAST_CHANNEL_DRY;
        case TILE_COAST_CHANNEL_DRY: return TILE_COAST_CHANNEL_WATER;
        default: return tile;
    }
}

static void place_coast_chamber(Map *m, int room_index, TileType water) {
    int cx;
    int cy;
    map_room_center(&m->rooms[room_index], &cx, &cy);
    // Dry treasure chambers are enclosed by a tidal moat in either basin.
    for (int y = cy - 2; y <= cy + 2; y++) {
        for (int x = cx - 3; x <= cx + 3; x++) {
            int edge = x == cx - 3 || x == cx + 3 || y == cy - 2 || y == cy + 2;
            m->tiles[y][x] = edge ? water : TILE_COAST_FLOOR;
        }
    }
    // Leave the center available for Mara's first beacon on stage two.
    m->tiles[cy][cx + 1] = TILE_COAST_CACHE;
}

int map_remove_coast_sluice(Map *m) {
    if (m->room_count < 3) {
        return 0;
    }
    Room *room = &m->rooms[m->room_count / 2];
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    if (m->tiles[cy][cx + 2] != TILE_COAST_SLUICE_CONTROL) {
        return 0;
    }
    for (int y = room->y; y < room->y + room->h; y++) {
        m->tiles[y][cx] = TILE_COAST_FLOOR;
    }
    m->tiles[cy][cx + 2] = TILE_COAST_FLOOR;
    return 1;
}

TileType map_coast_trap_underlay(const Map *m, int x, int y) {
    // Match flooded-room traps to the terrain left around them by structures.
    int control_room = m->room_count / 2;
    TileType underlay = TILE_COAST_FLOOR;
    for (int room = 1; room < m->room_count - 1; room += 2) {
        if (room == control_room) {
            continue;
        }
        int cx;
        int cy;
        map_room_center(&m->rooms[room], &cx, &cy);
        if (x < cx - 4 || x > cx + 4 || y < cy - 3 || y > cy + 3) {
            continue;
        }
        if (x < cx - 1 || x > cx + 1 || y < cy - 1 || y > cy + 1) {
            underlay = TILE_COAST_SHALLOW_WATER;
        } else {
            underlay = TILE_COAST_DEEP_WATER;
            for (int ny = cy - 1; ny <= cy + 1; ny++) {
                for (int nx = cx - 1; nx <= cx + 1; nx++) {
                    if (m->tiles[ny][nx] == TILE_COAST_DRAINED_WATER) {
                        underlay = TILE_COAST_DRAINED_WATER;
                    }
                }
            }
        }
        break;
    }
    if (underlay == TILE_COAST_FLOOR) {
        return underlay;
    }
    static const int neighbors[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    int water = 0;
    int stone = 0;
    for (int i = 0; i < 4; i++) {
        int nx = x + neighbors[i][0];
        int ny = y + neighbors[i][1];
        if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
            continue;
        }
        TileType tile = m->tiles[ny][nx];
        if (tile == TILE_COAST_SHALLOW_WATER || tile == TILE_COAST_DEEP_WATER ||
            tile == TILE_COAST_CHANNEL_WATER) {
            water++;
        } else if (tile == TILE_COAST_FLOOR || tile == TILE_COAST_WALL ||
            tile == TILE_COAST_DRAINED_WATER ||
            tile == TILE_COAST_CHANNEL_DRY) {
            stone++;
        }
    }
    if (stone > water) {
        return TILE_COAST_FLOOR;
    }
    return underlay;
}

void map_generate_coast(Map *m, int level) {
    static const int templates[COAST_DEPTH] = {0, 1, 2, 5, 7};
    static const OutdoorSide entrances[COAST_DEPTH] = {
        OUTDOOR_SIDE_NORTH, OUTDOOR_SIDE_WEST, OUTDOOR_SIDE_SOUTH,
        OUTDOOR_SIDE_EAST, OUTDOOR_SIDE_SOUTH
    };
    static const OutdoorSide exits[COAST_DEPTH] = {
        OUTDOOR_SIDE_EAST, OUTDOOR_SIDE_SOUTH, OUTDOOR_SIDE_WEST,
        OUTDOOR_SIDE_SOUTH, OUTDOOR_SIDE_EAST
    };
    int index = level - 1;
    if (index < 0) {
        index = 0;
    }
    if (index >= COAST_DEPTH) {
        index = COAST_DEPTH - 1;
    }
    map_generate_outdoor(m, templates[index] + 1, entrances[index], exits[index], 0,
        &forest_templates[templates[index]]);
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
    int control_room = m->room_count / 2;
    for (int room_index = 1; room_index < m->room_count - 1; room_index += 2) {
        if (room_index == control_room) {
            continue;
        }
        int center_x;
        int center_y;
        map_room_center(&m->rooms[room_index], &center_x, &center_y);
        for (int y = center_y - 3; y <= center_y + 3; y++) {
            for (int x = center_x - 4; x <= center_x + 4; x++) {
                TileType tile = m->tiles[y][x];
                if (tile != TILE_COAST_FLOOR && tile != TILE_TRAP_HIDDEN) {
                    continue;
                }
                if (x >= center_x - 1 && x <= center_x + 1 &&
                    y >= center_y - 1 && y <= center_y + 1) {
                    // Deep water is impassable, so a trap cannot remain here.
                    m->tiles[y][x] = TILE_COAST_DEEP_WATER;
                } else if (tile == TILE_COAST_FLOOR) {
                    m->tiles[y][x] = TILE_COAST_SHALLOW_WATER;
                }
            }
        }
    }
    place_coast_chamber(m, 1, TILE_COAST_DEEP_WATER);
    place_coast_chamber(m, 2, TILE_COAST_CHANNEL_DRY);
    int control_x;
    int control_y;
    map_room_center(&m->rooms[0], &control_x, &control_y);
    m->tiles[control_y][control_x] = TILE_COAST_TIDE_CONTROL;
}

void map_place_town_harbor(Map *m) {
    for (int y = 1; y < TOWN_H - 1; y++) {
        for (int x = 1; x < TOWN_W - 1; x++) {
            if (m->tiles[y][x] == TILE_WATCHTOWER) {
                m->tiles[y][x] = TILE_TOWN_FLOOR;
            }
        }
    }
    // Retain the serialized tile ID for the closed harbor footprint.
    for (int y = TOWN_HARBOR_Y; y < TOWN_H - 1; y++) {
        for (int x = TOWN_HARBOR_X; x < TOWN_W - 1; x++) {
            m->tiles[y][x] = TILE_WATCHTOWER;
        }
    }
}

void map_place_town_tavern(Map *m) {
    for (int y = TOWN_TAVERN_Y; y < TOWN_TAVERN_Y + TOWN_TAVERN_H; y++) {
        for (int x = TOWN_TAVERN_X; x < TOWN_TAVERN_X + TOWN_TAVERN_W; x++) {
            m->tiles[y][x] = TILE_TAVERN;
        }
    }
    m->tiles[TOWN_TAVERN_DOOR_Y][TOWN_TAVERN_DOOR_X] = TILE_TAVERN_DOOR;
}

void map_place_town_inn(Map *m) {
    for (int y = TOWN_INN_Y; y < TOWN_INN_Y + TOWN_INN_H; y++) {
        for (int x = TOWN_INN_X; x < TOWN_INN_X + TOWN_INN_W; x++) {
            m->tiles[y][x] = TILE_TAVERN;
        }
    }
    m->tiles[TOWN_INN_DOOR_Y][TOWN_INN_DOOR_X] = TILE_TAVERN_DOOR;
}

void map_place_town_apothecary(Map *m) {
    // The Apothecary opens directly onto the north edge of the town square.
    for (int y = TOWN_APOTHECARY_Y; y < TOWN_APOTHECARY_Y + TOWN_APOTHECARY_H; y++) {
        for (int x = TOWN_APOTHECARY_X; x < TOWN_APOTHECARY_X + TOWN_APOTHECARY_W; x++) {
            m->tiles[y][x] = TILE_SHOP_ALCHEMIST;
        }
    }
    m->tiles[TOWN_APOTHECARY_DOOR_Y][TOWN_APOTHECARY_DOOR_X] = TILE_ALCHEMIST_DOOR;
    if (m->tiles[TOWN_APOTHECARY_DOOR_Y + 1][TOWN_APOTHECARY_DOOR_X] != TILE_ITEM) {
        m->tiles[TOWN_APOTHECARY_DOOR_Y + 1][TOWN_APOTHECARY_DOOR_X] = TILE_TOWN_PATH;
    }
}

void map_place_town3_guild(Map *m) {
    for (int y = TOWN_GUILD_Y; y < TOWN_GUILD_Y + TOWN_GUILD_H - 1; y++) {
        for (int x = TOWN_GUILD_X; x < TOWN_GUILD_X + TOWN_GUILD_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    for (int x = TOWN_GUILD_X; x < TOWN_GUILD_X + TOWN_GUILD_W; x++) {
        m->tiles[TOWN_GUILD_DOOR_Y][x] = TILE_WALL;
    }
    m->tiles[TOWN_GUILD_DOOR_Y][TOWN_GUILD_DOOR_X] = TILE_GUILD_DOOR;
    if (m->tiles[TOWN_GUILD_DOOR_Y + 1][TOWN_GUILD_DOOR_X] != TILE_ITEM) {
        m->tiles[TOWN_GUILD_DOOR_Y + 1][TOWN_GUILD_DOOR_X] = TILE_TOWN_PATH;
    }
}

void map_place_town_labyrinth(Map *m) {
    // Branch from the east-west road and skirt the gate to its south entrance.
    for (int y = 13; y <= TOWN_LABYRINTH_Y + 1; y++) {
        m->tiles[y][TOWN_LABYRINTH_X - 2] = TILE_TOWN_PATH;
    }
    for (int x = TOWN_LABYRINTH_X - 2; x <= TOWN_LABYRINTH_X; x++) {
        m->tiles[TOWN_LABYRINTH_Y + 1][x] = TILE_TOWN_PATH;
    }
    m->tiles[TOWN_LABYRINTH_Y][TOWN_LABYRINTH_X] =
        TILE_LABYRINTH_ENTRANCE;
}

// Guards only replace open grass, so loot and the player's tile stay clear.
void map_place_town3_guards(Map *m, int avoid_x, int avoid_y) {
    const int guard_y[2] = {TOWN3_GUARD_NORTH_Y, TOWN3_GUARD_SOUTH_Y};
    for (int i = 0; i < 2; i++) {
        int y = guard_y[i];
        if (m->tiles[y][TOWN3_GUARD_X] == TILE_TOWN_FLOOR &&
            (TOWN3_GUARD_X != avoid_x || y != avoid_y)) {
            m->tiles[y][TOWN3_GUARD_X] = TILE_NPC_ROYAL_GUARD;
        }
    }
}

void map_place_town4_guards(Map *m, int avoid_x, int avoid_y) {
    const int guard_y[2] = {TOWN4_KING_GATE_Y - 1, TOWN4_KING_GATE_Y + 1};
    for (int i = 0; i < 2; i++) {
        int y = guard_y[i];
        if (m->tiles[y][TOWN4_KING_GATE_X] == TILE_TOWN_FLOOR &&
            (TOWN4_KING_GATE_X != avoid_x || y != avoid_y)) {
            m->tiles[y][TOWN4_KING_GATE_X] = TILE_NPC_ROYAL_GUARD;
        }
    }
}

void map_place_town2_center(Map *m) {
    for (int x = 18; x <= 22; x++) {
        m->tiles[0][x] = TILE_TOWN_EXIT;
    }
    // Sunscar Wastes branches west from Stillbury.
    for (int y = 10; y <= 14; y++) {
        m->tiles[y][0] = TILE_TOWN_EXIT;
    }
    map_place_town2_glassdeep_gate(m);
    for (int x = TOWN_HEALER_DOOR_X; x <= TOWN_WITCH_DOOR_X; x++) {
        m->tiles[13][x] = TILE_TOWN_PATH;
    }
}

void map_place_town2_glassdeep_gate(Map *m) {
    for (int x = STILLBURY_GLASSDEEP_GATE_X - 2; x <= STILLBURY_GLASSDEEP_GATE_X + 2; x++) {
        m->tiles[TOWN_H - 1][x] = TILE_TOWN_EXIT;
    }
}

void map_generate_town(Map *m, int *spawn_x, int *spawn_y) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
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

    // Goblin Mountains exit at the north end of the crossroad.
    for (int x = 18; x <= 22; x++)
        m->tiles[0][x] = TILE_TOWN_EXIT;

    // The forest gate stays on the main road. The Stillbury spur appears later.
    for (int y = 10; y <= 12; y++) {
        m->tiles[y][0] = TILE_TOWN_EXIT;
    }

    // Dungeon exit at the east end of the crossroad.
    for (int y = 10; y <= 14; y++)
        m->tiles[y][TOWN_W - 1] = TILE_TOWN_EXIT;

    // Sunken Coast exit at the south end of the crossroad.
    for (int x = 18; x <= 22; x++)
        m->tiles[TOWN_H - 1][x] = TILE_TOWN_EXIT;

    // Blacksmith faces the east-west road.
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            m->tiles[TOWN_BLACKSMITH_Y + dy][TOWN_BLACKSMITH_X + dx] = TILE_SHOP_BLACKSMITH;
        }
    }
    m->tiles[TOWN_BLACKSMITH_Y + 3][TOWN_BLACKSMITH_X + 2] = TILE_BLACKSMITH_DOOR;
    m->tiles[TOWN_BLACKSMITH_Y + 4][TOWN_BLACKSMITH_X + 2] = TILE_TOWN_PATH;

    // The alchemist faces the east-west road.
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            m->tiles[TOWN_ALCHEMIST_Y + dy][TOWN_ALCHEMIST_X + dx] =
                TILE_SHOP_ALCHEMIST;
        }
    }
    m->tiles[TOWN_ALCHEMIST_Y + 3][TOWN_ALCHEMIST_X + 2] =
        TILE_ALCHEMIST_DOOR;
    m->tiles[TOWN_ALCHEMIST_Y + 4][TOWN_ALCHEMIST_X + 2] =
        TILE_TOWN_PATH;

    // Widen the main road into a square spanning the Blacksmith and Alchemist,
    // one row above and two rows below the road.
    for (int y = 11; y <= 14; y++) {
        for (int x = TOWN_BLACKSMITH_X; x <= TOWN_ALCHEMIST_X + 4; x++) {
            m->tiles[y][x] = TILE_TOWN_PATH;
        }
    }

    // The Tavern stands east of the Blacksmith, its door opening onto the square.
    map_place_town_tavern(m);

    map_place_town_harbor(m);
    m->tiles[TOWN_CAIN_Y][TOWN_CAIN_X] = TILE_NPC_CAIN;
    m->tiles[TOWN_ROWAN_Y][TOWN_ROWAN_X] = TILE_NPC_ROWAN;

    // Spawn at the central crossroads so the south road remains unobstructed
    // for a future region.
    *spawn_x = 20;
    *spawn_y = 12;
}

void map_set_town2_road(Map *m, int unlocked) {
    // Keep the former gate position closed.
    m->tiles[19][0] = TILE_WALL;
    for (int x = 1; x < 4; x++) {
        m->tiles[19][x] = TILE_TOWN_FLOOR;
    }
    m->tiles[TOWN_ROAD_EXIT_Y][0] = TILE_WALL;
    for (int x = 1; x < 4; x++) {
        m->tiles[TOWN_ROAD_EXIT_Y][x] = TILE_TOWN_FLOOR;
    }
    for (int y = TOWN_ROAD_EXIT_Y; y < 12; y++) {
        m->tiles[y][4] = TILE_TOWN_FLOOR;
    }
    if (!unlocked) {
        return;
    }
    m->tiles[TOWN_ROAD_EXIT_Y][0] = TILE_TOWN_EXIT;
    for (int x = 1; x < 4; x++) {
        m->tiles[TOWN_ROAD_EXIT_Y][x] = TILE_TOWN_PATH;
    }
    for (int y = TOWN_ROAD_EXIT_Y; y < 12; y++) {
        m->tiles[y][4] = TILE_TOWN_PATH;
    }
}

void map_set_stillbury_forest_road(Map *m, int unlocked) {
    m->tiles[TOWN_ROAD_EXIT_Y][TOWN_W - 1] = unlocked ? TILE_TOWN_EXIT : TILE_WALL;
    for (int y = TOWN_ROAD_EXIT_Y; y < 12; y++) {
        if (m->tiles[y][STILLBURY_FOREST_ROAD_X] != TILE_ITEM) {
            m->tiles[y][STILLBURY_FOREST_ROAD_X] = unlocked ? TILE_TOWN_PATH : TILE_TOWN_FLOOR;
        }
    }
    if (m->tiles[TOWN_ROAD_EXIT_Y][TOWN_W - 2] != TILE_ITEM) {
        m->tiles[TOWN_ROAD_EXIT_Y][TOWN_W - 2] = unlocked ? TILE_TOWN_PATH : TILE_TOWN_FLOOR;
    }
}

void map_set_town4_road(Map *m, int unlocked) {
    if (!unlocked) {
        return;
    }
    for (int y = 1; y <= 12; y++) {
        m->tiles[y][TOWN4_ROAD_X] = TILE_TOWN_PATH;
    }
    m->tiles[0][TOWN4_ROAD_X] = TILE_TOWN_EXIT;
}

void map_set_ridgeshire_mountain_road(Map *m, int unlocked) {
    m->tiles[TOWN_H - 1][RIDGESHIRE_MOUNTAIN_ROAD_X] = unlocked ? TILE_TOWN_EXIT : TILE_WALL;
    for (int y = 13; y < TOWN_H - 1; y++) {
        if (m->tiles[y][RIDGESHIRE_MOUNTAIN_ROAD_X] != TILE_ITEM) {
            m->tiles[y][RIDGESHIRE_MOUNTAIN_ROAD_X] = unlocked ? TILE_TOWN_PATH : TILE_TOWN_FLOOR;
        }
    }
}

void map_place_town4_workshop(Map *m) {
    for (int y = TOWN4_WORKSHOP_Y; y < TOWN4_WORKSHOP_DOOR_Y; y++) {
        for (int x = TOWN4_WORKSHOP_X; x < TOWN4_WORKSHOP_X + TOWN4_WORKSHOP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    for (int x = TOWN4_WORKSHOP_X; x < TOWN4_WORKSHOP_X + TOWN4_WORKSHOP_W; x++) {
        m->tiles[TOWN4_WORKSHOP_DOOR_Y][x] = TILE_WALL;
    }
    m->tiles[TOWN4_WORKSHOP_DOOR_Y][TOWN4_WORKSHOP_DOOR_X] = TILE_WORKSHOP_DOOR;
}

void map_generate_town4(Map *m, int *spawn_x, int *spawn_y) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    for (int y = 1; y < TOWN_H - 1; y++) {
        for (int x = 1; x < TOWN_W - 1; x++) {
            m->tiles[y][x] = TILE_TOWN_FLOOR;
        }
        m->tiles[y][20] = TILE_TOWN_PATH;
    }
    for (int x = 1; x < TOWN_W - 1; x++) {
        m->tiles[12][x] = TILE_TOWN_PATH;
    }
    m->tiles[TOWN_H - 1][20] = TILE_TOWN_EXIT;
    for (int y = TOWN4_DRAGON_GATE_Y - 2; y <= TOWN4_DRAGON_GATE_Y + 2; y++) {
        m->tiles[y][TOWN_W - 1] = TILE_TOWN_EXIT;
    }
    for (int y = 10; y <= 14; y++) {
        m->tiles[y][0] = TILE_TOWN_EXIT;
    }
    map_place_town4_ashen_gate(m);
    map_place_town4_workshop(m);
    map_place_town4_hall(m);
    map_place_town4_guards(m, -1, -1);
    m->tiles[TOWN4_ILYA_Y][TOWN4_ILYA_X] = TILE_NPC_DRAGON_SEEKER;
    *spawn_x = 20;
    *spawn_y = TOWN_H - 2;
}

void map_place_town4_ashen_gate(Map *m) {
    for (int x = RIDGESHIRE_ASHEN_GATE_X - 2; x <= RIDGESHIRE_ASHEN_GATE_X + 2; x++) {
        m->tiles[0][x] = TILE_TOWN_EXIT;
    }
}

void map_place_town4_hall(Map *m) {
    for (int y = TOWN4_HALL_Y; y <= TOWN4_HALL_DOOR_Y; y++) {
        for (int x = TOWN4_HALL_X; x < TOWN4_HALL_X + TOWN4_HALL_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    m->tiles[TOWN4_HALL_DOOR_Y][TOWN4_HALL_DOOR_X] = TILE_TOWN_HALL_DOOR;
    for (int y = TOWN4_HALL_DOOR_Y + 1; y <= 12; y++) {
        if (m->tiles[y][TOWN4_HALL_DOOR_X] != TILE_ITEM) {
            m->tiles[y][TOWN4_HALL_DOOR_X] = TILE_TOWN_PATH;
        }
    }
}

void map_generate_town2(Map *m, int *spawn_x, int *spawn_y) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    for (int y = 1; y < TOWN_H - 1; y++) {
        for (int x = 1; x < TOWN_W - 1; x++) {
            m->tiles[y][x] = TILE_TOWN_FLOOR;
        }
        m->tiles[y][20] = TILE_TOWN_PATH;
    }
    for (int x = 1; x < TOWN_W - 1; x++) {
        m->tiles[12][x] = TILE_TOWN_PATH;
    }
    for (int y = 10; y <= 14; y++) {
        m->tiles[y][TOWN_W - 1] = TILE_TOWN_EXIT;
    }
    for (int y = TOWN_HEALER_Y; y < TOWN_HEALER_Y + TOWN_HEALER_H; y++) {
        for (int x = TOWN_HEALER_X; x < TOWN_HEALER_X + TOWN_HEALER_W; x++) {
            m->tiles[y][x] = TILE_HEALER;
        }
    }
    m->tiles[TOWN_HEALER_DOOR_Y][TOWN_HEALER_DOOR_X] = TILE_HEALER_DOOR;
    for (int y = TOWN_WITCH_Y; y < TOWN_WITCH_Y + TOWN_WITCH_H; y++) {
        for (int x = TOWN_WITCH_X; x < TOWN_WITCH_X + TOWN_WITCH_W; x++) {
            m->tiles[y][x] = TILE_WITCH;
        }
    }
    m->tiles[TOWN_WITCH_DOOR_Y][TOWN_WITCH_DOOR_X] = TILE_WITCH_DOOR;
    for (int x = TOWN_HEALER_DOOR_X; x <= TOWN_WITCH_DOOR_X; x++) {
        m->tiles[11][x] = TILE_TOWN_PATH;
    }
    map_place_town2_center(m);
    // The Inn stands east of the Healer, its door opening onto the square.
    map_place_town_inn(m);
    map_place_town_labyrinth(m);
    *spawn_x = TOWN_W - 2;
    *spawn_y = 12;
}

static void map_generate_town_square(Map *m) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = x > 0 && x < TOWN_W - 1 &&
                y > 0 && y < TOWN_H - 1 ? TILE_TOWN_FLOOR : TILE_WALL;
        }
    }
    for (int y = 1; y < TOWN_H - 1; y++) {
        m->tiles[y][CROWNROAD_X] = TILE_TOWN_PATH;
    }
    for (int x = 1; x < TOWN_W - 1; x++) {
        m->tiles[CASTLE_ROAD_Y][x] = TILE_TOWN_PATH;
    }
    for (int y = 12; y <= 15; y++) {
        for (int x = 12; x <= 28; x++) {
            m->tiles[y][x] = TILE_TOWN_PATH;
        }
    }
}

void map_generate_town3(Map *m, int *spawn_x, int *spawn_y) {
    map_generate_town_square(m);
    map_place_town3_frost_gate(m);
    map_place_town3_moonveil_gate(m);
    m->tiles[TOWN_H - 1][20] = TILE_TOWN_EXIT;
    for (int y = TOWN3_KING_GATE_Y - 2; y <= TOWN3_KING_GATE_Y + 2; y++) {
        m->tiles[y][TOWN_W - 1] = TILE_TOWN_EXIT;
    }
    map_place_town_apothecary(m);
    map_place_town3_guild(m);
    map_place_town3_guards(m, -1, -1);
    *spawn_x = 20;
    *spawn_y = TOWN_H - 2;
}

void map_place_town3_moonveil_gate(Map *m) {
    for (int y = ROSEMOOR_MOONVEIL_GATE_Y - 2; y <= ROSEMOOR_MOONVEIL_GATE_Y + 2; y++) {
        m->tiles[y][0] = TILE_TOWN_EXIT;
    }
}

void map_place_town3_frost_gate(Map *m) {
    for (int x = 18; x <= 22; x++) {
        m->tiles[0][x] = TILE_TOWN_EXIT;
    }
}

void map_generate_castle(Map *m, int *spawn_x, int *spawn_y) {
    map_generate_town_square(m);
    for (int y = 1; y < TOWN_MOAT_Y; y++) {
        m->tiles[y][CROWNROAD_X] = TILE_TOWN_FLOOR;
    }
    for (int y = TOWN_MOAT_Y; y < TOWN_MOAT_Y + TOWN_MOAT_H; y++) {
        for (int x = TOWN_MOAT_X; x < TOWN_MOAT_X + TOWN_MOAT_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    m->tiles[10][CROWNROAD_X] = TILE_TOWN_EXIT;
    m->tiles[11][CROWNROAD_X] = TILE_TOWN_PATH;
    for (int y = CASTLE_ROAD_Y - 2; y <= CASTLE_ROAD_Y + 2; y++) {
        m->tiles[y][0] = TILE_TOWN_EXIT;
        m->tiles[y][TOWN_W - 1] = TILE_TOWN_EXIT;
    }
    for (int y = CASTLE_ROAD_Y; y < TOWN_H - 1; y++) {
        m->tiles[y][CROWNROAD_X] = TILE_TOWN_PATH;
    }
    m->tiles[TOWN_H - 1][CROWNROAD_X] = TILE_TOWN_EXIT;
    *spawn_x = CROWNROAD_X;
    *spawn_y = CASTLE_ROAD_Y;
}

void map_set_town3_road(Map *m, int unlocked) {
    if (!unlocked) {
        return;
    }
    for (int y = 1; y <= 12; y++) {
        m->tiles[y][TOWN3_ROAD_X] = TILE_TOWN_PATH;
    }
    m->tiles[0][TOWN3_ROAD_X] = TILE_TOWN_EXIT;
}

void map_set_rosemoor_swamp_road(Map *m, int unlocked) {
    m->tiles[TOWN_H - 1][ROSEMOOR_SWAMP_ROAD_X] = unlocked ? TILE_TOWN_EXIT : TILE_WALL;
    for (int y = 15; y < TOWN_H - 1; y++) {
        if (m->tiles[y][ROSEMOOR_SWAMP_ROAD_X] != TILE_ITEM) {
            m->tiles[y][ROSEMOOR_SWAMP_ROAD_X] = unlocked ? TILE_TOWN_PATH : TILE_TOWN_FLOOR;
        }
    }
}

void map_generate_swamp_road(Map *m) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_SWAMP_WALL;
        }
    }
    for (int y = 1; y < SWAMP_ROAD_H - 1; y++) {
        m->tiles[y][SWAMP_ROAD_X] = TILE_SWAMP_FLOOR;
    }
    m->stairs_up_x = SWAMP_ROAD_X;
    m->stairs_up_y = SWAMP_ROAD_H - 1;
    m->stairs_down_x = SWAMP_ROAD_X;
    m->stairs_down_y = 0;
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_SWAMP_ENTRANCE;
    m->tiles[m->stairs_down_y][m->stairs_down_x] = TILE_SWAMP_EXIT;
}

void map_generate_crownroad(Map *m) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    // Rotate the authored ambush so travel runs west to east.
    for (int x = 1; x < CROWNROAD_W - 1; x++) {
        int old_y = CROWNROAD_W - 1 - x;
        int top_tree_edge = 12 + ((old_y / 6) % 3 == 1);
        int bottom_tree_edge = 28 - ((old_y / 7) % 3 == 1);
        for (int y = 1; y < CROWNROAD_H - 1; y++) {
            m->tiles[y][x] = y <= top_tree_edge || y >= bottom_tree_edge ?
                TILE_FOREST_WALL : TILE_TOWN_FLOOR;
        }
        m->tiles[CROWNROAD_Y][x] = TILE_TOWN_PATH;
    }
    for (int branch = 0; branch < 4; branch++) {
        int x = CROWNROAD_W - 1 - (9 + branch * 10);
        int from = branch % 2 == 0 ? 17 : CROWNROAD_Y;
        int to = branch % 2 == 0 ? CROWNROAD_Y : 23;
        for (int y = from; y <= to; y++) {
            m->tiles[y][x] = TILE_TOWN_PATH;
        }
        for (int y = from - 2; y <= from - 1; y++) {
            m->tiles[y][x + 2] = TILE_WALL;
        }
        for (int y = to + 1; y <= to + 2; y++) {
            m->tiles[y][x - 3] = TILE_WALL;
        }
    }
    for (int old_y = 5; old_y < CROWNROAD_W - 1; old_y += 8) {
        m->tiles[15][CROWNROAD_W - 1 - old_y] = TILE_FOREST_WALL;
        if (old_y + 4 < CROWNROAD_W - 1) {
            m->tiles[25][CROWNROAD_W - 1 - old_y - 4] = TILE_FOREST_WALL;
        }
    }
    m->tiles[CROWNROAD_Y][0] = TILE_TOWN_EXIT;
    m->tiles[CROWNROAD_Y][CROWNROAD_W - 1] = TILE_TOWN_EXIT;
    m->stairs_up_x = 1;
    m->stairs_up_y = CROWNROAD_Y;
    m->stairs_down_x = CROWNROAD_W - 2;
    m->stairs_down_y = CROWNROAD_Y;
}

static void swamp_carve(Map *m, int x, int y) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px > 0 && px < SWAMP_MAP_W - 1 &&
                py > 0 && py < SWAMP_MAP_H - 1 &&
                (dx == 0 || dy == 0 || rand() % 3 != 0)) {
                m->tiles[py][px] = TILE_SWAMP_FLOOR;
            }
        }
    }
}

static void swamp_carve_clearing(Map *m, const Room *room) {
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    int rx = room->w / 2;
    int ry = room->h / 2;
    int limit = rx * rx * ry * ry;
    for (int y = room->y; y < room->y + room->h; y++) {
        int dy = y - cy;
        int span = rx;
        while (span > 0 &&
            span * span * ry * ry + dy * dy * rx * rx > limit) {
            span--;
        }
        unsigned int seed = (unsigned int)room->x * 73856093u ^
            (unsigned int)y * 19349663u;
        seed ^= seed >> 16;
        int left = span + (int)(seed % 3u) - 1;
        int right = span + (int)((seed >> 5) % 3u) - 1;
        if (left < 0) {
            left = 0;
        }
        if (right < 0) {
            right = 0;
        }
        for (int x = cx - left; x <= cx + right; x++) {
            if (x >= room->x && x < room->x + room->w) {
                m->tiles[y][x] = TILE_SWAMP_FLOOR;
            }
        }
    }
}

static void swamp_carve_trail(Map *m, int x, int y, int tx, int ty, int turn) {
    while (x != tx || y != ty) {
        swamp_carve(m, x, y);
        if (rand() % 4 == 0) {
            turn = !turn;
        }
        if ((turn && x != tx) || y == ty) {
            x += x < tx ? 1 : -1;
        } else {
            y += y < ty ? 1 : -1;
        }
    }
    swamp_carve(m, tx, ty);
}

int map_swamp_difficulty(int level) {
    static const int difficulty[SWAMP_DEPTH] = {1, 2, 3, 5, 3, 2, 1};
    if (level < 1 || level > SWAMP_DEPTH) {
        return 1;
    }
    return difficulty[level - 1];
}

void map_generate_swamp(Map *m, int level) {
    static const int route[9] = {0, 1, 2, 5, 4, 3, 6, 7, 8};
    static const int anchor_x[9] = {5, 28, 50, 7, 29, 53, 4, 26, 52};
    static const int anchor_y[9] = {8, 13, 4, 25, 31, 21, 45, 44, 42};
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 9;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_SWAMP_WALL;
        }
    }
    for (int i = 0; i < 9; i++) {
        Room *room = &m->rooms[i];
        room->x = anchor_x[i] + rand() % 7 - 3;
        room->y = anchor_y[i] + rand() % 7 - 3;
        room->w = 12 + rand() % 5;
        room->h = 9 + rand() % 4;
        swamp_carve_clearing(m, room);
    }
    for (int i = 1; i < 9; i++) {
        int x;
        int y;
        int tx;
        int ty;
        map_room_center(&m->rooms[route[i - 1]], &x, &y);
        map_room_center(&m->rooms[route[i]], &tx, &ty);
        swamp_carve_trail(m, x, y, tx, ty, (level + i) % 2);
    }
    int start_x;
    int end_x;
    map_room_center(&m->rooms[0], &start_x, &m->stairs_up_y);
    map_room_center(&m->rooms[8], &end_x, &m->stairs_down_y);
    m->stairs_up_x = 0;
    m->stairs_down_x = SWAMP_MAP_W - 1;
    swamp_carve_trail(m, start_x, m->stairs_up_y,
        m->stairs_up_x, m->stairs_up_y, 0);
    swamp_carve_trail(m, end_x, m->stairs_down_y,
        m->stairs_down_x, m->stairs_down_y, 0);
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        m->tiles[y][0] = TILE_SWAMP_WALL;
        m->tiles[y][SWAMP_MAP_W - 1] = TILE_SWAMP_WALL;
    }
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_SWAMP_ENTRANCE;
    m->tiles[m->stairs_down_y][m->stairs_down_x] = TILE_SWAMP_EXIT;
}

void map_generate_high_pass(Map *m) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_DRAGON_WALL;
        }
    }
    for (int y = 1; y < HIGH_PASS_H - 1; y++) {
        m->tiles[y][HIGH_PASS_X] = TILE_DRAGON_FLOOR;
    }
    m->stairs_up_x = HIGH_PASS_X;
    m->stairs_up_y = HIGH_PASS_H - 1;
    m->stairs_down_x = HIGH_PASS_X;
    m->stairs_down_y = 0;
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_HIGH_PASS_ENTRANCE;
    m->tiles[m->stairs_down_y][m->stairs_down_x] = TILE_HIGH_PASS_EXIT;
}

static int place_lake_hole(Map *m, int x, int y) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (m->tiles[y + dy][x + dx] != TILE_FROST_LAKE) {
                return 0;
            }
        }
    }
    m->tiles[y][x] = TILE_FROST_LAKE_HOLE;
    return 1;
}

static void freeze_kraken_lake(Map *m) {
    // The last clearing freezes into the Polar Kraken's lake, with up to six
    // holes around the centre where its tentacles break through. A hole needs
    // open ice on all eight sides so it can never cut off part of the lake.
    static const int hole_dx[6] = {-3, 3, -4, 4, -1, 1};
    static const int hole_dy[6] = {-2, -2, 1, 1, 3, 3};
    const Room *lake = &m->rooms[m->room_count - 1];
    for (int y = lake->y; y < lake->y + lake->h; y++) {
        for (int x = lake->x; x < lake->x + lake->w; x++) {
            if (x > 0 && x < SWAMP_MAP_W - 1 && y > 0 && y < SWAMP_MAP_H - 1 &&
                m->tiles[y][x] == TILE_FROST_FLOOR) {
                m->tiles[y][x] = TILE_FROST_LAKE;
            }
        }
    }
    int cx;
    int cy;
    map_room_center(lake, &cx, &cy);
    int holes = 0;
    for (int i = 0; i < 6; i++) {
        holes += place_lake_hole(m, cx + hole_dx[i], cy + hole_dy[i]);
    }
    // Irregular clearings can block the usual spots, so search outward from
    // the Kraken until the lake has at least four holes.
    for (int radius = 2; radius <= 8 && holes < 4; radius++) {
        for (int dy = -radius; dy <= radius && holes < 4; dy++) {
            for (int dx = -radius; dx <= radius && holes < 4; dx++) {
                if ((dx == -radius || dx == radius || dy == -radius || dy == radius) &&
                    cx + dx > lake->x && cx + dx < lake->x + lake->w - 1 &&
                    cy + dy > lake->y && cy + dy < lake->y + lake->h - 1) {
                    holes += place_lake_hole(m, cx + dx, cy + dy);
                }
            }
        }
    }
}

// A slick-ice patch fits when the patch and a one-tile ring around it lie
// inside the clearing's interior and are all snow. The ring means every slide
// ends on snow and the player can always walk round the patch instead.
static int ice_patch_fits(const Map *m, const Room *r, int px, int py, int pw, int ph) {
    if (px - 1 < r->x + 1 || py - 1 < r->y + 1 || px + pw > r->x + r->w - 2 ||
        py + ph > r->y + r->h - 2) {
        return 0;
    }
    for (int y = py - 1; y <= py + ph; y++) {
        for (int x = px - 1; x <= px + pw; x++) {
            if (m->tiles[y][x] != TILE_FROST_FLOOR) {
                return 0;
            }
        }
    }
    return 1;
}

// Places a 5x3 patch in a clearing, as near its centre as it fits.
static int place_ice_patch(Map *m, const Room *r) {
    const int pw = 5;
    const int ph = 3;
    int cx;
    int cy;
    map_room_center(r, &cx, &cy);
    int best_x = -1;
    int best_y = -1;
    int best_distance = 0;
    for (int py = r->y; py < r->y + r->h; py++) {
        for (int px = r->x; px < r->x + r->w; px++) {
            if (!ice_patch_fits(m, r, px, py, pw, ph)) {
                continue;
            }
            int distance = abs(px + pw / 2 - cx) + abs(py + ph / 2 - cy);
            if (best_x < 0 || distance < best_distance) {
                best_x = px;
                best_y = py;
                best_distance = distance;
            }
        }
    }
    if (best_x < 0) {
        return 0;
    }
    for (int y = best_y; y < best_y + ph; y++) {
        for (int x = best_x; x < best_x + pw; x++) {
            m->tiles[y][x] = TILE_FROST_ICE;
        }
    }
    return 1;
}

// Stage N (2-5) gets up to N-1 slick-ice patches, at most one per clearing and
// never in the first or last clearing (the stage-5 lake). Placement draws no
// random numbers, so the rest of the stage's terrain is unchanged.
static void place_slick_ice_patches(Map *m, int level) {
    int candidates = m->room_count - 2;
    if (level < 2 || candidates <= 0) {
        return;
    }
    int start = (m->rooms[0].x + m->rooms[0].y + level) % candidates;
    int placed = 0;
    for (int k = 0; k < candidates && placed < level - 1; k++) {
        placed += place_ice_patch(m, &m->rooms[1 + (start + k) % candidates]);
    }
}

static TileType frost_line_tile(const Map *m, int vertical, int line, int along) {
    return vertical ? m->tiles[along][line] : m->tiles[line][along];
}

// Tries one straight line between two clearings, on a column (vertical) or a
// row: from the last snow inside room a's bounds to the first snow inside room
// b's. Only those two end tiles touch open ground. The 2-14 tiles between must
// all be wall with wall on both sides, so nothing along the tunnel borders a
// trail, a clearing or another tunnel.
static int carve_thin_ice_line(Map *m, const Room *a, const Room *b, int vertical, int line) {
    int a_start = vertical ? a->y : a->x;
    int a_end = vertical ? a->y + a->h : a->x + a->w;
    int b_start = vertical ? b->y : b->x;
    int b_end = vertical ? b->y + b->h : b->x + b->w;
    int ea = -1;
    for (int t = a_start; t < a_end; t++) {
        if (frost_line_tile(m, vertical, line, t) == TILE_FROST_FLOOR) {
            ea = t;
        }
    }
    int eb = -1;
    for (int t = b_end - 1; t >= b_start; t--) {
        if (frost_line_tile(m, vertical, line, t) == TILE_FROST_FLOOR) {
            eb = t;
        }
    }
    int gap = eb - ea - 1;
    if (ea < 0 || eb < 0 || gap < 2 || gap > 14) {
        return 0;
    }
    for (int t = ea + 1; t < eb; t++) {
        if (frost_line_tile(m, vertical, line, t) != TILE_FROST_WALL ||
            frost_line_tile(m, vertical, line - 1, t) != TILE_FROST_WALL ||
            frost_line_tile(m, vertical, line + 1, t) != TILE_FROST_WALL) {
            return 0;
        }
    }
    for (int t = ea + 1; t < eb; t++) {
        if (vertical) {
            m->tiles[t][line] = TILE_FROST_THIN_ICE;
        } else {
            m->tiles[line][t] = TILE_FROST_THIN_ICE;
        }
    }
    return 1;
}

// Runs a tunnel north-south when the clearings share columns, otherwise
// east-west when they share rows, trying lines from the middle outwards.
static int carve_thin_ice_shortcut(Map *m, const Room *a, const Room *b) {
    int vertical = 1;
    int lo = a->x > b->x ? a->x : b->x;
    int hi = (a->x + a->w < b->x + b->w ? a->x + a->w : b->x + b->w) - 1;
    int max_line = SWAMP_MAP_W - 3;
    if (lo > hi) {
        vertical = 0;
        lo = a->y > b->y ? a->y : b->y;
        hi = (a->y + a->h < b->y + b->h ? a->y + a->h : b->y + b->h) - 1;
        max_line = SWAMP_MAP_H - 3;
    }
    lo = lo < 2 ? 2 : lo;
    hi = hi > max_line ? max_line : hi;
    if (lo > hi) {
        return 0;
    }
    int a_first = vertical ? a->y <= b->y : a->x <= b->x;
    const Room *first = a_first ? a : b;
    const Room *second = a_first ? b : a;
    int mid = (lo + hi) / 2;
    for (int offset = 0; offset <= 2 * (hi - lo); offset++) {
        int line = offset % 2 == 0 ? mid + offset / 2 : mid - (offset + 1) / 2;
        if (line >= lo && line <= hi && carve_thin_ice_line(m, first, second, vertical, line)) {
            return 1;
        }
    }
    return 0;
}

// Stage N (3-5) gets up to N-2 thin-ice shortcuts between clearings that are
// not next to each other on the route, never to the stage-5 lake. Each is an
// extra path carved through walls, so the stage stays fully connected after
// every shortcut has collapsed.
static void place_thin_ice_shortcuts(Map *m, int level) {
    static const int pairs[5][2] = {{1, 4}, {4, 7}, {0, 3}, {1, 5}, {5, 8}};
    if (level < 3 || m->room_count < 9) {
        return;
    }
    int lake = level == FROSTFELL_DEPTH ? m->room_count - 1 : -1;
    int placed = 0;
    for (int i = 0; i < 5 && placed < level - 2; i++) {
        if (pairs[i][0] == lake || pairs[i][1] == lake) {
            continue;
        }
        placed += carve_thin_ice_shortcut(m, &m->rooms[pairs[i][0]], &m->rooms[pairs[i][1]]);
    }
}

// Frostfell follows the swamp's layout, mirrored to run east to west, then
// snows over its tiles.
void map_generate_frostfell(Map *m, int level) {
    map_generate_swamp(m, level);
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W / 2; x++) {
            TileType tile = m->tiles[y][x];
            m->tiles[y][x] = m->tiles[y][SWAMP_MAP_W - 1 - x];
            m->tiles[y][SWAMP_MAP_W - 1 - x] = tile;
        }
    }
    for (int i = 0; i < m->room_count; i++) {
        m->rooms[i].x = SWAMP_MAP_W - m->rooms[i].x - m->rooms[i].w;
    }
    m->stairs_up_x = SWAMP_MAP_W - 1 - m->stairs_up_x;
    m->stairs_down_x = SWAMP_MAP_W - 1 - m->stairs_down_x;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = m->tiles[y][x];
            if (tile == TILE_SWAMP_WALL) {
                m->tiles[y][x] = TILE_FROST_WALL;
            } else if (tile == TILE_SWAMP_FLOOR) {
                m->tiles[y][x] = TILE_FROST_FLOOR;
            } else if (tile == TILE_SWAMP_ENTRANCE) {
                m->tiles[y][x] = TILE_FROST_ENTRANCE;
            } else if (tile == TILE_SWAMP_EXIT) {
                m->tiles[y][x] = TILE_FROST_EXIT;
            }
        }
    }
    if (level == FROSTFELL_DEPTH) {
        freeze_kraken_lake(m);
    }
    place_thin_ice_shortcuts(m, level);
    place_slick_ice_patches(m, level);
}

// Reuse connected clearings, mirrored east to west, as sand between rocks.
void map_generate_desert(Map *m, int level) {
    map_generate_swamp(m, level);
    for (int y = 0; y < DESERT_MAP_H; y++) {
        for (int x = 0; x < DESERT_MAP_W / 2; x++) {
            TileType tile = m->tiles[y][x];
            m->tiles[y][x] = m->tiles[y][DESERT_MAP_W - 1 - x];
            m->tiles[y][DESERT_MAP_W - 1 - x] = tile;
        }
    }
    for (int i = 0; i < m->room_count; i++) {
        m->rooms[i].x = DESERT_MAP_W - m->rooms[i].x - m->rooms[i].w;
    }
    m->stairs_up_x = DESERT_MAP_W - 1 - m->stairs_up_x;
    m->stairs_down_x = DESERT_MAP_W - 1 - m->stairs_down_x;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = m->tiles[y][x];
            if (tile == TILE_SWAMP_WALL) {
                m->tiles[y][x] = TILE_DESERT_WALL;
            } else if (tile == TILE_SWAMP_FLOOR) {
                m->tiles[y][x] = TILE_DESERT_FLOOR;
            } else if (tile == TILE_SWAMP_ENTRANCE) {
                m->tiles[y][x] = TILE_DESERT_ENTRANCE;
            } else if (tile == TILE_SWAMP_EXIT) {
                m->tiles[y][x] = TILE_DESERT_EXIT;
            }
        }
    }
}

void map_generate_moonveil(Map *m, int level) {
    // Connected garden clearings run east to west from Rosemoor.
    map_generate_desert(m, level);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = m->tiles[y][x];
            if (tile == TILE_DESERT_FLOOR) {
                m->tiles[y][x] = TILE_MOONVEIL_FLOOR;
            } else if (tile == TILE_DESERT_WALL) {
                m->tiles[y][x] = TILE_MOONVEIL_WALL;
            } else if (tile == TILE_DESERT_ENTRANCE) {
                m->tiles[y][x] = TILE_MOONVEIL_ENTRANCE;
            } else if (tile == TILE_DESERT_EXIT) {
                m->tiles[y][x] = TILE_MOONVEIL_EXIT;
            }
        }
    }
    // Pools replace only solid hedge patches, preserving every travel path.
    for (int cy = 7; cy < SWAMP_MAP_H - 7; cy += 13) {
        for (int cx = 7; cx < SWAMP_MAP_W - 7; cx += 14) {
            int enclosed = 1;
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    enclosed &= m->tiles[cy + dy][cx + dx] == TILE_MOONVEIL_WALL;
                }
            }
            if (!enclosed) {
                continue;
            }
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    if (dx * dx + dy * dy <= 5) {
                        m->tiles[cy + dy][cx + dx] = TILE_MOONVEIL_POOL;
                    }
                }
            }
        }
    }
    for (int i = 1; i < m->room_count; i++) {
        if (i % 2 == 0 && i != m->room_count - 1) {
            continue;
        }
        int cx;
        int cy;
        map_room_center(&m->rooms[i], &cx, &cy);
        int radius = i == m->room_count - 1 ? 3 : 2;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int distance = dx * dx + dy * dy;
                if (distance >= radius * radius - 2 && distance <= radius * radius + 1 &&
                    m->tiles[cy + dy][cx + dx] == TILE_MOONVEIL_FLOOR) {
                    m->tiles[cy + dy][cx + dx] = TILE_MOONVEIL_CIRCLE;
                }
            }
        }
    }
}

void map_generate_ashen(Map *m, int level) {
    static const int route[9] = {6, 7, 8, 5, 4, 3, 0, 1, 2};
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 9;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_SWAMP_WALL;
        }
    }
    for (int i = 0; i < m->room_count; i++) {
        Room *room = &m->rooms[i];
        int anchor = route[i];
        room->x = 5 + anchor % 3 * 23 + rand() % 5 - 2;
        room->y = 5 + anchor / 3 * 20 + rand() % 5 - 2;
        room->w = 12 + rand() % 5;
        room->h = 9 + rand() % 4;
        swamp_carve_clearing(m, room);
        if (i > 0) {
            int x;
            int y;
            int tx;
            int ty;
            map_room_center(&m->rooms[i - 1], &x, &y);
            map_room_center(room, &tx, &ty);
            swamp_carve_trail(m, x, y, tx, ty, (level + i) % 2);
        }
    }
    int start_y;
    int end_y;
    map_room_center(&m->rooms[0], &m->stairs_up_x, &start_y);
    map_room_center(&m->rooms[m->room_count - 1], &m->stairs_down_x, &end_y);
    m->stairs_up_y = SWAMP_MAP_H - 1;
    m->stairs_down_y = 0;
    swamp_carve_trail(m, m->stairs_up_x, start_y, m->stairs_up_x, m->stairs_up_y, 1);
    swamp_carve_trail(m, m->stairs_down_x, end_y, m->stairs_down_x, 0, 1);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = m->tiles[y][x] == TILE_SWAMP_FLOOR ? TILE_ASHEN_FLOOR : TILE_ASHEN_WALL;
        }
    }
    m->tiles[m->stairs_up_y][m->stairs_up_x] = TILE_ASHEN_ENTRANCE;
    m->tiles[m->stairs_down_y][m->stairs_down_x] = TILE_ASHEN_EXIT;
    // Lava replaces solid rock only, leaving the connected route intact.
    for (int row = 8; row < SWAMP_MAP_H - 4; row += 11) {
        for (int column = 8; column < SWAMP_MAP_W - 4; column += 13) {
            int cx = column + rand() % 7 - 3;
            int cy = row + rand() % 7 - 3;
            int solid = 1;
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    solid &= m->tiles[cy + dy][cx + dx] == TILE_ASHEN_WALL;
                }
            }
            if (!solid) {
                continue;
            }
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    if (dx * dx + dy * dy <= 5) {
                        m->tiles[cy + dy][cx + dx] = TILE_ASHEN_LAVA;
                    }
                }
            }
        }
    }
    for (int i = 1; i < m->room_count; i += 2) {
        const Room *room = &m->rooms[i];
        for (int x = room->x + 2; x < room->x + room->w - 2; x++) {
            int y = room->y + room->h / 2 + 2;
            if (m->tiles[y][x] == TILE_ASHEN_FLOOR) {
                m->tiles[y][x] = TILE_ASHEN_RUIN;
            }
        }
    }
    if (level == ASHEN_DEPTH) {
        int x;
        int y;
        map_room_center(&m->rooms[m->room_count - 1], &x, &y);
        m->tiles[y][x] = TILE_ASHEN_RUIN;
    }
}

void map_generate_glassdeep(Map *m, int level) {
    // Reuse connected irregular chambers, mirrored to descend north to south.
    map_generate_ashen(m, level);
    for (int y = 0; y < SWAMP_MAP_H / 2; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            TileType tile = m->tiles[y][x];
            m->tiles[y][x] = m->tiles[SWAMP_MAP_H - 1 - y][x];
            m->tiles[SWAMP_MAP_H - 1 - y][x] = tile;
        }
    }
    for (int i = 0; i < m->room_count; i++) {
        m->rooms[i].y = SWAMP_MAP_H - m->rooms[i].y - m->rooms[i].h;
    }
    m->stairs_up_y = SWAMP_MAP_H - 1 - m->stairs_up_y;
    m->stairs_down_y = SWAMP_MAP_H - 1 - m->stairs_down_y;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            switch (m->tiles[y][x]) {
                case TILE_ASHEN_FLOOR: m->tiles[y][x] = TILE_GLASSDEEP_FLOOR; break;
                case TILE_ASHEN_WALL: m->tiles[y][x] = TILE_GLASSDEEP_WALL; break;
                case TILE_ASHEN_ENTRANCE: m->tiles[y][x] = TILE_GLASSDEEP_ENTRANCE; break;
                case TILE_ASHEN_EXIT: m->tiles[y][x] = TILE_GLASSDEEP_EXIT; break;
                case TILE_ASHEN_LAVA: m->tiles[y][x] = TILE_GLASSDEEP_POOL; break;
                case TILE_ASHEN_RUIN: m->tiles[y][x] = TILE_GLASSDEEP_RUIN; break;
                default: break;
            }
        }
    }
    if (level == GLASSDEEP_DEPTH) {
        int cx;
        int cy;
        map_room_center(&m->rooms[m->room_count - 1], &cx, &cy);
        for (int y = cy - 3; y <= cy + 3; y++) {
            for (int x = cx - 3; x <= cx + 3; x++) {
                if (m->tiles[y][x] == TILE_GLASSDEEP_FLOOR) {
                    m->tiles[y][x] = TILE_GLASSDEEP_RUIN;
                }
            }
        }
    }
}

void map_generate_dragonspine(Map *m, int level) {
    map_generate_swamp(m, level);
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType tile = m->tiles[y][x];
            if (tile == TILE_SWAMP_WALL) {
                m->tiles[y][x] = TILE_DRAGON_WALL;
            } else if (tile == TILE_SWAMP_ENTRANCE) {
                m->tiles[y][x] = TILE_DRAGON_ENTRANCE;
            } else if (tile == TILE_SWAMP_EXIT) {
                m->tiles[y][x] = TILE_DRAGON_EXIT;
            } else if (tile == TILE_SWAMP_FLOOR) {
                int ash = level >= 3 && ((x * 3 + y + level) % 5 < level - 2);
                m->tiles[y][x] = ash ? TILE_DRAGON_ASH : TILE_DRAGON_FLOOR;
            }
        }
    }
    if (level == DRAGONSPINE_DEPTH) {
        Room *lair = &m->rooms[m->room_count - 1];
        for (int y = lair->y + 1; y < lair->y + lair->h - 1; y++) {
            for (int x = lair->x + 1; x < lair->x + lair->w - 1; x++) {
                if ((x + y) % 3 == 0 &&
                    (m->tiles[y][x] == TILE_DRAGON_FLOOR ||
                    m->tiles[y][x] == TILE_DRAGON_ASH)) {
                    m->tiles[y][x] = TILE_DRAGON_HOARD;
                }
            }
        }
    }
}

void map_generate_forest_road(Map *m) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_FOREST_WALL;
        }
    }
    for (int x = 1; x < FOREST_ROAD_W - 1; x++) {
        m->tiles[FOREST_ROAD_Y][x] = TILE_FOREST_FLOOR;
    }
    m->tiles[FOREST_ROAD_Y][0] = TILE_FOREST_ENTRANCE;
    m->tiles[FOREST_ROAD_Y][FOREST_ROAD_W - 1] = TILE_FOREST_EXIT;
    m->stairs_up_x = 1;
    m->stairs_up_y = FOREST_ROAD_Y;
    m->stairs_down_x = FOREST_ROAD_W - 2;
    m->stairs_down_y = FOREST_ROAD_Y;
}

static unsigned int labyrinth_random(unsigned int *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

void map_generate_labyrinth(Map *m, int level, int switches, int *spawn_x, int *spawn_y) {
    enum { MAX_CELLS_W = 16, MAX_CELLS_H = 10, CELL_COUNT = 160 };
    int cells_w = 10 + level * 2;
    int cells_h = 7 + level;
    if (cells_w > MAX_CELLS_W) {
        cells_w = MAX_CELLS_W;
    }
    if (cells_h > MAX_CELLS_H) {
        cells_h = MAX_CELLS_H;
    }
    unsigned char visited[MAX_CELLS_H][MAX_CELLS_W] = {{0}};
    int stack_x[CELL_COUNT];
    int stack_y[CELL_COUNT];
    int stack_count = 1;
    unsigned int random_state = 0x524f4f4bu ^ (unsigned int)level * 2654435761u;

    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_LABYRINTH_WALL;
        }
    }

    stack_x[0] = 0;
    stack_y[0] = cells_h - 1;
    visited[cells_h - 1][0] = 1;
    int entry_y = 2 + (cells_h - 1) * 2;
    m->tiles[entry_y][2] = TILE_LABYRINTH_FLOOR;
    while (stack_count > 0) {
        int cell_x = stack_x[stack_count - 1];
        int cell_y = stack_y[stack_count - 1];
        int options_x[4];
        int options_y[4];
        int option_count = 0;
        static const int offsets[4][2] = {
            {0, -1}, {1, 0}, {0, 1}, {-1, 0}
        };
        for (int direction = 0; direction < 4; direction++) {
            int next_x = cell_x + offsets[direction][0];
            int next_y = cell_y + offsets[direction][1];
            if (next_x < 0 || next_x >= cells_w || next_y < 0 ||
                next_y >= cells_h || visited[next_y][next_x]) {
                continue;
            }
            options_x[option_count] = next_x;
            options_y[option_count] = next_y;
            option_count++;
        }
        if (option_count == 0) {
            stack_count--;
            continue;
        }
        int choice = (int)(labyrinth_random(&random_state) %
            (unsigned int)option_count);
        int next_x = options_x[choice];
        int next_y = options_y[choice];
        int map_x = 2 + cell_x * 2;
        int map_y = 2 + cell_y * 2;
        int next_map_x = 2 + next_x * 2;
        int next_map_y = 2 + next_y * 2;
        m->tiles[(map_y + next_map_y) / 2][(map_x + next_map_x) / 2] =
            TILE_LABYRINTH_FLOOR;
        m->tiles[next_map_y][next_map_x] = TILE_LABYRINTH_FLOOR;
        visited[next_y][next_x] = 1;
        stack_x[stack_count] = next_x;
        stack_y[stack_count] = next_y;
        stack_count++;
    }

    int dead_end_x[CELL_COUNT];
    int dead_end_y[CELL_COUNT];
    int dead_end_count = 0;
    for (int cell_y = 0; cell_y < cells_h; cell_y++) {
        for (int cell_x = 0; cell_x < cells_w; cell_x++) {
            int map_x = 2 + cell_x * 2;
            int map_y = 2 + cell_y * 2;
            int exits = 0;
            exits += m->tiles[map_y - 1][map_x] == TILE_LABYRINTH_FLOOR;
            exits += m->tiles[map_y][map_x + 1] == TILE_LABYRINTH_FLOOR;
            exits += m->tiles[map_y + 1][map_x] == TILE_LABYRINTH_FLOOR;
            exits += m->tiles[map_y][map_x - 1] == TILE_LABYRINTH_FLOOR;
            if (exits == 1 && !(cell_x == 0 && cell_y == cells_h - 1)) {
                dead_end_x[dead_end_count] = map_x;
                dead_end_y[dead_end_count] = map_y;
                dead_end_count++;
            }
        }
    }

    m->tiles[dead_end_y[0]][dead_end_x[0]] =
        switches & (1 << (level - 1)) ? TILE_LABYRINTH_SWITCH_ON :
            TILE_LABYRINTH_SWITCH_OFF;
    if (level < LABYRINTH_DEPTH) {
        int main_stair = dead_end_count / 2;
        int false_stair = dead_end_count - 1;
        m->stairs_down_x = dead_end_x[main_stair];
        m->stairs_down_y = dead_end_y[main_stair];
        m->tiles[m->stairs_down_y][m->stairs_down_x] = TILE_LABYRINTH_STAIRS;
        m->tiles[dead_end_y[false_stair]][dead_end_x[false_stair]] =
            TILE_LABYRINTH_STAIRS;
    } else {
        for (int x = 33; x <= 40; x++) {
            m->tiles[entry_y][x] = TILE_LABYRINTH_FLOOR;
        }
        for (int y = entry_y - 2; y <= entry_y + 2; y++) {
            for (int x = 37; x <= 41; x++) {
                m->tiles[y][x] = TILE_LABYRINTH_FLOOR;
            }
        }
        if (switches != (1 << LABYRINTH_SWITCH_COUNT) - 1) {
            m->tiles[entry_y][35] = TILE_LABYRINTH_GATE;
        }
        m->tiles[entry_y][40] = TILE_LABYRINTH_RELIC;
        m->stairs_down_x = 40;
        m->stairs_down_y = entry_y;
    }

    if (level > 1) {
        m->tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X] =
            TILE_LABYRINTH_EXIT;
        m->tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X + 1] =
            TILE_LABYRINTH_FLOOR;
        m->tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X + 2] =
            TILE_TRAP_REVEALED;
        m->tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X + 3] =
            TILE_LABYRINTH_FLOOR;
    }

    m->tiles[entry_y][1] = TILE_LABYRINTH_EXIT;
    m->stairs_up_x = 1;
    m->stairs_up_y = entry_y;
    *spawn_x = 2;
    *spawn_y = entry_y;
}

static void map_generate_tavern_room(Map *m, int *spawn_x, int *spawn_y, int inn) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }

    for (int y = TAVERN_Y; y < TAVERN_Y + TAVERN_H; y++) {
        for (int x = TAVERN_X; x < TAVERN_X + TAVERN_W; x++) {
            int boundary = x == TAVERN_X || x == TAVERN_X + TAVERN_W - 1 ||
                y == TAVERN_Y || y == TAVERN_Y + TAVERN_H - 1;
            m->tiles[y][x] = boundary ? TILE_TAVERN_WALL : TILE_TAVERN_FLOOR;
        }
    }

    // Bar and dining furniture create navigable pockets without blocking the
    // route between the quest givers and the exit.
    for (int x = 25; x <= 32; x++) {
        m->tiles[7][x] = TILE_TAVERN_TABLE;
    }
    for (int y = 11; y <= 13; y += 2) {
        for (int x = 9; x <= 11; x++) {
            m->tiles[y][x] = TILE_TAVERN_TABLE;
        }
        for (int x = 17; x <= 19; x++) {
            m->tiles[y][x] = TILE_TAVERN_TABLE;
        }
        for (int x = 27; x <= 29; x++) {
            m->tiles[y][x] = TILE_TAVERN_TABLE;
        }
    }

    if (inn) {
        m->tiles[18][10] = TILE_NPC_ROOK;
        m->tiles[7][28] = TILE_NPC_INNKEEPER;
    } else {
        m->tiles[7][28] = TILE_NPC_ALDER;
    }
    m->tiles[22][20] = TILE_TAVERN_EXIT;
    *spawn_x = 20;
    *spawn_y = 21;
    m->stairs_up_x = *spawn_x;
    m->stairs_up_y = *spawn_y;
    m->stairs_down_x = 20;
    m->stairs_down_y = 22;
}

void map_generate_tavern(Map *m, int *spawn_x, int *spawn_y) {
    map_generate_tavern_room(m, spawn_x, spawn_y, 0);
    map_place_tavern_brenna(m);
    map_place_tavern_liora(m);
}

void map_place_tavern_liora(Map *m) {
    m->tiles[LIORA_Y][LIORA_X] = TILE_NPC_LIORA;
}

void map_place_tavern_brenna(Map *m) {
    m->tiles[BRENNA_Y][BRENNA_X] = TILE_NPC_BRENNA;
}

void map_generate_inn(Map *m, int *spawn_x, int *spawn_y) {
    map_generate_tavern_room(m, spawn_x, spawn_y, 1);
    map_place_inn_elowen(m);
}

void map_place_inn_elowen(Map *m) {
    m->tiles[ELOWEN_INN_Y][ELOWEN_INN_X] = TILE_NPC_ELOWEN;
}

void map_generate_guild(Map *m, int *sx, int *sy) {
    map_generate_tavern_room(m, sx, sy, 1);
    m->tiles[GUILD_ORIN_Y][GUILD_ORIN_X] = TILE_NPC_ORIN;
    m->tiles[GUILD_ZARA_Y][GUILD_ZARA_X] = TILE_NPC_GUILD_SEEKER;
    m->tiles[GUILD_DAIN_Y][GUILD_DAIN_X] = TILE_NPC_DAIN;
}

void map_generate_workshop(Map *m, int *sx, int *sy) {
    map_generate_tavern_room(m, sx, sy, 0);
    for (int y = TAVERN_Y + 1; y < TAVERN_Y + TAVERN_H - 1; y++) {
        for (int x = TAVERN_X + 1; x < TAVERN_X + TAVERN_W - 1; x++) {
            m->tiles[y][x] = TILE_TAVERN_FLOOR;
        }
    }
    for (int x = 25; x <= 32; x++) {
        m->tiles[7][x] = TILE_TAVERN_TABLE;
    }
    for (int x = 9; x <= 11; x++) {
        m->tiles[11][x] = TILE_TAVERN_TABLE;
    }
    m->tiles[WORKSHOP_SMITH_Y][WORKSHOP_SMITH_X] = TILE_NPC_SHARPENER;
}

void map_generate_town_hall(Map *m, int *sx, int *sy) {
    map_generate_workshop(m, sx, sy);
    m->tiles[WORKSHOP_SMITH_Y][WORKSHOP_SMITH_X] = TILE_TAVERN_FLOOR;
    for (int x = 25; x <= 32; x++) {
        m->tiles[7][x] = TILE_TAVERN_FLOOR;
    }
    for (int x = 18; x <= 22; x++) {
        m->tiles[7][x] = TILE_TAVERN_TABLE;
    }
    for (int y = 13; y <= 17; y += 2) {
        for (int x = 27; x <= 30; x++) {
            m->tiles[y][x] = TILE_TAVERN_TABLE;
        }
    }
    m->tiles[HALL_STEWARD_Y][HALL_STEWARD_X] = TILE_NPC_STEWARD;
    m->tiles[HALL_MARA_Y][HALL_MARA_X] = TILE_NPC_MARA;
}

void map_generate_island(Map *m, int *spawn_x, int *spawn_y) {
    static const int shore_left[21] = {
        16, 12, 9, 6, 4, 3, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 3, 4, 6, 8, 11, 15
    };
    static const int shore_right[21] = {
        23, 27, 30, 33, 35, 36, 37, 37, 37, 37, 37,
        37, 37, 37, 37, 36, 35, 33, 31, 28, 24
    };
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    m->room_count = 0;

    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_ISLAND_WATER;
        }
    }

    for (int y = 2; y <= 22; y++) {
        int index = y - 2;
        for (int x = shore_left[index]; x <= shore_right[index]; x++) {
            m->tiles[y][x] = TILE_ISLAND_SAND;
        }
    }

    for (int y = 4; y <= 19; y++) {
        int shore_index = y - 2;
        int left = shore_left[shore_index] + 2;
        int right = shore_right[shore_index] - 2;
        for (int x = left; x <= right; x++) {
            m->tiles[y][x] = TILE_ISLAND_GRASS;
        }
    }

    for (int x = 6; x <= 15; x++) {
        m->tiles[6][x] = TILE_ISLAND_JUNGLE;
    }
    for (int x = 23; x <= 33; x++) {
        m->tiles[6][x] = TILE_ISLAND_JUNGLE;
    }
    for (int y = 7; y <= 18; y++) {
        m->tiles[y][4] = TILE_ISLAND_JUNGLE;
        m->tiles[y][5] = TILE_ISLAND_JUNGLE;
        m->tiles[y][34] = TILE_ISLAND_JUNGLE;
        m->tiles[y][35] = TILE_ISLAND_JUNGLE;
    }
    fill_rect(m, 6, 7, 3, 4, TILE_ISLAND_JUNGLE);
    fill_rect(m, 6, 17, 4, 2, TILE_ISLAND_JUNGLE);
    fill_rect(m, 31, 7, 3, 4, TILE_ISLAND_JUNGLE);
    fill_rect(m, 31, 17, 3, 2, TILE_ISLAND_JUNGLE);

    for (int y = 7; y <= 22; y++) {
        m->tiles[y][19] = TILE_ISLAND_PATH;
        m->tiles[y][20] = TILE_ISLAND_PATH;
    }
    fill_rect(m, 15, 12, 10, 5, TILE_ISLAND_PATH);
    for (int x = ISLAND_MARKER_X; x <= ISLAND_STATUE_X; x++) {
        m->tiles[10][x] = TILE_ISLAND_PATH;
    }
    for (int x = ISLAND_CAMP_X; x <= ISLAND_LAGOON_X; x++) {
        m->tiles[15][x] = TILE_ISLAND_PATH;
    }
    m->tiles[16][ISLAND_CAMP_X] = TILE_ISLAND_PATH;

    fill_rect(m, 16, 20, 9, 4, TILE_ISLAND_DOCK);
    m->tiles[ISLAND_GATE_Y][ISLAND_GATE_X] = TILE_ISLAND_TEMPLE_GATE;
    m->tiles[ISLAND_GATE_Y][ISLAND_GATE_X + 1] = TILE_ISLAND_TEMPLE_GATE;
    m->tiles[ISLAND_CAMP_Y][ISLAND_CAMP_X] = TILE_ISLAND_CAMP;
    m->tiles[ISLAND_MARKER_Y][ISLAND_MARKER_X] = TILE_ISLAND_MARKER;
    m->tiles[ISLAND_STATUE_Y][ISLAND_STATUE_X] = TILE_ISLAND_STATUE;
    m->tiles[ISLAND_LAGOON_Y][ISLAND_LAGOON_X] = TILE_ISLAND_LAGOON;
    m->tiles[ISLAND_SHIP_Y][ISLAND_SHIP_X] = TILE_ISLAND_SHIP;
    m->tiles[ISLAND_CAPTAIN_Y][ISLAND_CAPTAIN_X] = TILE_NPC_ISLAND_CAPTAIN;
    m->tiles[ISLAND_NAHLA_Y][ISLAND_NAHLA_X] = TILE_NPC_ISLAND_NAHLA;

    *spawn_x = ISLAND_SPAWN_X;
    *spawn_y = ISLAND_SPAWN_Y;
    m->stairs_up_x = *spawn_x;
    m->stairs_up_y = *spawn_y;
    m->stairs_down_x = ISLAND_GATE_X;
    m->stairs_down_y = ISLAND_GATE_Y;
}

void map_generate_temple(Map *m, int level, int *spawn_x, int *spawn_y) {
    map_clear_exploration(m);
    m->burial_trap_count = 0;
    memset(m->burial_traps, 0, sizeof(m->burial_traps));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_TEMPLE_WALL;
        }
    }

    if (level == 2) {
        m->room_count = 6;
        fill_rect(m, 24, 28, 17, 7, TILE_TEMPLE_FLOOR);
        fill_rect(m, 5, 21, 20, 9, TILE_TEMPLE_FLOOR);
        fill_rect(m, 39, 21, 20, 9, TILE_TEMPLE_FLOOR);
        fill_rect(m, 22, 12, 21, 10, TILE_TEMPLE_FLOOR);
        fill_rect(m, 5, 3, 20, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 39, 3, 20, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 30, 3, 5, 7, TILE_TEMPLE_FLOOR);
        fill_rect(m, 30, 8, 5, 21, TILE_TEMPLE_FLOOR);
        fill_rect(m, 23, 25, 18, 2, TILE_TEMPLE_FLOOR);
        fill_rect(m, 23, 8, 18, 2, TILE_TEMPLE_FLOOR);
        m->rooms[0] = (Room){24, 28, 17, 7};
        m->rooms[1] = (Room){5, 21, 20, 9};
        m->rooms[2] = (Room){39, 21, 20, 9};
        m->rooms[3] = (Room){22, 12, 21, 10};
        m->rooms[4] = (Room){5, 3, 20, 8};
        m->rooms[5] = (Room){39, 3, 20, 8};
        m->tiles[25][23] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[25][40] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[16][32] = TILE_TEMPLE_ALTAR;
        m->tiles[23][20] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[23][44] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[18][27] = TILE_TEMPLE_SOLAR_TRAP;
        m->tiles[18][37] = TILE_TEMPLE_SOLAR_TRAP;
    } else if (level == 3 || level == 4) {
        m->room_count = 6;
        fill_rect(m, 27, 29, 11, 6, TILE_TEMPLE_FLOOR);
        fill_rect(m, 4, 22, 22, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 38, 22, 22, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 17, 13, 30, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 5, 3, 20, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 39, 3, 20, 8, TILE_TEMPLE_FLOOR);
        fill_rect(m, 30, 3, 5, 7, TILE_TEMPLE_FLOOR);
        fill_rect(m, 30, 8, 5, 22, TILE_TEMPLE_FLOOR);
        fill_rect(m, 24, 25, 17, 2, TILE_TEMPLE_FLOOR);
        fill_rect(m, 23, 8, 18, 2, TILE_TEMPLE_FLOOR);
        m->rooms[0] = (Room){27, 29, 11, 6};
        m->rooms[1] = (Room){4, 22, 22, 8};
        m->rooms[2] = (Room){38, 22, 22, 8};
        m->rooms[3] = (Room){17, 13, 30, 8};
        m->rooms[4] = (Room){5, 3, 20, 8};
        m->rooms[5] = (Room){39, 3, 20, 8};
        m->tiles[25][24] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[25][40] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[12][32] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[17][32] = TILE_TEMPLE_ALTAR;
        m->tiles[24][20] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[24][44] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[16][22] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[16][42] = TILE_TEMPLE_DORMANT_SENTINEL;
        fill_rect(m, 18, 15, 5, 4, TILE_TEMPLE_WATER);
        fill_rect(m, 42, 15, 4, 4, TILE_TEMPLE_WATER);
        if (level == 4) {
            fill_rect(m, 14, 4, 5, 4, TILE_TEMPLE_WATER);
            fill_rect(m, 45, 4, 4, 4, TILE_TEMPLE_WATER);
            m->tiles[24][14] = TILE_TEMPLE_SOLAR_TRAP;
            m->tiles[24][51] = TILE_TEMPLE_SOLAR_TRAP;
            m->tiles[19][26] = TILE_TEMPLE_SOLAR_TRAP;
            m->tiles[19][38] = TILE_TEMPLE_SOLAR_TRAP;
        }
    } else {
        m->room_count = 5;
        fill_rect(m, 26, 28, 13, 7, TILE_TEMPLE_FLOOR);
        fill_rect(m, 4, 22, 21, 10, TILE_TEMPLE_FLOOR);
        fill_rect(m, 39, 22, 21, 10, TILE_TEMPLE_FLOOR);
        fill_rect(m, 20, 13, 25, 10, TILE_TEMPLE_FLOOR);
        fill_rect(m, 22, 2, 21, 10, TILE_TEMPLE_FLOOR);
        fill_rect(m, 30, 10, 5, 19, TILE_TEMPLE_FLOOR);
        fill_rect(m, 24, 27, 3, 1, TILE_TEMPLE_FLOOR);
        fill_rect(m, 38, 27, 3, 1, TILE_TEMPLE_FLOOR);
        m->rooms[0] = (Room){26, 28, 13, 7};
        m->rooms[1] = (Room){4, 22, 21, 10};
        m->rooms[2] = (Room){39, 22, 21, 10};
        m->rooms[3] = (Room){20, 13, 25, 10};
        m->rooms[4] = (Room){22, 2, 21, 10};
        m->tiles[30][32] = TILE_TEMPLE_ALTAR;
        m->tiles[27][25] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[27][39] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[12][32] = TILE_TEMPLE_MOON_DOOR_CLOSED;
        m->tiles[27][22] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[27][42] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[16][23] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[16][41] = TILE_TEMPLE_DORMANT_SENTINEL;
        m->tiles[26][29] = TILE_TEMPLE_SOLAR_TRAP;
        m->tiles[26][35] = TILE_TEMPLE_SOLAR_TRAP;
        m->tiles[18][28] = TILE_TEMPLE_SOLAR_TRAP;
        m->tiles[18][36] = TILE_TEMPLE_SOLAR_TRAP;
        fill_rect(m, 46, 24, 10, 5, TILE_TEMPLE_WATER);
        m->tiles[25][50] = TILE_TEMPLE_FLOOR;
        m->tiles[26][52] = TILE_TEMPLE_ALTAR;
        m->tiles[17][32] = TILE_TEMPLE_ALTAR;
    }

    *spawn_x = TEMPLE_ENTRANCE_X;
    *spawn_y = TEMPLE_ENTRANCE_Y;
    if (level == 1) {
        m->tiles[35][TEMPLE_ENTRANCE_X] = TILE_TEMPLE_ENTRANCE;
    } else {
        m->tiles[*spawn_y][*spawn_x] = TILE_STAIRS_DOWN;
    }
    if (level < TEMPLE_DEPTH) {
        m->stairs_down_x = 32;
        m->stairs_down_y = 4;
        m->tiles[m->stairs_down_y][m->stairs_down_x] = TILE_STAIRS_UP;
    } else {
        for (int x = 22; x <= 42; x++) {
            m->tiles[6][x] = TILE_TEMPLE_WALL;
        }
        m->tiles[6][32] = TILE_TEMPLE_VAULT_DOOR;
        m->tiles[TEMPLE_TREASURE_Y][TEMPLE_TREASURE_X] =
            TILE_TEMPLE_TREASURE;
        m->stairs_down_x = TEMPLE_TREASURE_X;
        m->stairs_down_y = TEMPLE_TREASURE_Y;
    }
    m->stairs_up_x = *spawn_x;
    m->stairs_up_y = *spawn_y;
}
