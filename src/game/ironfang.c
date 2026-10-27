#include "boss_gates.h"
#include "containers.h"
#include "ironfang.h"
#include "war_machine.h"
#include "approach.h"
#include "town_road.h"
#include <stdlib.h>
#include <string.h>

static int inside(int x, int y) {
    return x >= 0 && x < IRON_W && y >= 0 && y < IRON_H;
}

static IronfangStage *current(GameState *g) {
    return g->location == LOCATION_IRONFANG && g->level >= 1 && g->level <= IRON_DEPTH ? &g->ironfang[g->level - 1] : NULL;
}

const char *ironfang_stage_name(int level) {
    static const char *names[] = {"Abandoned Workings", "Goblin Warrens", "Flooded Galleries", "Deep Ironworks", "The Rootshaft"};
    return level >= 1 && level <= IRON_DEPTH ? names[level - 1] : "Ironfang Mines";
}

static void chamber(Map *map, int cx, int cy, int rx, int ry, int natural) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (x > 1 && x < IRON_W - 2 && y > 1 && y < IRON_H - 2 &&
                (!natural || (x - cx) * (x - cx) * ry * ry + (y - cy) * (y - cy) * rx * rx <= rx * rx * ry * ry)) {
                map->tiles[y][x] = TILE_IRON_FLOOR;
            }
        }
    }
}

static void tunnel(Map *map, int x, int y, int tx, int ty, int wide) {
    while (x != tx || y != ty) {
        chamber(map, x, y, wide, wide, 0);
        if (x != tx) {
            x += (tx > x) - (tx < x);
        } else {
            y += (ty > y) - (ty < y);
        }
    }
    chamber(map, tx, ty, wide, wide, 0);
}

