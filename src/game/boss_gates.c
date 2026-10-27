#include "boss_gates.h"
#include "approach.h"
#include "elderwood.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// Travel policy is static. Victories use the existing serialized boss records.
static const BossGateRule rules[] = {
    {LOCATION_DUNGEON, BOSS_GATE_ROAD, "Lich King"},
    {LOCATION_FOREST, BOSS_GATE_ROAD, "Necromancer"},
    {LOCATION_MOUNTAINS, BOSS_GATE_ROAD, "Goblin King"},
    {LOCATION_SWAMP, BOSS_GATE_ROAD, "Swamp Demon"},
    {LOCATION_ELDERWOOD, BOSS_GATE_ROAD, "Elderthorn"},
    {LOCATION_CATACOMBS, BOSS_GATE_ROAD, "Grave Marshal"},
    {LOCATION_COAST, BOSS_GATE_DESTINATION, "Drowned Queen"},
    {LOCATION_CASTLE_INTERIOR, BOSS_GATE_INTERNAL, "Castellan / Royal Arcanist / Lord Veyr"},
    {LOCATION_LABYRINTH, BOSS_GATE_NONE, "Minotaur"},
    {LOCATION_GLASSDEEP, BOSS_GATE_NONE, "Quarry Colossus"},
    {LOCATION_MANGROVES, BOSS_GATE_NONE, "Sszareth"},
    {LOCATION_STORMWATCH, BOSS_GATE_NONE, "Tempest Matriarch"},
    {LOCATION_IRONFANG, BOSS_GATE_NONE, "Goblin Mine Boss"},
    {LOCATION_GIANTS_CROWN, BOSS_GATE_NONE, "The Last Jarl"},
    {LOCATION_DESERT, BOSS_GATE_NONE, "Desert Pharaoh"},
    {LOCATION_FROSTFELL, BOSS_GATE_NONE, "Frost Kraken"},
    {LOCATION_DRAGONSPINE, BOSS_GATE_NONE, "Red Dragon"},
    {LOCATION_ASHEN, BOSS_GATE_NONE, "Cinder Lord"},
    {LOCATION_TEMPLE, BOSS_GATE_NONE, "Fallen Sun Guardian"},
    {LOCATION_ISLAND, BOSS_GATE_NONE, NULL},
    {LOCATION_CROWNROAD, BOSS_GATE_NONE, NULL},
    {LOCATION_KING_ROAD_WEST, BOSS_GATE_NONE, NULL},
    {LOCATION_CROWNROAD_NORTH, BOSS_GATE_NONE, NULL},
    {LOCATION_CASTLE, BOSS_GATE_NONE, NULL}
};

const BossGateRule *boss_gate_rule(Location area) {
    for (unsigned int i = 0; i < sizeof(rules) / sizeof(rules[0]); i++) {
        if (rules[i].area == area) {
            return &rules[i];
        }
    }
    return NULL;
}

int boss_gate_open(const GameState *g, Location area) {
    const BossGateRule *rule = boss_gate_rule(area);
    if (!rule || rule->policy == BOSS_GATE_NONE || g->debug_passages_unlocked) {
        return 1;
    }
    if (area == LOCATION_ELDERWOOD) {
        return g->elderwood_defeated;
    }
    // The castle retains its floor-specific checks in castle_travel().
    if (rule->policy == BOSS_GATE_INTERNAL) {
        return 1;
    }
    if ((unsigned int)area >= sizeof(g->defeated_bosses) * CHAR_BIT) {
        return 0;
    }
    return (g->defeated_bosses & (1u << (unsigned int)area)) != 0;
}

static void passage_message(GameState *g, Location area) {
    char message[MAX_MESSAGE_LEN];
    const BossGateRule *rule = boss_gate_rule(area);
    if (boss_gate_open(g, area)) {
        snprintf(message, sizeof(message), "Passage secured through %s. The road is open in both directions.", approach_area_name(area));
    } else {
        snprintf(message, sizeof(message), "Passage blocked. Defeat %s in %s to secure this road.", rule->boss, approach_area_name(area));
    }
    push_message(g, message);
}

