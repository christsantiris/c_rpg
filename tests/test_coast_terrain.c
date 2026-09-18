#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include <stdlib.h>
#include <string.h>

static void coast_reachable(const Map *m, unsigned char seen[MAP_H][MAP_W]) {
    int queue[MAP_H * MAP_W];
    int head = 0;
    int tail = 0;
    memset(seen, 0, MAP_H * MAP_W);
    queue[tail++] = m->stairs_up_y * MAP_W + m->stairs_up_x;
    seen[m->stairs_up_y][m->stairs_up_x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MAP_W + offsets[i][0];
            int y = cell / MAP_W + offsets[i][1];
            if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H || seen[y][x] ||
                !map_is_walkable(m, x, y)) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * MAP_W + x;
        }
    }
}

static void use_coast_control(GameState *g) {
    map_room_center(&g->map.rooms[0], &g->player.x, &g->player.y);
    action_resolve_player(g, (Action){ACTION_INTERACT, 0, 0});
}

static void test_coast_routes(void) {
    static GameState g;
    static TileType original[MAP_H][MAP_W];
    unsigned char seen[MAP_H][MAP_W];
    int solvable = 1;
    int chambers = 1;
    int reversible = 1;
    int guarded = 1;
    for (int seed = 0; seed < 256; seed++) {
        srand(seed);
        for (int level = 1; level <= COAST_DEPTH; level++) {
            memset(&g, 0, sizeof(g));
            g.location = LOCATION_COAST;
            g.level = level;
            map_generate_coast(&g.map, level);
            memcpy(original, g.map.tiles, sizeof(original));
            enemies_spawn(&g);
            for (int room = 1; room <= 2; room++) {
                int cx;
                int cy;
                map_room_center(&g.map.rooms[room], &cx, &cy);
                int found = 0;
                for (int i = 0; i < g.enemy_count; i++) {
                    found |= g.enemies[i].active && g.enemies[i].x == cx - 1 && g.enemies[i].y == cy;
                }
                guarded &= found;
            }
            for (int state = 0; state < 2; state++) {
                coast_reachable(&g.map, seen);
                int cx;
                int cy;
                solvable &= seen[g.map.stairs_down_y][g.map.stairs_down_x];
                map_room_center(&g.map.rooms[g.map.room_count - 1], &cx, &cy);
                solvable &= seen[cy][cx];
                map_room_center(&g.map.rooms[0], &cx, &cy);
                solvable &= seen[cy][cx];
                map_room_center(&g.map.rooms[g.map.room_count / 2], &cx, &cy);
                solvable &= seen[cy][cx] &&
                    g.map.tiles[cy][cx + 2] != TILE_COAST_SLUICE_CONTROL;
                map_room_center(&g.map.rooms[1], &cx, &cy);
                chambers &= seen[cy][cx + 1] == state;
                map_room_center(&g.map.rooms[2], &cx, &cy);
                chambers &= seen[cy][cx + 1] == !state;
                use_coast_control(&g);
            }
            reversible &= memcmp(original, g.map.tiles, sizeof(original)) == 0;
        }
    }
    ASSERT("2048 coast stages keep exits, boss rooms and the tide control reachable in both tides", solvable);
    ASSERT("opposite tides switch the guarded treasure chambers", chambers);
    ASSERT("the tide control reverses the water changes", reversible);
    ASSERT("each optional coast treasure chamber has a guardian", guarded);
}