void ironfang_generate(Map *map, int level) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = TILE_IRON_WALL;
        }
    }
    // Different chamber graphs, rather than one corridor with a new palette.
    static const int nodes[IRON_DEPTH][9][2] = {
        {{9,39},{12,25},{30,25},{30,9},{49,9},{50,38},{12,8},{29,40},{51,25}},
        {{8,39},{11,25},{30,25},{30,9},{49,10},{48,38},{8,11},{22,40},{51,25}},
        {{9,39},{12,25},{30,25},{30,9},{49,9},{50,38},{10,14},{26,40},{51,25}},
        {{9,39},{12,25},{30,25},{30,9},{49,10},{50,35},{11,10},{29,40},{51,25}},
        {{9,39},{12,25},{30,25},{30,9},{49,10},{46,38},{15,8},{25,36},{51,25}}
    };
    int points[9][2];
    memcpy(points, nodes[level - 1], sizeof(points));
    for (int n = 0; n < 9; n++) {
        if (n == 0 || n == 4 || n == 5 || n == 6 || n == 7) {
            points[n][0] += rand() % 3 - 1;
            points[n][1] += rand() % 3 - 1;
        }
        int rx = level == 2 ? 4 : level == 4 ? 5 : 5 + rand() % 3;
        int ry = level == 2 ? 3 : 4 + rand() % 2;
        chamber(map, points[n][0], points[n][1], rx, ry, level == 3 || level == 5);
    }
    static const int edges[IRON_DEPTH][12][2] = {
        {{0,1},{1,2},{2,3},{3,4},{4,8},{8,5},{5,7},{7,0},{1,6},{6,3},{2,7},{4,6}},
        {{0,7},{7,5},{5,8},{8,4},{4,3},{3,6},{6,1},{1,0},{1,2},{2,8},{2,3},{6,4}},
        {{0,1},{1,6},{6,3},{3,4},{4,8},{8,5},{5,7},{7,0},{1,2},{2,7},{3,2},{5,2}},
        {{0,1},{1,2},{2,8},{8,5},{5,7},{7,0},{1,6},{6,3},{3,4},{4,8},{2,7},{2,4}},
        {{0,7},{7,5},{5,8},{8,4},{4,3},{3,6},{6,1},{1,0},{2,1},{2,8},{2,3},{2,7}}
    };
    for (int e = 0; e < 12; e++) {
        int a = edges[level - 1][e][0];
        int b = edges[level - 1][e][1];
        if (level == 2 || level == 4) {
            tunnel(map, points[a][0], points[a][1], points[b][0], points[b][1], 1);
        } else {
            int mx = (points[a][0] + points[b][0]) / 2;
            int my = (points[a][1] + points[b][1]) / 2;
            tunnel(map, points[a][0], points[a][1], mx, my, 1);
            tunnel(map, mx, my, points[b][0], points[b][1], 1);
        }
    }
    if (level == 2) {
        // Occupied warrens: small linked rooms, with cover around their doors.
        for (int y = 14; y <= 38; y += 12) {
            chamber(map, 21, y, 4, 3, 0);
            tunnel(map, 21, y, 30, 25, 0);
            map->tiles[y - 1][20] = TILE_IRON_BUNK;
            map->tiles[y + 1][23] = TILE_IRON_BUNK;
        }
    }
    if (level == 3) {
        // Underground pools consume chamber interiors, leaving dry perimeter loops.
        for (int y = 14; y <= 20; y++) {
            for (int x = 36; x <= 45; x++) {
                if ((x - 40) * (x - 40) * 9 + (y - 17) * (y - 17) * 25 <= 225) {
                    map->tiles[y][x] = TILE_IRON_WATER;
                }
            }
        }
        tunnel(map, 30, 9, 50, 9, 1);
        tunnel(map, 50, 9, 50, 39, 1);
    }
    if (level == 4) {
        for (int x = 18; x <= 46; x += 14) {
            chamber(map, x, 36, 5, 4, 0);
            tunnel(map, x, 36, 30, 40, 1);
            map->tiles[36][x] = TILE_IRON_WALL;
            map->tiles[34][x + 2] = TILE_IRON_FORGE;
        }
    }
    // A recognizable T junction serves every expedition. Its north and east
    // branches require different switch settings; the east branch needs repair.
    tunnel(map, 10, 25, 52, 25, 1);
    tunnel(map, 30, 25, 30, 9, 1);
    for (int x = 10; x <= 60; x++) {
        map->tiles[25][x] = TILE_IRON_RAIL;
    }
    for (int y = 9; y < 25; y++) {
        map->tiles[y][30] = TILE_IRON_RAIL;
    }
    map->tiles[25][40] = TILE_IRON_BROKEN;
    map->tiles[24][29] = TILE_IRON_SWITCH;
    // The salvage alcove can only be reached by riding over the flooded trestle.
    for (int y = 21; y <= 29; y++) {
        for (int x = 55; x <= 62; x++) {
            map->tiles[y][x] = TILE_IRON_WATER;
        }
    }
    for (int x = 55; x <= 60; x++) {
        map->tiles[25][x] = TILE_IRON_TRESTLE;
    }
    chamber(map, 60, 24, 1, 1, 0);
    map->tiles[25][59] = TILE_IRON_TRESTLE;
    map->tiles[25][60] = TILE_IRON_RAIL;
    map->tiles[23][60] = TILE_IRON_CACHE;
    // Lamps reveal the next working, without consuming fuel or hiding actors.
    const int lamps[][2] = {{11,24},{31,24},{31,10},{50,24},{12,38}};
    for (int n = 0; n < 5; n++) {
        int x = lamps[n][0];
        int y = lamps[n][1];
        if (map->tiles[y + 1][x] == TILE_IRON_WALL) {
            map->tiles[y + 1][x] = TILE_IRON_FLOOR;
        }
        map->tiles[y][x] = n == 0 ? TILE_IRON_LAMP_LIT : TILE_IRON_LAMP;
    }
    for (int n = 5; n <= 7; n++) {
        map->tiles[points[n][1]][points[n][0]] = TILE_IRON_CACHE;
    }
    // An optional excavated pocket is sealed by one cracked rock face.
    for (int y = 4; y <= 12; y++) {
        for (int x = 3; x <= 8; x++) {
            map->tiles[y][x] = TILE_IRON_WALL;
        }
    }
    chamber(map, 6, 8, 1, 2, 0);
    tunnel(map, 6, 8, 9, 8, 0);
    map->tiles[8][6] = TILE_IRON_CACHE;
    for (int y = 5; y <= 11; y++) {
        map->tiles[y][8] = TILE_IRON_WALL;
    }
    map->tiles[8][8] = TILE_IRON_CRACK;
    tunnel(map, 9, 8, 12, 25, 0);
    // Boss gallery has a fixed, fully visible haulage lane and no spawn overlap.
    if (level == IRON_DEPTH) {
        chamber(map, 49, 11, 9, 7, 0);
        tunnel(map, 30, 9, 49, 11, 1);
        for (int x = 40; x <= 58; x++) {
            map->tiles[11][x] = TILE_IRON_RAIL;
        }
        map->tiles[15][41] = TILE_IRON_WINCH;
    }
    tunnel(map, 9, 39, points[0][0], points[0][1], 1);
    tunnel(map, 54, 7, points[4][0], points[4][1], 1);
    for (int y = 39; y < IRON_H; y++) {
        map->tiles[y][9] = TILE_IRON_FLOOR;
    }
    for (int y = 0; y <= 7; y++) {
        map->tiles[y][54] = TILE_IRON_FLOOR;
    }
    // Excavation and room dressing must not erase the continuous track graph.
    for (int x = 10; x <= 60; x++) {
        map->tiles[25][x] = x >= 55 && x < 60 ? TILE_IRON_TRESTLE : TILE_IRON_RAIL;
    }
    map->tiles[25][40] = TILE_IRON_BROKEN;
    for (int y = 9; y < 25; y++) {
        map->tiles[y][30] = TILE_IRON_RAIL;
    }
    map->stairs_up_x = 9;
    map->stairs_up_y = IRON_H - 1;
    map->stairs_down_x = 54;
    map->stairs_down_y = 0;
    map->tiles[IRON_H - 1][9] = TILE_IRON_ENTRANCE;
    map->tiles[0][54] = TILE_IRON_EXIT;
    map->room_count = 1;
    map->rooms[0] = (Room){2, 2, IRON_W - 4, IRON_H - 4};
}