int boss_gate_crossing(GameState *g, Location area) {
    if (boss_gate_open(g, area)) {
        return 1;
    }
    passage_message(g, area);
    return 0;
}

int boss_gate_checkpoints(const GameState *g, RoadCheckpoint *points) {
    const TownApproach *route = approach_route(g);
    if (!route) {
        return 0;
    }
    if (northern_approach(g)) {
        points[0] = (RoadCheckpoint){LOCATION_ELDERWOOD, NORTH_ROAD_X, NORTH_FOREST_BARRIER_Y, 1};
        points[1] = (RoadCheckpoint){LOCATION_CATACOMBS, NORTH_ROAD_X, NORTH_CAT_Y - 2, 1};
        return 2;
    }
    const BossGateRule *rule = boss_gate_rule(route->area);
    if (!rule || rule->policy != BOSS_GATE_ROAD) {
        return 0;
    }
    int vertical = route->area == LOCATION_SWAMP || route->area == LOCATION_MOUNTAINS;
    int far = vertical ? g->map.stairs_up_y == 0 : g->map.stairs_up_x == 0;
    points[0] = (RoadCheckpoint){route->area, vertical ? APPROACH_W / 2 : far ? APPROACH_W - 4 : 3,
        vertical ? far ? APPROACH_H - 4 : 3 : APPROACH_H / 2, vertical};
    return 1;
}

static void landing(const GameState *g, int *x, int *y) {
    *x = g->map.stairs_up_x;
    *y = g->map.stairs_up_y;
    if (northern_approach(g) || *y == 0 || *y == APPROACH_H - 1) {
        *y += *y == 0 ? 1 : -1;
    } else {
        *x += *x == 0 ? 1 : -1;
    }
}

static void replace_tile(GameState *g, int x, int y, TileType tile) {
    g->map.tiles[y][x] = tile;
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            if (tile == TILE_ROAD_BANK || tile == TILE_ROAD_GATE_CLOSED) {
                landing(g, &item->x, &item->y);
                item->underlying_tile = TILE_TOWN_PATH;
                continue;
            }
            item->underlying_tile = tile;
            g->map.tiles[y][x] = TILE_ITEM;
        }
    }
}