static void test_coast_trap_underlays(void) {
    static Map map;
    srand(42);
    map_generate_coast(&map, 4);
    int room = 3;
    if (room == map.room_count / 2) {
        room += 2;
    }
    int cx;
    int cy;
    map_room_center(&map.rooms[room], &cx, &cy);
    map.tiles[cy][cx + 2] = TILE_TRAP_HIDDEN;
    ASSERT("hidden traps in flooded rooms use shallow-water terrain",
        map_coast_trap_underlay(&map, cx + 2, cy) == TILE_COAST_SHALLOW_WATER);
    ASSERT("flooded traps remain armed before being stepped on",
        map.tiles[cy][cx + 2] == TILE_TRAP_HIDDEN);
    ASSERT("traps on dry coast ground keep their stone underlay",
        map_coast_trap_underlay(&map, cx + 5, cy) == TILE_COAST_FLOOR);
    int no_deep_traps = 1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (map.tiles[y][x] == TILE_TRAP_HIDDEN &&
                map_coast_trap_underlay(&map, x, y) == TILE_COAST_DEEP_WATER) {
                no_deep_traps = 0;
            }
        }
    }
    ASSERT("new coast maps do not hide walkable traps in deep water", no_deep_traps);
    map.tiles[cy - 1][cx + 2] = TILE_COAST_FLOOR;
    map.tiles[cy][cx + 1] = TILE_COAST_FLOOR;
    map.tiles[cy + 1][cx + 2] = TILE_COAST_FLOOR;
    map.tiles[cy][cx + 3] = TILE_COAST_FLOOR;
    ASSERT("a trap surrounded by stone does not appear as isolated water",
        map_coast_trap_underlay(&map, cx + 2, cy) == TILE_COAST_FLOOR);
}

static void test_legacy_sluice_removal(void) {
    static Map map;
    srand(17);
    map_generate_coast(&map, 3);
    Room *room = &map.rooms[map.room_count / 2];
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    for (int y = room->y; y < room->y + room->h; y++) {
        map.tiles[y][cx] = TILE_COAST_WALL;
    }
    map.tiles[cy - 2][cx] = TILE_COAST_DEEP_WATER;
    map.tiles[cy + 2][cx] = TILE_COAST_CHANNEL_DRY;
    map.tiles[cy][cx + 2] = TILE_COAST_SLUICE_CONTROL;
    ASSERT("legacy sluice is detected and removed",
        map_remove_coast_sluice(&map));
    int open = 1;
    for (int y = room->y; y < room->y + room->h; y++) {
        open &= map.tiles[y][cx] == TILE_COAST_FLOOR;
    }
    ASSERT("legacy sluice wall and crossings become open floor",
        open && map.tiles[cy][cx + 2] == TILE_COAST_FLOOR);
    ASSERT("legacy sluice repair is safe to repeat",
        !map_remove_coast_sluice(&map));
}