void ironfang_pack(IronfangStage *state, const Map *map) {
    memset(state->explored, 0, sizeof(state->explored));
    for (int y = 0; y < IRON_H; y++) {
        for (int x = 0; x < IRON_W; x++) {
            state->tiles[y][x] = map->tiles[y][x];
            if (map_is_explored(map, x, y)) {
                int bit = y * IRON_W + x;
                state->explored[bit / 8] |= 1u << (bit % 8);
            }
        }
    }
}

void ironfang_unpack(const IronfangStage *state, Map *map) {
    memset(map, 0, sizeof(*map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            map->tiles[y][x] = inside(x, y) ? state->tiles[y][x] : TILE_IRON_WALL;
            if (inside(x, y)) {
                int bit = y * IRON_W + x;
                if (state->explored[bit / 8] & (1u << (bit % 8))) {
                    map_mark_explored(map, x, y);
                }
            }
        }
    }
    map->stairs_up_x = 9;
    map->stairs_up_y = IRON_H - 1;
    map->stairs_down_x = 54;
    map->stairs_down_y = 0;
    map->room_count = 1;
    map->rooms[0] = (Room){2, 2, IRON_W - 4, IRON_H - 4};
}

void ironfang_capture(const GameState *g, IronfangStage *state) {
    ironfang_pack(state, &g->map);
    state->enemy_count = g->enemy_count;
    state->loot_count = g->floor_item_count;
    memcpy(state->enemies, g->enemies, sizeof(Enemy) * state->enemy_count);
    memcpy(state->loot, g->floor_items, sizeof(FloorItem) * state->loot_count);
    for (int n = 0; n < state->loot_count; n++) {
        const FloorItem *item = &state->loot[n];
        if (item->active && inside(item->x, item->y)) {
            state->tiles[item->y][item->x] = item->underlying_tile;
        }
    }
    state->valid = 1;
}

void ironfang_store(GameState *g) {
    IronfangStage *state = current(g);
    if (state) {
        ironfang_capture(g, state);
    }
}

static int occupied(const GameState *g, int x, int y) {
    for (int n = 0; n < g->enemy_count; n++) {
        if (g->enemies[n].active && g->enemies[n].x == x && g->enemies[n].y == y) {
            return 1;
        }
    }
    return 0;
}

static void spawn(GameState *g, EnemyType type, int x, int y) {
    static const char *names[] = {"Goblin Scout", "Goblin Archer", "Goblin Bomber", "Tunnel Spider", "Cave Troll", "Hobgoblin Guard", "Goblin Shaman"};
    const int hp[] = {12,16,18,20,52,36,26};
    const int attack[] = {5,7,9,8,14,11,10};
    const int defense[] = {1,1,1,3,5,7,2};
    int boss = type == ENEMY_GOBLIN_MINE_BOSS;
    int kind = boss ? 0 : type - ENEMY_GOBLIN_SCOUT;
    if (g->enemy_count >= AREA_ENEMY_LIMIT || occupied(g, x, y)) {
        return;
    }
    Enemy *enemy = &g->enemies[g->enemy_count++];
    *enemy = (Enemy){.active = 1, .type = type, .x = x, .y = y, .is_boss = boss, .facing_dy = 1};
    snprintf(enemy->name, sizeof(enemy->name), "%s", boss ? "Goblin Mine Boss" : names[kind]);
    enemy->hp = enemy->max_hp = boss ? 310 : hp[kind] + g->level * 4 + 8;
    enemy->attack = boss ? 23 : attack[kind] + 3 + g->level / 2;
    enemy->defense = boss ? 8 : defense[kind] + 1;
    enemy->experience = boss ? 550 : 22 + kind * 7;
}