void boss_gate_refresh(GameState *g) {
    RoadCheckpoint points[2];
    int count = boss_gate_checkpoints(g, points);
    if (!count) {
        return;
    }
    for (int n = 0; n < count; n++) {
        RoadCheckpoint point = points[n];
        if (!northern_approach(g)) {
            // An irregular woodland/rock edge frames a single visible crossing.
            // The bank reaches the map edge, so grass cannot bypass the beam.
            int far = point.vertical ? point.y > APPROACH_H / 2 : point.x > APPROACH_W / 2;
            int across = point.vertical ? APPROACH_W : APPROACH_H;
            int length = point.vertical ? APPROACH_H : APPROACH_W;
            for (int step = 0; step < length; step++) {
                int x = point.vertical ? point.x : step;
                int y = point.vertical ? step : point.y;
                if (x == g->map.stairs_up_x && y == g->map.stairs_up_y) {
                    replace_tile(g, x, y, TILE_APPROACH_TOWN);
                } else {
                    replace_tile(g, x, y, TILE_TOWN_PATH);
                }
            }
            for (int side = 1; side < across - 1; side++) {
                int depth = 4 + ((side / 3 + side / 7) % 4);
                for (int step = 0; step <= depth; step++) {
                    int x = point.vertical ? side : far ? APPROACH_W - 1 - step : step;
                    int y = !point.vertical ? side : far ? APPROACH_H - 1 - step : step;
                    int road = point.vertical ? x == point.x : y == point.y;
                    replace_tile(g, x, y, road ? TILE_TOWN_PATH : TILE_ROAD_BANK);
                }
            }
            replace_tile(g, point.vertical ? point.x : far ? APPROACH_W - 1 : 0,
                point.vertical ? far ? APPROACH_H - 1 : 0 : point.y, TILE_APPROACH_DUNGEON_TOWN);
        }
        replace_tile(g, point.x, point.y, boss_gate_open(g, point.area) ? TILE_ROAD_GATE_OPEN : TILE_ROAD_GATE_CLOSED);
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
}

static int nearby(const GameState *g, RoadCheckpoint *point) {
    RoadCheckpoint points[2];
    int count = boss_gate_checkpoints(g, points);
    for (int n = 0; n < count; n++) {
        if (abs(g->player.x - points[n].x) + abs(g->player.y - points[n].y) <= 2) {
            *point = points[n];
            return 1;
        }
    }
    return 0;
}

int boss_gate_hint(const GameState *g) {
    RoadCheckpoint point;
    return nearby(g, &point);
}

int boss_gate_interact(GameState *g) {
    RoadCheckpoint point;
    if (!nearby(g, &point)) {
        return 0;
    }
    passage_message(g, point.area);
    return 1;
}

int boss_gate_bump(GameState *g, int x, int y) {
    RoadCheckpoint points[2];
    int count = boss_gate_checkpoints(g, points);
    for (int n = 0; n < count; n++) {
        if (x == points[n].x && y == points[n].y && !boss_gate_open(g, points[n].area)) {
            passage_message(g, points[n].area);
            return 1;
        }
    }
    return 0;
}

static int return_reachable(const GameState *g) {
    int sx;
    int sy;
    landing(g, &sx, &sy);
    unsigned char seen[APPROACH_H][APPROACH_W] = {{0}};
    int queue[APPROACH_W * APPROACH_H];
    const int steps[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int head = 0;
    int tail = 1;
    queue[0] = sy * APPROACH_W + sx;
    seen[sy][sx] = 1;
    while (head < tail) {
        int cell = queue[head++];
        int x = cell % APPROACH_W;
        int y = cell / APPROACH_W;
        if (x == g->player.x && y == g->player.y) {
            return 1;
        }
        for (int side = 0; side < 4; side++) {
            int nx = x + steps[side][0];
            int ny = y + steps[side][1];
            if (nx >= 0 && ny >= 0 && nx < APPROACH_W && ny < APPROACH_H && !seen[ny][nx] && map_is_walkable(&g->map, nx, ny)) {
                seen[ny][nx] = 1;
                queue[tail++] = ny * APPROACH_W + nx;
            }
        }
    }
    return 0;
}

void boss_gate_migrate(GameState *g) {
    const TownApproach *route = approach_route(g);
    const BossGateRule *rule = route ? boss_gate_rule(route->area) : NULL;
    if (!route || !rule || (!northern_approach(g) && rule->policy != BOSS_GATE_ROAD)) {
        return;
    }
    // Only safe approach terrain changes. Combat caches and quest sites stay put.
    Map old = g->map;
    int loot_count = g->floor_item_count;
    g->floor_item_count = 0;
    if (northern_approach(g)) {
        northern_refresh(g);
    } else {
        approach_refresh_dungeon(g);
        approach_refresh_forest(g);
        approach_refresh_mountains(g);
        approach_refresh_swamp(g);
        boss_gate_refresh(g);
    }
    g->floor_item_count = loot_count;
    memcpy(g->map.explored, old.explored, sizeof(old.explored));
    // Preserve loot even when its old tile becomes an impassable bank.
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && !map_is_walkable(&g->map, item->x, item->y)) {
            landing(g, &item->x, &item->y);
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            item->underlying_tile = g->map.tiles[item->y][item->x];
        }
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active) {
            g->map.tiles[item->y][item->x] = TILE_ITEM;
        }
    }
    if (!return_reachable(g)) {
        landing(g, &g->player.x, &g->player.y);
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
}
