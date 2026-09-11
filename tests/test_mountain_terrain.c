#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static int find_mountain_tile(const Map *m, TileType tile, int *tx, int *ty) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (m->tiles[y][x] == tile) {
                *tx = x;
                *ty = y;
                return 1;
            }
        }
    }
    return 0;
}

static int required_route_is_solvable(const Map *m) {
    unsigned char seen[MAP_H][MAP_W] = {{0}};
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    queue[tail++] = m->stairs_up_y * MAP_W + m->stairs_up_x;
    seen[m->stairs_up_y][m->stairs_up_x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MAP_W + offsets[i][0];
            int y = cell / MAP_W + offsets[i][1];
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H || seen[y][x]) {
                continue;
            }
            // Gates and broken spans are operable from any adjacent floor.
            // Optional rockfalls stay closed: no required route can depend on HP.
            if (!map_is_walkable(m, x, y) && m->tiles[y][x] != TILE_MOUNTAIN_GATE &&
                m->tiles[y][x] != TILE_MOUNTAIN_CHASM) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * MAP_W + x;
        }
    }
    if (!seen[m->stairs_down_y][m->stairs_down_x]) {
        return 0;
    }
    for (int i = 0; i < m->room_count; i++) {
        int x;
        int y;
        map_room_center(&m->rooms[i], &x, &y);
        if (!seen[y][x]) {
            return 0;
        }
    }
    return 1;
}

static void test_mountain_generation(void) {
    static GameState g;
    int solvable = 1;
    int bridges = 1;
    int guarded = 1;
    for (int seed = 0; seed < 256; seed++) {
        srand(seed);
        for (int level = 1; level <= MOUNTAIN_DEPTH; level++) {
            memset(&g, 0, sizeof(g));
            g.location = LOCATION_MOUNTAINS;
            g.level = level;
            g.dain_quest_state = 1;
            map_generate_mountains(&g.map, level);
            if (!required_route_is_solvable(&g.map)) {
                printf("Unsolvable mountain seed %d stage %d\n", seed, level);
                solvable = 0;
            }
            int x = 0;
            int y = 0;
            if (level == 2 || level == 7) {
                if (!find_mountain_tile(&g.map, TILE_MOUNTAIN_WEAK_BRIDGE, &x, &y)) {
                    bridges = 0;
                } else {
                    int neighbors = map_is_walkable(&g.map, x - 1, y) +
                        map_is_walkable(&g.map, x + 1, y) +
                        map_is_walkable(&g.map, x, y - 1) +
                        map_is_walkable(&g.map, x, y + 1);
                    bridges &= neighbors == 2;
                    g.map.tiles[y][x] = TILE_MOUNTAIN_CHASM;
                    solvable &= required_route_is_solvable(&g.map);
                }
            }
            if (find_mountain_tile(&g.map, TILE_MOUNTAIN_GATE, &x, &y)) {
                enemies_spawn(&g);
                int archer = 0;
                int guard = 0;
                for (int i = 0; i < g.enemy_count; i++) {
                    Enemy *e = &g.enemies[i];
                    archer |= e->type == ENEMY_GOBLIN_ARCHER && e->x == x + 2 && e->y == y;
                    guard |= e->type == ENEMY_HOBGOBLIN_GUARD && e->x == x + 2 && e->y == y + 1;
                }
                guarded &= archer && guard && g.map.tiles[y][x + 1] == TILE_TRAP_REVEALED;
            }
        }
    }
    ASSERT("2048 generated mountain stages remain solvable without risky caves", solvable);
    ASSERT("weak bridges always form a one-tile combat bottleneck", bridges);
    ASSERT("fort gates always have a trap, archer and shortcut guard", guarded);
    ASSERT("new terrain preserves existing serialized tile IDs",
        TILE_MOUNTAIN_CAVE_FLOOR == 43 && TILE_COAST_FLOOR == 37 && TILE_WATCHTOWER == 65);
}