static void populate(GameState *g) {
    g->enemy_count = 0;
    // Reserve the boss before random regular actors, so the encounter always exists.
    if (g->level == IRON_DEPTH && !g->ironfang_defeated) {
        spawn(g, ENEMY_GOBLIN_MINE_BOSS, 51, 11);
    }
    unsigned char reached[IRON_H][IRON_W] = {{0}};
    int queue[IRON_W * IRON_H];
    int front = 0;
    int count = 1;
    queue[0] = (IRON_H - 2) * IRON_W + 9;
    reached[IRON_H - 2][9] = 1;
    const int dx[] = {1,0,-1,0};
    const int dy[] = {0,1,0,-1};
    while (front < count) {
        int cell = queue[front++];
        int x = cell % IRON_W;
        int y = cell / IRON_W;
        for (int d = 0; d < 4; d++) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (inside(nx, ny) && !reached[ny][nx] && map_is_walkable(&g->map, nx, ny)) {
                reached[ny][nx] = 1;
                queue[count++] = ny * IRON_W + nx;
            }
        }
    }
    int limit = 8 + g->level;
    for (int n = 0; n < count && g->enemy_count < limit; n++) {
        int cell = queue[(n * 97 + g->level * 31) % count];
        int x = cell % IRON_W;
        int y = cell / IRON_W;
        if (y < 5 || y > 39 || g->map.tiles[y][x] != TILE_IRON_FLOOR || occupied(g, x, y) ||
            (g->player.x == x && g->player.y == y) || (g->level == IRON_DEPTH && x >= 40 && y <= 18)) {
            continue;
        }
        int kinds = g->level == 1 ? 4 : g->level == 2 ? 6 : 7;
        spawn(g, ENEMY_GOBLIN_SCOUT + rand() % kinds, x, y);
    }
}

static void load_stage(GameState *g, int refresh) {
    IronfangStage *state = current(g);
    if (!state->valid) {
        ironfang_generate(&g->map, g->level);
        state->cart_x = 10;
        state->cart_y = 25;
        state->ore_loads = g->level == IRON_DEPTH ? 3 : 0;
        g->floor_item_count = 0;
        populate(g);
    } else {
        ironfang_unpack(state, &g->map);
        g->enemy_count = state->enemy_count;
        memcpy(g->enemies, state->enemies, sizeof(Enemy) * state->enemy_count);
        g->floor_item_count = state->loot_count;
        memcpy(g->floor_items, state->loot, sizeof(FloorItem) * state->loot_count);
        for (int n = 0; n < g->floor_item_count; n++) {
            FloorItem *item = &g->floor_items[n];
            if (item->active) {
                g->map.tiles[item->y][item->x] = item->underlying_tile;
            }
        }
        if (refresh) {
            state->aboard = 0;
            state->cart_x = 10;
            state->cart_y = 25;
            state->ore_fuse = 0;
            state->ore_loads = g->level == IRON_DEPTH && !g->ironfang_defeated ? 3 : 0;
            populate(g);
        }
    }
    ironfang_store(g);
    g->level_cleared = 0;
    game_update_level_progress(g);
}

void ironfang_enter(GameState *g, int portal) {
    g->location = LOCATION_IRONFANG;
    g->level = portal ? g->portal_level : 1;
    if (!portal) {
        for (int n = IRON_DEPTH - 1; n >= 0; n--) {
            if (g->ironfang[n].valid) {
                g->level = n + 1;
                load_stage(g, 1);
            }
        }
        g->level = 1;
    }
    load_stage(g, 0);
    g->player.x = portal ? g->portal_x : 9;
    g->player.y = portal ? g->portal_y : IRON_H - 2;
    g->player.last_dx = 0;
    g->player.last_dy = -1;
    if (g->portal_location == LOCATION_IRONFANG) {
        g->portal_active = 0;
    }
    map_mark_explored(&g->map, g->player.x, g->player.y);
    push_message(g, "Ironfang Mines: A repairs lamps and track, works switches, and boards the cart. Explore the side workings.");
}