static void test_coast_water_items(void) {
    static GameState g;
    memset(&g, 0, sizeof(g));
    game_init(&g);
    g.location = LOCATION_COAST;
    g.level = 2;
    map_generate_coast(&g.map, g.level);
    g.enemy_count = 0;
    int cx;
    int cy;
    map_room_center(&g.map.rooms[1], &cx, &cy);
    int water_x = cx;
    int water_y = cy - 2;
    ASSERT("flooded chambers start unexplored",
        !map_is_explored(&g.map, water_x, water_y));
    use_coast_control(&g);
    ASSERT("tide changes reveal affected water on the minimap",
        map_is_explored(&g.map, water_x, water_y));
    g.player.x = water_x;
    g.player.y = water_y;
    g.inventory_count = 1;
    g.inventory[0] = item_make_health_potion();
    action_resolve_player(&g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("dropped items retain tidal terrain on the map",
        g.map.tiles[water_y][water_x] == TILE_COAST_DRAINED_WATER);
    g.enemy_count = 2;
    g.enemies[0].active = 1;
    g.enemies[0].x = water_x;
    g.enemies[0].y = water_y;
    g.enemies[1].active = 1;
    g.enemies[1].x = water_x - 1;
    g.enemies[1].y = water_y;
    use_coast_control(&g);
    ASSERT("rising water submerges loot without creating a walkable item tile",
        !map_is_walkable(&g.map, water_x, water_y) &&
        g.floor_items[0].underlying_tile == TILE_COAST_DEEP_WATER);
    ASSERT("rising water moves enemies to distinct walkable banks",
        map_is_walkable(&g.map, g.enemies[0].x, g.enemies[0].y) &&
        !(g.enemies[0].x == g.enemies[1].x && g.enemies[0].y == g.enemies[1].y) &&
        !(g.enemies[0].x == g.player.x && g.enemies[0].y == g.player.y));
    ASSERT("tide controls leave the player on safe ground",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    use_coast_control(&g);
    g.player.x = water_x;
    g.player.y = water_y;
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("submerged loot can be recovered after draining again",
        g.inventory_count == 1 && !g.floor_items[0].active &&
        g.map.tiles[water_y][water_x] == TILE_COAST_DRAINED_WATER);
    use_coast_control(&g);
    g.inventory_count = 1;
    g.inventory[0] = item_make_health_potion();
    action_resolve_player(&g, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("loot cannot hide or disable the tide control", game_has_regional_interaction(&g));
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("A still toggles a control beneath dropped loot",
        g.map.tiles[water_y][water_x] == TILE_COAST_DRAINED_WATER);
    g.enemy_count = 0;
    map_room_center(&g.map.rooms[1], &cx, &cy);
    g.player.x = cx + 1;
    g.player.y = cy;
    g.inventory_count = 1;
    g.inventory[0] = item_make_health_potion();
    action_resolve_player(&g, (Action){ACTION_DROP_ITEM, 0, 0});
    int gold = g.gold;
    ASSERT("A does not treat the flooded-chamber cache as a mechanism", !game_has_regional_interaction(&g));
    action_resolve_player(&g, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("interacting does not claim the chamber reward",
        g.gold == gold && g.map.tiles[cy][cx + 1] == TILE_COAST_CACHE);
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("P claims the chamber reward before loose loot",
        g.gold == gold + 70 && g.inventory_count == 0);
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("P still retrieves loot dropped on the claimed cache", g.inventory_count == 1);
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("a guarded cache grants its larger reward only once",
        g.gold == gold + 70 && g.map.tiles[cy][cx + 1] == TILE_COAST_FLOOR);
}

static void test_coast_persistence(void) {
    static GameState g;
    static GameState loaded;
    static Map expected;
    memset(&g, 0, sizeof(g));
    game_init(&g);
    g.location = LOCATION_COAST;
    g.level = 2;
    map_generate_coast(&g.map, g.level);
    use_coast_control(&g);
    int x;
    int y;
    map_room_center(&g.map.rooms[1], &x, &y);
    g.player.x = x + 1;
    g.player.y = y;
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    expected = g.map;
    game_descend(&g);
    game_ascend(&g);
    ASSERT("tides, looted chambers and discoveries survive cached revisits",
        memcmp(&g.map, &expected, sizeof(Map)) == 0);
    push_message(&g, "Blue channels drain; amber channels rise.");
    const int slot = 99010;
    if (save_exists(slot)) {
        ASSERT("coast test save slot must be unused", 0);
        return;
    }
    int saved = save_game(&g, slot);
    int restored = saved && load_game(&loaded, slot);
    ASSERT("coast mechanics save and load", restored);
    if (restored) {
        ASSERT("full tide status survives save/load",
            strcmp(loaded.messages[loaded.message_count - 1],
                "Blue channels drain; amber channels rise.") == 0);
        ASSERT("active tide, loot and minimap state round trip",
            memcmp(&loaded.map, &expected, sizeof(Map)) == 0);
        ASSERT("cached tide, loot and minimap state round trip",
            memcmp(&loaded.coast_cache[1].map, &expected, sizeof(Map)) == 0);
        use_coast_control(&loaded);
        ASSERT("loaded tide control still reverses the treasure chambers",
            loaded.map.tiles[y - 2][x] == TILE_COAST_DEEP_WATER &&
            loaded.map.tiles[y][x + 1] == TILE_COAST_FLOOR);
    }
    remove("saves/savegame_99010.json");
}

void test_coast_terrain(void) {
    printf("Coast terrain tests:\n");
    test_coast_routes();
    test_coast_trap_underlays();
    test_legacy_sluice_removal();
    test_coast_water_items();
    test_coast_persistence();
}