static void test_mountain_interactions(void) {
    static GameState g;
    memset(&g, 0, sizeof(g));
    game_init(&g);
    g.location = LOCATION_MOUNTAINS;
    g.level = 4;
    map_generate_mountains(&g.map, g.level);
    g.enemy_count = 0;
    int x = 0;
    int y = 0;
    int found = find_mountain_tile(&g.map, TILE_MOUNTAIN_GATE, &x, &y);
    ASSERT("fort contains an operable gate", found);
    if (!found) {
        return;
    }
    Action interact = {ACTION_INTERACT, 0, 0};
    for (int side = -1; side <= 1; side += 2) {
        g.map.tiles[y][x] = TILE_MOUNTAIN_GATE;
        g.player.x = x + side;
        g.player.y = y;
        ASSERT("A recognizes the gate from either side", game_has_regional_interaction(&g));
        Action move = {ACTION_MOVE, x, y};
        action_resolve_player(&g, move);
        ASSERT("bumping a gate does not open it", g.map.tiles[y][x] == TILE_MOUNTAIN_GATE);
        action_resolve_player(&g, interact);
        ASSERT("A opens the gate without moving the player",
            map_is_walkable(&g.map, x, y) && g.player.x == x + side);
    }
    for (int side = -1; side <= 1; side += 2) {
        map_generate_mountains(&g.map, g.level);
        find_mountain_tile(&g.map, TILE_MOUNTAIN_GATE, &x, &y);
        g.player.x = x + side * 3;
        g.player.y = y + 1;
        int hp = g.player.hp;
        action_resolve_player(&g, interact);
        ASSERT("clearing either rockfall reveals the whole cave at an HP cost",
            g.player.hp == hp - 8 && g.map.tiles[y + 2][x] == TILE_MOUNTAIN_CACHE &&
            g.map.tiles[y + 2][x - 3] == TILE_MOUNTAIN_CAVE_FLOOR &&
            g.map.tiles[y + 2][x + 3] == TILE_MOUNTAIN_CAVE_FLOOR);
        ASSERT("cave discovery updates exploration", map_is_explored(&g.map, x, y + 2));
        g.player.x = x - 3;
        g.player.y = y + 2;
        for (int cx = x - 2; cx <= x + 3; cx++) {
            Action move = {ACTION_MOVE, cx, y + 2};
            action_resolve_player(&g, move);
        }
        ASSERT("cave crosses the fort with its gate still closed",
            g.player.x == x + 3 && g.map.tiles[y][x] == TILE_MOUNTAIN_GATE);
        g.player.x = x;
        g.inventory_count = 1;
        g.inventory[0] = item_make_health_potion();
        Action drop = {ACTION_DROP_ITEM, 0, 0};
        action_resolve_player(&g, drop);
        ASSERT("dropped loot leaves the cache interactive", game_has_regional_interaction(&g));
        int gold = g.gold;
        action_resolve_player(&g, interact);
        action_resolve_player(&g, interact);
        ASSERT("risky cache gives its larger reward only once", g.gold == gold + 80);
        Action pickup = {ACTION_PICK_UP, 0, 0};
        action_resolve_player(&g, pickup);
        ASSERT("picking up loot cannot restore a claimed cache",
            g.map.tiles[y + 2][x] == TILE_MOUNTAIN_CAVE_FLOOR);
    }
    g.level = 2;
    map_generate_mountains(&g.map, g.level);
    found = find_mountain_tile(&g.map, TILE_MOUNTAIN_WEAK_BRIDGE, &x, &y);
    ASSERT("bridge stage contains a weak span", found);
    if (!found) {
        return;
    }
    int dx = map_is_walkable(&g.map, x + 1, y) ? 1 : 0;
    int dy = dx ? 0 : 1;
    for (int side = -1; side <= 1; side += 2) {
        g.map.tiles[y][x] = TILE_MOUNTAIN_WEAK_BRIDGE;
        g.player.x = x;
        g.player.y = y;
        Action move = {ACTION_MOVE, x + side * dx, y + side * dy};
        action_resolve_player(&g, move);
        ASSERT("span collapses behind the player onto solid ground",
            !map_is_walkable(&g.map, x, y) && g.player.x == move.target_x &&
            g.player.y == move.target_y && map_is_explored(&g.map, x, y));
        action_resolve_player(&g, interact);
        ASSERT("A repairs a collapse from either bank",
            g.map.tiles[y][x] == TILE_MOUNTAIN_BRIDGE);
    }
    g.map.tiles[y][x] = TILE_MOUNTAIN_WEAK_BRIDGE;
    g.player.x = x;
    g.player.y = y;
    g.inventory_count = 1;
    g.inventory[0] = item_make_health_potion();
    action_resolve_player(&g, (Action){ACTION_DROP_ITEM, 0, 0});
    action_resolve_player(&g, (Action){ACTION_MOVE, x + dx, y + dy});
    ASSERT("loot cannot prevent bridge collapse", g.map.tiles[y][x] == TILE_MOUNTAIN_CHASM);
    action_resolve_player(&g, interact);
    action_resolve_player(&g, (Action){ACTION_MOVE, x, y});
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("loot pickup preserves the repaired span", g.map.tiles[y][x] == TILE_MOUNTAIN_BRIDGE);
}