void ironfang_leave(GameState *g) {
    ironfang_store(g);
    approach_enter_region(g, 19);
    g->player.x = g->map.stairs_down_x;
    g->player.y = g->map.stairs_down_y + 1;
    g->player.last_dx = 0;
    g->player.last_dy = 1;
}

void ironfang_change_level(GameState *g, int forward) {
    if (!current(g)) {
        return;
    }
    if ((!forward && g->level == 1) || (forward && g->level == IRON_DEPTH)) {
        if (forward && !boss_gate_crossing(g, LOCATION_IRONFANG)) {
            return;
        }
        ironfang_leave(g);
        return;
    }
    ironfang_store(g);
    g->level += forward ? 1 : -1;
    load_stage(g, 0);
    g->player.x = forward ? 9 : 54;
    g->player.y = forward ? IRON_H - 2 : 1;
    map_mark_explored(&g->map, g->player.x, g->player.y);
    char text[MAX_MESSAGE_LEN];
    snprintf(text, sizeof(text), "Ironfang %d: %s", g->level, ironfang_stage_name(g->level));
    push_message(g, text);
}

int ironfang_travel(GameState *g, int x, int y) {
    if (!current(g) || !inside(x, y)) {
        return 0;
    }
    TileType tile = g->map.tiles[y][x];
    if (tile != TILE_IRON_ENTRANCE && tile != TILE_IRON_EXIT) {
        return 0;
    }
    ironfang_change_level(g, tile == TILE_IRON_EXIT);
    return 1;
}

static int rail(TileType tile) {
    return tile == TILE_IRON_RAIL || tile == TILE_IRON_TRESTLE;
}

int ironfang_move(GameState *g, int x, int y) {
    IronfangStage *state = current(g);
    if (!state || !state->aboard) {
        return 0;
    }
    if (!inside(x, y) || abs(x - state->cart_x) + abs(y - state->cart_y) != 1 || !rail(g->map.tiles[y][x]) || occupied(g, x, y)) {
        push_message(g, "The cart needs clear, repaired track. A disembarks beside solid ground.");
        return 1;
    }
    if ((state->cart_x == 30 && state->cart_y == 25 && ((y < 25 && !state->route) || (x > 30 && state->route))) ||
        (x == 30 && y == 25 && ((state->cart_y < 25 && !state->route) || (state->cart_x > 30 && state->route)))) {
        push_message(g, "The points are set for the other branch. Disembark and operate the junction switch.");
        return 1;
    }
    int dx = x - state->cart_x;
    int dy = y - state->cart_y;
    // A cart covers two clear straight cells per action, stopping at points,
    // obstructions and endpoints so every junction remains a deliberate choice.
    int nx = x + dx;
    int ny = y + dy;
    if (!(x == 30 && y == 25) && inside(nx, ny) && rail(g->map.tiles[ny][nx]) &&
        !occupied(g, nx, ny) && !(nx == 30 && ny == 25)) {
        x = nx;
        y = ny;
    }
    state->cart_x = g->player.x = x;
    state->cart_y = g->player.y = y;
    g->player.last_dx = dx;
    g->player.last_dy = dy;
    map_mark_explored(&g->map, x, y);
    return 1;
}

static TileType nearby(const GameState *g, int *tx, int *ty) {
    static const int dx[] = {0,1,0,-1,0};
    static const int dy[] = {0,0,1,0,-1};
    for (int n = 0; n < 5; n++) {
        int x = g->player.x + dx[n];
        int y = g->player.y + dy[n];
        if (!inside(x, y)) {
            continue;
        }
        TileType tile = g->map.tiles[y][x];
        if (tile == TILE_IRON_BROKEN || tile == TILE_IRON_SWITCH || tile == TILE_IRON_LAMP ||
            tile == TILE_IRON_CACHE || tile == TILE_IRON_CRACK || tile == TILE_IRON_WINCH) {
            *tx = x;
            *ty = y;
            return tile;
        }
    }
    return TILE_IRON_FLOOR;
}

const char *ironfang_hint(const GameState *g) {
    if (g->location != LOCATION_IRONFANG || g->level < 1 || g->level > IRON_DEPTH) {
        return NULL;
    }
    const IronfangStage *state = &g->ironfang[g->level - 1];
    if (state->aboard) {
        return "A: DISEMBARK | ARROWS: RIDE TRACK";
    }
    if (war_machine_hint(g)) {
        return war_machine_hint(g);
    }
    if (containers_hint(g)) {
        return containers_hint(g);
    }
    int x;
    int y;
    switch (nearby(g, &x, &y)) {
        case TILE_IRON_BROKEN: return "A: REPAIR TRACK";
        case TILE_IRON_SWITCH: return state->route ? "A: SET TRACK EAST" : "A: SET TRACK NORTH";
        case TILE_IRON_LAMP: return "A: RESTORE LAMP";
        case TILE_IRON_CRACK: return "A: EXCAVATE PASSAGE";
        case TILE_IRON_WINCH: return "A: RELEASE ORE CART";
        default: break;
    }
    return abs(g->player.x - state->cart_x) + abs(g->player.y - state->cart_y) <= 1 ? "A: BOARD MINECART" : NULL;
}

int ironfang_interact(GameState *g) {
    IronfangStage *state = current(g);
    if (!state || !ironfang_hint(g)) {
        return 0;
    }
    if (state->aboard) {
        const int dx[] = {0,1,0,-1};
        const int dy[] = {-1,0,1,0};
        for (int n = 0; n < 4; n++) {
            int x = g->player.x + dx[n];
            int y = g->player.y + dy[n];
            if (inside(x, y) && map_is_walkable(&g->map, x, y) && !occupied(g, x, y)) {
                g->player.x = x;
                g->player.y = y;
                state->aboard = 0;
                return 1;
            }
        }
        push_message(g, "Ride to a landing before disembarking. Water is not solid ground.");
        return 1;
    }
    int x;
    int y;
    if (war_machine_interact(g)) {
        return 1;
    }
    if (containers_hint(g)) {
        return containers_interact(g);
    }
    TileType tile = nearby(g, &x, &y);
    if (tile == TILE_IRON_SWITCH) {
        state->route = !state->route;
        push_message(g, state->route ? "The points now connect the west and north galleries." : "The points now connect the west and east galleries.");
    } else if (tile == TILE_IRON_BROKEN || tile == TILE_IRON_CRACK) {
        g->map.tiles[y][x] = tile == TILE_IRON_BROKEN ? TILE_IRON_RAIL : TILE_IRON_FLOOR;
        push_message(g, tile == TILE_IRON_BROKEN ? "You repair the broken haulage rail. The eastern route is usable." : "You break through into an abandoned side working.");
    } else if (tile == TILE_IRON_LAMP) {
        g->map.tiles[y][x] = TILE_IRON_LAMP_LIT;
        for (int yy = y - 8; yy <= y + 8; yy++) {
            for (int xx = x - 8; xx <= x + 8; xx++) {
                if (inside(xx, yy) && abs(xx - x) + abs(yy - y) <= 10) {
                    map_mark_explored(&g->map, xx, yy);
                }
            }
        }
        push_message(g, "The repaired lamp reveals the surrounding galleries on your map.");
    } else if (tile == TILE_IRON_CACHE) {
        return containers_interact(g);
    } else if (tile == TILE_IRON_WINCH) {
        if (state->ore_fuse) {
            push_message(g, "The ore cart is already rolling. Clear the marked rail!");
        } else if (!state->ore_loads) {
            push_message(g, "The ore hopper is empty. The boss can still be defeated with your weapons.");
        } else {
            state->ore_loads--;
            state->ore_fuse = 3;
            push_message(g, "Ore cart released! Clear the marked gallery before it strikes in two actions.");
        }
    } else {
        if (occupied(g, state->cart_x, state->cart_y)) {
            push_message(g, "An enemy blocks the cart. Clear the track before boarding.");
            return 1;
        }
        state->aboard = 1;
        g->player.x = state->cart_x;
        g->player.y = state->cart_y;
        push_message(g, "You board the minecart. Move along the rails; A disembarks at solid ground.");
    }
    return 1;
}

int ironfang_lit(const GameState *g, int x, int y) {
    if (abs(x - g->player.x) + abs(y - g->player.y) <= 6) {
        return 1;
    }
    for (int yy = y - 7; yy <= y + 7; yy++) {
        for (int xx = x - 7; xx <= x + 7; xx++) {
            if (inside(xx, yy) && g->map.tiles[yy][xx] == TILE_IRON_LAMP_LIT && abs(x - xx) + abs(y - yy) <= 9) {
                return 1;
            }
        }
    }
    return 0;
}