static void test_mountain_persistence(void) {
    static GameState g;
    static GameState loaded;
    static Map expected;
    memset(&g, 0, sizeof(g));
    game_init(&g);
    g.location = LOCATION_MOUNTAINS;
    g.level = 4;
    map_generate_mountains(&g.map, g.level);
    int x = 0;
    int y = 0;
    find_mountain_tile(&g.map, TILE_MOUNTAIN_GATE, &x, &y);
    g.player.x = x - 3;
    g.player.y = y + 1;
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    expected = g.map;
    game_descend(&g);
    game_ascend(&g);
    ASSERT("cave reward and discovered route survive a cached revisit",
        memcmp(&g.map, &expected, sizeof(Map)) == 0);
    g.player.x = x;
    g.player.y = y + 2;
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    g.player.x = x - 1;
    g.player.y = y;
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    expected = g.map;
    game_descend(&g);
    game_ascend(&g);
    ASSERT("opened gate and looted cave survive a cached revisit",
        memcmp(&g.map, &expected, sizeof(Map)) == 0);
    map_generate_mountains(&g.mountain_cache[1].map, 2);
    g.mountain_cache[1].valid = 1;
    find_mountain_tile(&g.mountain_cache[1].map, TILE_MOUNTAIN_WEAK_BRIDGE, &x, &y);
    g.mountain_cache[1].map.tiles[y][x] = TILE_MOUNTAIN_CHASM;
    const int slot = 99009;
    if (save_exists(slot)) {
        ASSERT("mountain test save slot must be unused", 0);
        return;
    }
    int saved = save_game(&g, slot);
    int restored = saved && load_game(&loaded, slot);
    ASSERT("mountain mechanics save and load successfully", restored);
    if (restored) {
        ASSERT("active mountain tile and minimap state round trip",
            memcmp(&loaded.map, &expected, sizeof(Map)) == 0);
        ASSERT("cached collapse survives save/load",
            loaded.mountain_cache[1].map.tiles[y][x] == TILE_MOUNTAIN_CHASM);
        ASSERT("cached claimed cave and open gate survive save/load",
            memcmp(&loaded.mountain_cache[3].map, &expected, sizeof(Map)) == 0);
    }
    remove("saves/savegame_99009.json");
}

void test_mountain_terrain(void) {
    printf("Mountain terrain tests:\n");
    test_mountain_generation();
    test_mountain_interactions();
    test_mountain_persistence();
}