int ironfang_ore_marks(const GameState *g, int x, int y) {
    return g->location == LOCATION_IRONFANG && g->level == IRON_DEPTH && g->ironfang[IRON_DEPTH - 1].ore_fuse > 0 && y == 11 && x >= 40 && x <= 58;
}

int ironfang_tick_ore(GameState *g) {
    IronfangStage *state = current(g);
    if (!state || state->ore_fuse <= 0 || --state->ore_fuse > 0) {
        return 0;
    }
    for (int n = 0; n < g->enemy_count; n++) {
        Enemy *enemy = &g->enemies[n];
        if (enemy->active && enemy->y == 11 && enemy->x >= 40 && enemy->x <= 58) {
            enemy->hp -= enemy->is_boss ? enemy->max_hp / 5 : 45;
            enemy->attack_phase = 0;
            enemy->move_timer = -3;
        }
    }
    push_message(g, "The ore cart crashes through the gallery, staggering everything in its path!");
    return g->player.y == 11 && g->player.x >= 40 && g->player.x <= 58 ? 24 : 0;
}

int ironfang_attack_marks(const GameState *g, const Enemy *enemy, int x, int y) {
    if (g->location != LOCATION_IRONFANG || enemy->type != ENEMY_GOBLIN_MINE_BOSS || !enemy->active || !enemy->attack_phase || enemy->move_timer <= 0) {
        return 0;
    }
    if (enemy->boss_attack == IRON_CART_ATTACK) {
        return y == enemy->attack_target_y && x >= 40 && x <= 58;
    }
    int distance = abs(x - enemy->attack_target_x) + abs(y - enemy->attack_target_y);
    return enemy->boss_attack == IRON_ROCKFALL ? distance <= 1 : abs(x - enemy->x) <= 1 && abs(y - enemy->y) <= 1;
}

int ironfang_tick(GameState *g, Enemy *enemy, int *damage) {
    if (g->location != LOCATION_IRONFANG || enemy->type != ENEMY_GOBLIN_MINE_BOSS) {
        return 0;
    }
    *damage = 0;
    if (enemy->move_timer < 0) {
        enemy->move_timer++;
        enemy->defense = enemy->move_timer ? 2 : 8;
        return 1;
    }
    if (enemy->attack_phase) {
        if (enemy->move_timer > 1) {
            enemy->move_timer--;
            return 1;
        }
        *damage = ironfang_attack_marks(g, enemy, g->player.x, g->player.y) ? enemy->attack : 0;
        enemy->attack_phase = 0;
        enemy->move_timer = -2;
        enemy->defense = 2;
        return 1;
    }
    int distance = abs(enemy->x - g->player.x) + abs(enemy->y - g->player.y);
    if (distance > 16) {
        return 1;
    }
    if (enemy->hp <= enemy->max_hp / 2 && !enemy->boss_wave_used) {
        enemy->boss_wave_used = 1;
        int made = 0;
        for (int x = 45; x <= 57 && made < 2; x += 3) {
            if (!occupied(g, x, 15) && !(g->player.x == x && g->player.y == 15) && g->enemy_count < AREA_ENEMY_LIMIT) {
                spawn(g, ENEMY_HOBGOBLIN_GUARD, x, 15);
                Enemy *guard = &g->enemies[g->enemy_count - 1];
                guard->experience = 0;
                guard->revived = 1;
                guard->frozen_turns = 1;
                made++;
            }
        }
        enemy->move_timer = -2;
        push_message(g, "The Mine Boss calls his last two guards from the rear workings!");
        return 1;
    }
    enemy->boss_attack = distance <= 2 ? IRON_PICK_SWEEP : enemy->boss_attack == IRON_ROCKFALL ? IRON_CART_ATTACK : IRON_ROCKFALL;
    enemy->attack_target_x = g->player.x;
    enemy->attack_target_y = enemy->boss_attack == IRON_CART_ATTACK ? 11 : g->player.y;
    enemy->attack_phase = 1;
    enemy->move_timer = enemy->boss_attack == IRON_PICK_SWEEP ? 1 : 2;
    push_message(g, enemy->boss_attack == IRON_CART_ATTACK ? "The Mine Boss signals a cart charge. Leave the marked rail!" : enemy->boss_attack == IRON_ROCKFALL ? "The Mine Boss loosens the roof. Leave the marked rockfall!" : "The Mine Boss raises his pick. Step away!");
    return 1;
}

void ironfang_defeat(GameState *g) {
    if (!g->ironfang_defeated) {
        g->ironfang_defeated = 1;
        g->gold += 180;
        g->score += 1800;
        push_message(g, "The Goblin Mine Boss falls! You recover 180 gold; the war supply operation is broken.");
    }
}

int ironfang_valid(const GameState *g) {
    if (g->ironfang_defeated < 0 || g->ironfang_defeated > 1 ||
        (g->location == LOCATION_IRONFANG && (g->level < 1 || g->level > IRON_DEPTH || !inside(g->player.x, g->player.y) ||
        g->enemy_count < 0 || g->enemy_count > AREA_ENEMY_LIMIT || g->floor_item_count < 0 || g->floor_item_count > MAX_FLOOR_ITEMS))) {
        return 0;
    }
    for (int n = 0; n < IRON_DEPTH; n++) {
        const IronfangStage *state = &g->ironfang[n];
        if ((state->valid != 0 && state->valid != 1) || state->aboard < 0 || state->aboard > 1 ||
            state->route < 0 || state->route > 1 || state->ore_loads < 0 || state->ore_loads > 3 ||
            state->ore_fuse < 0 || state->ore_fuse > 3 || state->enemy_count < 0 || state->enemy_count > AREA_ENEMY_LIMIT ||
            state->loot_count < 0 || state->loot_count > MAX_FLOOR_ITEMS) {
            return 0;
        }
        if (!state->valid) {
            continue;
        }
        int active = g->location == LOCATION_IRONFANG && g->level == n + 1;
        if (active && state->aboard && (g->player.x != state->cart_x || g->player.y != state->cart_y)) {
            return 0;
        }
        if (!inside(state->cart_x, state->cart_y) || !rail(active ? g->map.tiles[state->cart_y][state->cart_x] : state->tiles[state->cart_y][state->cart_x])) {
            return 0;
        }
        for (int y = 0; y < IRON_H; y++) {
            for (int x = 0; x < IRON_W; x++) {
                if (state->tiles[y][x] < TILE_IRON_FLOOR || state->tiles[y][x] > TILE_IRON_EXIT) {
                    return 0;
                }
            }
        }
        for (int i = 0; i < state->loot_count; i++) {
            const FloorItem *item = &state->loot[i];
            if (!inside(item->x, item->y) || item->underlying_tile < TILE_IRON_FLOOR || item->underlying_tile > TILE_IRON_EXIT) {
                return 0;
            }
        }
        for (int e = 0; e < state->enemy_count; e++) {
            const Enemy *enemy = &state->enemies[e];
            if (!inside(enemy->x, enemy->y) || (enemy->type != ENEMY_GOBLIN_MINE_BOSS &&
                (enemy->type < ENEMY_GOBLIN_SCOUT || enemy->type > ENEMY_GOBLIN_SHAMAN))) {
                return 0;
            }
        }
    }
    return g->location != LOCATION_IRONFANG || g->ironfang[g->level - 1].valid;
}

static void road_tile(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] != TILE_PORTAL) {
        g->map.tiles[y][x] = tile;
    }
    for (int n = 0; n < g->floor_item_count; n++) {
        FloorItem *item = &g->floor_items[n];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
            g->map.tiles[y][x] = TILE_ITEM;
        }
    }
    if (g->portal_active && g->portal_location == g->location && g->portal_x == x && g->portal_y == y) {
        g->portal_origin_tile = tile;
    }
}

void ironfang_migrate_road(GameState *g) {
    if (g->location == LOCATION_TOWN4) {
        for (int y = 10; y <= 14; y++) {
            road_tile(g, TOWN_W - 1, y, TILE_TOWN_EXIT);
        }
    } else if (g->location == LOCATION_TOWN_ROAD && g->road_town == LOCATION_TOWN4) {
        int y = TOWN_ROAD_H / 2;
        int from = TOWN_SPRITE_X + TOWN_SPRITE_W;
        for (int x = from; x < TOWN_ROAD_W - 1; x++) {
            road_tile(g, x, y, TILE_TOWN_PATH);
        }
        road_tile(g, from - 1, y, TILE_TOWN_ROAD_ENTRY_EAST);
        road_tile(g, TOWN_ROAD_W - 1, y, TILE_TOWN_ROAD_EXIT_EAST);
    }
}
