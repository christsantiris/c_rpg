#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/regional_terrain.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;
static Map previous;
static unsigned char seen[MAP_H][MAP_W];

static int find_tile(const Map *m, TileType kind, int *x, int *y) {
    for (int ty = 0; ty < MAP_H; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            if (m->tiles[ty][tx] == kind) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static void reachable(const Map *m, int avoid_heat) {
    int queue[MAP_W * MAP_H];
    int head = 0;
    int tail = 0;
    memset(seen, 0, sizeof(seen));
    queue[tail++] = m->stairs_up_y * MAP_W + m->stairs_up_x;
    seen[m->stairs_up_y][m->stairs_up_x] = 1;
    static const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    while (head < tail) {
        int cell = queue[head++];
        for (int i = 0; i < 4; i++) {
            int x = cell % MAP_W + offsets[i][0];
            int y = cell / MAP_W + offsets[i][1];
            if (!map_is_walkable(m, x, y) || seen[y][x] ||
                (avoid_heat && regional_terrain_heat(m->tiles[y][x]))) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * MAP_W + x;
        }
    }
}

static void test_layouts(void) {
    int connected = 1;
    int safe = 1;
    int distinct = 1;
    int features = 1;
    int heat_cells = 0;
    int gates = 0;
    for (int seed = 0; seed < 128; seed++) {
        for (int level = 1; level <= 5; level++) {
            srand(seed);
            map_generate_ashen(&previous, level);
            srand(seed);
            map_generate_glassdeep(&game.map, level);
            int intersection = 0;
            int total = 0;
            for (int y = 0; y < SWAMP_MAP_H; y++) {
                for (int x = 0; x < SWAMP_MAP_W; x++) {
                    int a = map_is_walkable(&previous, x, SWAMP_MAP_H - 1 - y);
                    int b = map_is_walkable(&game.map, x, y);
                    intersection += a && b;
                    total += a || b;
                    heat_cells += regional_terrain_heat(previous.tiles[y][x]);
                    gates += game.map.tiles[y][x] == TILE_STONEWAKE_CRANE_NEW;
                }
            }
            distinct &= intersection * 100 < total * 80;
            for (int region = 0; region < 2; region++) {
                const Map *m = region ? &game.map : &previous;
                reachable(m, 0);
                connected &= seen[m->stairs_down_y][m->stairs_down_x];
                for (int i = 0; i < m->room_count; i++) {
                    int x;
                    int y;
                    map_room_center(&m->rooms[i], &x, &y);
                    connected &= seen[y][x];
                }
                for (int ty = 0; ty < SWAMP_MAP_H; ty++) {
                    for (int tx = 0; tx < SWAMP_MAP_W; tx++) {
                        if (map_is_walkable(m, tx, ty) && m->tiles[ty][tx] != TILE_STONEWAKE_LEDGE) {
                            connected &= seen[ty][tx];
                        }
                    }
                }
                int x;
                int y;
                features &= find_tile(m, region ? TILE_STONEWAKE_CRANE_NEW : TILE_ASHEN_VALVE, &x, &y);
                features &= seen[y][x];
            }
            reachable(&previous, 1);
            safe &= seen[previous.stairs_down_y][previous.stairs_down_x];
        }
    }
    ASSERT("1280 new volcanic and quarry layouts keep every chamber, control and exit reachable", connected && features);
    ASSERT("every volcanic stage has an exit route avoiding all cycling heat", safe);
    ASSERT("Stonewake does not mirrors Ashen Hollow's walking layout", distinct);
    ASSERT("all stages include real crossing hazards or a crane-controlled shortcut", heat_cells >= 640 * 9 && gates == 1280);
}

static void setup_heat(void) {
    game_init(&game);
    game.location = LOCATION_ASHEN;
    game.level = 2;
    map_generate_ashen(&game.map, 2);
    game.enemy_count = 0;
    game.floor_item_count = 0;
    find_tile(&game.map, TILE_ASHEN_COLD_6, &game.player.x, &game.player.y);
    game.player.hp = 100;
    game.player.defense = 0;
}

static void test_heat(void) {
    setup_heat();
    int x = game.player.x;
    int y = game.player.y;
    int harmless = 1;
    for (int i = 0; i < 6; i++) {
        harmless &= regional_terrain_tick(&game) == 0;
    }
    ASSERT("cool crossings give six actions before a two-action orange warning", harmless && game.map.tiles[y][x] == TILE_ASHEN_WARN_2);
    ASSERT("the first warning action does no damage", regional_terrain_tick(&game) == 0 && game.map.tiles[y][x] == TILE_ASHEN_WARN_1);
    ASSERT("crossings flare only after both warned actions", regional_terrain_tick(&game) == 8 && game.map.tiles[y][x] == TILE_ASHEN_ACTIVE_2);
    ASSERT("heat lasts two actions then cools without lingering damage", regional_terrain_tick(&game) == 8 && regional_terrain_tick(&game) == 0 && game.map.tiles[y][x] == TILE_ASHEN_COLD_6);
    game.map.tiles[y][x] = TILE_ASHEN_WARN_1;
    action_resolve_enemies(&game);
    ASSERT("ordinary action resolution applies crossing damage through the combat system", game.player.hp == 92);
    game.map.tiles[y][x] = TILE_ASHEN_WARN_1;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_ASHEN_WARN_1, .item = item_make_health_potion()};
    game.map.tiles[y][x] = TILE_ITEM;
    regional_terrain_tick(&game);
    ASSERT("loot does not hide or freeze crossing heat", game.map.tiles[y][x] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_ASHEN_ACTIVE_2);
    regional_terrain_cancel_landing(&game, x, y);
    ASSERT("return portals cool the landing crossing without losing loose loot", game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_ASHEN_COLD_6);
    for (int cls = 0; cls < 3; cls++) {
        setup_heat();
        game.player.player_class = cls;
        find_tile(&game.map, TILE_ASHEN_COLD_6, &x, &y);
        int vx, vy;
        find_tile(&game.map, TILE_ASHEN_VALVE, &vx, &vy);
        game.map.tiles[y][x] = TILE_ASHEN_ACTIVE_2;
        game.player.x = vx;
        game.player.y = vy;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("every class can divert a crossing with the contextual interact action", game_has_regional_interaction(&game) && game.map.tiles[y][x] == TILE_ASHEN_BASALT_CROSSING && game.map.tiles[vy][vx] == TILE_ASHEN_VALVE_DIVERTED);
    }
}

static void test_diversions(void) {
    setup_heat();
    // A real stage-two visit already has a stage-one cache to backtrack into.
    LevelCache *first = &game.ashen_cache[0];
    first->valid = 1;
    map_generate_ashen(&first->map, 1);
    first->enemy_count = 1;
    first->enemies[0] = (Enemy){.type = ENEMY_ASH_HOUND, .hp = 0, .max_hp = 60};
    int vx, vy;
    find_tile(&game.map, TILE_ASHEN_VALVE, &vx, &vy);
    game.player.x = vx;
    game.player.y = vy;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    int stable = 1;
    for (int turn = 0; turn < 40; turn++) {
        action_resolve_enemies(&game);
    }
    for (int y = vy - 4; y <= vy - 2; y++) {
        for (int x = vx + 2; x <= vx + 4; x++) {
            stable &= game.map.tiles[y][x] == TILE_ASHEN_BASALT_CROSSING;
        }
    }
    int other_x, other_y;
    ASSERT("diversion solidifies the entire linked crossing while other crossings keep cycling", stable && find_tile(&game.map, TILE_ASHEN_COLD_6, &other_x, &other_y));
    int walked = 1;
    for (int step = 0; step < 9; step++) {
        int x = game.player.x + (step < 3 ? 1 : 0);
        int y = game.player.y - (step >= 3 ? 1 : 0);
        action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
        action_resolve_enemies(&game);
        walked &= game.player.x == x && game.player.y == y && game.player.hp == 100;
    }
    ASSERT("players can wait, walk from the valve and cross to the far bank without racing heat", walked && game.player.x == vx + 3 && game.player.y == vy - 6);
    game.player.x = vx;
    game.player.y = vy;
    previous = game.map;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("reusing a diverted valve never reheats its basalt path", memcmp(&previous, &game.map, sizeof(Map)) == 0);
    int up_x = game.map.stairs_up_x;
    int up_y = game.map.stairs_up_y;
    game.player.x = up_x;
    game.player.y = up_y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, up_x, up_y});
    up_x = game.map.stairs_up_x;
    up_y = game.map.stairs_up_y;
    game.player.x = up_x;
    game.player.y = up_y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, up_x, up_y});
    game_enter_ashen(&game);
    ASSERT("a new expedition preserves diverted paths while refreshing ordinary enemies", game.location == LOCATION_ASHEN && game.level == 1 && game.enemy_count > 0);
    // Return to the stage where the diversion was made through its real cache.
    int down_x = game.map.stairs_down_x;
    int down_y = game.map.stairs_down_y;
    game.player.x = down_x;
    game.player.y = down_y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, down_x, down_y});
    ASSERT("stage backtracking retains the diverted valve and solid crossing", game.level == 2 && game.map.tiles[vy][vx] == TILE_ASHEN_VALVE_DIVERTED && game.map.tiles[vy - 3][vx + 3] == TILE_ASHEN_BASALT_CROSSING);

    setup_heat();
    find_tile(&game.map, TILE_ASHEN_VALVE, &vx, &vy);
    int x = vx + 3;
    int y = vy - 3;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_ASHEN_ACTIVE_1, .item = item_make_health_potion()};
    game.map.tiles[y][x] = TILE_ITEM;
    game.portal_active = 1;
    game.portal_location = LOCATION_ASHEN;
    game.portal_level = 2;
    game.portal_x = x + 1;
    game.portal_y = y;
    game.portal_origin_tile = TILE_ASHEN_WARN_1;
    game.map.tiles[y][x + 1] = TILE_PORTAL;
    game.player.x = vx;
    game.player.y = vy;
    regional_terrain_interact(&game);
    regional_terrain_cancel_landing(&game, x + 1, y);
    for (int turn = 0; turn < 20; turn++) {
        regional_terrain_tick(&game);
    }
    ASSERT("diversion preserves loot and portals while solidifying their underlying crossing", game.map.tiles[y][x] == TILE_ITEM && game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_ASHEN_BASALT_CROSSING && game.map.tiles[y][x + 1] == TILE_PORTAL && game.portal_origin_tile == TILE_ASHEN_BASALT_CROSSING);
}

static void setup_light(int *x, int *y) {
    game_init(&game);
    game.location = LOCATION_GLASSDEEP;
    game.level = 2;
    map_generate_glassdeep(&game.map, 2);
    game.enemy_count = 0;
    game.floor_item_count = 0;
    // Explicit legacy optics fixture; new Stonewake floors use cranes instead.
    game.map.tiles[0][0] = TILE_GLASSDEEP_WALL;
    *x = 30;
    *y = 30;
    for (int ty = 24; ty <= 31; ty++) {
        for (int tx = 25; tx <= 38; tx++) {
            game.map.tiles[ty][tx] = TILE_GLASSDEEP_FLOOR;
        }
    }
    game.map.tiles[30][26] = TILE_GLASS_SOURCE;
    game.map.tiles[30][30] = TILE_GLASS_FACE_E;
    game.map.tiles[26][30] = TILE_GLASS_RECEIVER;
    game.map.tiles[25][30] = TILE_GLASS_SEAL_CLOSED;
    game.player.x = *x;
    game.player.y = *y;
}

static void test_light(void) {
    int x, y;
    setup_light(&x, &y);
    ASSERT("incoming crystal light and the east-facing beam have visible paths", regional_terrain_light(&game, x - 2, y) == 1 && regional_terrain_light(&game, x + 2, y) == 1);
    for (int i = 0; i < 3; i++) {
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    }
    ASSERT("turning a face north lights its receiver and opens the actual shortcut", game.map.tiles[y][x] == TILE_GLASS_FACE_N && regional_terrain_light(&game, x, y - 4) == 2 && game.map.tiles[y - 5][x] == TILE_GLASS_SEAL_OPEN && map_is_walkable(&game.map, x, y - 5));
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_BLIND_STALKER, .x = x + 6, .y = y, .hp = 40, .max_hp = 40};
    regional_terrain_interact(&game);
    ASSERT("turning a crystal wakes nearby Stalkers without changing their health", game.enemies[0].move_timer == 1 && game.enemies[0].hp == 40);
    ASSERT("redirecting light away closes the unoccupied shortcut", game.map.tiles[y - 5][x] == TILE_GLASS_SEAL_CLOSED && !map_is_walkable(&game.map, x, y - 5));
    game.map.tiles[y][x] = TILE_GLASS_FACE_W;
    game.map.tiles[y - 2][x] = TILE_GLASSDEEP_WALL;
    regional_terrain_interact(&game);
    ASSERT("a blocked reflected beam cannot open the seal or draw through rock", game.map.tiles[y - 5][x] == TILE_GLASS_SEAL_CLOSED && !regional_terrain_light(&game, x, y - 3));
    game.map.tiles[y - 2][x] = TILE_GLASSDEEP_FLOOR;
    game.map.tiles[y][x] = TILE_GLASS_FACE_W;
    game.map.tiles[y][x - 2] = TILE_GLASSDEEP_WALL;
    regional_terrain_interact(&game);
    ASSERT("blocking the light source also prevents the receiver from opening the gate", game.map.tiles[y - 5][x] == TILE_GLASS_SEAL_CLOSED && !regional_terrain_light(&game, x, y - 2));
    setup_light(&x, &y);
    game.map.tiles[y][x] = TILE_GLASS_FACE_N;
    game.map.tiles[y - 5][x] = TILE_GLASS_SEAL_OPEN;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y - 5, .underlying_tile = TILE_GLASS_SEAL_OPEN, .item = item_make_health_potion()};
    regional_terrain_interact(&game);
    ASSERT("a crystal seal cannot close on dropped loot", game.map.tiles[y - 5][x] == TILE_GLASS_SEAL_OPEN && game.floor_items[0].active);
    game.floor_item_count = 0;
    game.map.tiles[y][x] = TILE_GLASS_FACE_N;
    game.portal_active = 1;
    game.portal_location = LOCATION_GLASSDEEP;
    game.portal_level = 2;
    game.portal_x = x;
    game.portal_y = y - 5;
    regional_terrain_interact(&game);
    ASSERT("a seal cannot close across an existing return-portal landing", game.map.tiles[y - 5][x] == TILE_GLASS_SEAL_OPEN);
}

static void legacy_map(Map *m, int glass) {
    memset(m, 0, sizeof(*m));
    m->room_count = 9;
    static const int route[9] = {6, 7, 8, 5, 4, 3, 0, 1, 2};
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = glass ? TILE_GLASSDEEP_WALL : TILE_ASHEN_WALL;
        }
    }
    for (int i = 0; i < 9; i++) {
        int anchor = route[i];
        int top = 5 + anchor / 3 * 20;
        Room *room = &m->rooms[i];
        *room = (Room){5 + anchor % 3 * 23, glass ? SWAMP_MAP_H - top - 10 : top, 14, 10};
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                m->tiles[y][x] = glass ? TILE_GLASSDEEP_FLOOR : TILE_ASHEN_FLOOR;
            }
        }
    }
    map_room_center(&m->rooms[0], &m->stairs_up_x, &m->stairs_up_y);
    map_room_center(&m->rooms[8], &m->stairs_down_x, &m->stairs_down_y);
}

static int rewrite_version(int slot, int version) {
    char path[80];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", slot);
    FILE *file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc(size + 1, 1);
    fread(data, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    test_mark_legacy_portal(root);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    if (!file) {
        free(data);
        return 0;
    }
    fputs(data, file);
    fclose(file);
    free(data);
    return 1;
}

static void test_saves(void) {
    const int slot = 99210;
    if (save_exists(slot)) {
        ASSERT("regional terrain save slot must be unused", 0);
        return;
    }
    setup_heat();
    game.map.tiles[game.player.y][game.player.x] = TILE_ASHEN_WARN_1;
    map_generate_glassdeep(&game.glassdeep_cache[1].map, 2);
    game.glassdeep_cache[1].valid = 1;
    int x, y;
    find_tile(&game.glassdeep_cache[1].map, TILE_STONEWAKE_CRANE_NEW, &x, &y);
    game.glassdeep_cache[1].map.tiles[y][x] = TILE_STONEWAKE_CRANE_DOWN;
    for (int dx = 1; dx <= 9; dx++) {
        game.glassdeep_cache[1].map.tiles[y][x + dx] = TILE_STONEWAKE_BRIDGE;
    }
    int ok = save_game(&game, slot) && load_game(&loaded, slot);
    ASSERT("heat warnings, crane angles and walkway states round trip exactly", ok && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0 && memcmp(&game.glassdeep_cache[1].map, &loaded.glassdeep_cache[1].map, sizeof(Map)) == 0);
    setup_heat();
    int vx, vy;
    find_tile(&game.map, TILE_ASHEN_VALVE, &vx, &vy);
    game.player.x = vx;
    game.player.y = vy;
    previous = game.map;
    ok = save_game(&game, slot) && rewrite_version(slot, 144) && load_game(&loaded, slot);
    ASSERT("version 144 keeps existing crossings and player progress without regenerating maps", ok && memcmp(&previous, &loaded.map, sizeof(Map)) == 0 && loaded.player.x == vx && loaded.player.y == vy);
    regional_terrain_interact(&loaded);
    ASSERT("existing version 144 valves gain permanent diversion without a map migration", loaded.map.tiles[vy][vx] == TILE_ASHEN_VALVE_DIVERTED && loaded.map.tiles[vy - 3][vx + 3] == TILE_ASHEN_BASALT_CROSSING);
    game = loaded;
    game.ashen_cache[1].valid = 1;
    game.ashen_cache[1].map = game.map;
    ok = save_game(&game, slot) && load_game(&loaded, slot);
    ASSERT("diverted valves and stable paths round trip in active and cached maps", ok && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0 && memcmp(&game.ashen_cache[1].map, &loaded.ashen_cache[1].map, sizeof(Map)) == 0);
    for (int glass = 0; glass < 2; glass++) {
        game_init(&game);
        game.location = glass ? LOCATION_GLASSDEEP : LOCATION_ASHEN;
        game.level = 2;
        legacy_map(&game.map, glass);
        map_room_center(&game.map.rooms[0], &game.player.x, &game.player.y);
        game.gold = 432;
        game.enemy_count = 2;
        game.enemies[0] = (Enemy){.active = 1, .type = glass ? ENEMY_BLIND_STALKER : ENEMY_ASH_HOUND, .x = game.player.x + 1, .y = game.player.y, .hp = 11, .max_hp = 60};
        game.enemies[1] = game.enemies[0];
        game.enemies[1].active = 0;
        game.enemies[1].hp = 0;
        game.defeated_bosses |= 1 << game.location;
        game.portal_active = 1;
        game.portal_location = game.location;
        game.portal_level = 2;
        game.portal_x = game.player.x;
        game.portal_y = game.player.y;
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = game.player.x, .y = game.player.y, .underlying_tile = glass ? TILE_GLASSDEEP_FLOOR : TILE_ASHEN_FLOOR, .item = item_make_health_potion()};
        game.map.tiles[game.player.y][game.player.x] = TILE_ITEM;
        map_mark_explored(&game.map, game.player.x, game.player.y);
        LevelCache *cache = glass ? &game.glassdeep_cache[1] : &game.ashen_cache[1];
        cache->valid = 1;
        cache->map = game.map;
        previous = game.map;
        ok = save_game(&game, slot) && rewrite_version(slot, 143) && load_game(&loaded, slot);
        int retained = ok;
        int features = 0;
        for (int ty = 0; ty < MAP_H; ty++) {
            for (int tx = 0; tx < MAP_W; tx++) {
                if (map_is_walkable(&previous, tx, ty)) {
                    retained &= map_is_walkable(&loaded.map, tx, ty);
                }
                features += regional_terrain_tile(loaded.map.tiles[ty][tx]);
            }
        }
        ASSERT("older regional maps gain interactive terrain without blocking existing walking squares", retained && features > 0);
        ASSERT("migration preserves position, enemy wounds and deaths, loot, portals and victories", ok && loaded.player.x == game.player.x && loaded.player.y == game.player.y && loaded.enemies[0].hp == 11 && !loaded.enemies[1].active && loaded.floor_items[0].active && loaded.map.tiles[game.player.y][game.player.x] == TILE_ITEM && loaded.portal_x == game.portal_x && loaded.portal_y == game.portal_y && loaded.gold == 432 && loaded.defeated_bosses == game.defeated_bosses && map_is_explored(&loaded.map, game.player.x, game.player.y));
        previous = loaded.map;
        regional_terrain_migrate(&loaded);
        ASSERT("regional terrain migration is stable when repeated", memcmp(&previous, &loaded.map, sizeof(Map)) == 0);
    }
    for (int glass = 0; glass < 2; glass++) {
        game_init(&game);
        game.location = glass ? LOCATION_GLASSDEEP : LOCATION_ASHEN;
        game.level = 5;
        legacy_map(&game.map, glass);
        map_room_center(&game.map.rooms[0], &game.player.x, &game.player.y);
        game.enemy_count = 1;
        int bx;
        int by;
        map_room_center(&game.map.rooms[8], &bx, &by);
        game.enemies[0] = (Enemy){.active = 1, .is_boss = 1,
            .type = glass ? ENEMY_PRISM_SOVEREIGN : ENEMY_CINDER_LORD,
            .x = bx, .y = by, .hp = 111, .max_hp = 250, .move_timer = 2,
            .attack_phase = 1, .boss_attack = 1, .attack_target_x = bx - 2, .attack_target_y = by};
        previous = game.map;
        LevelCache *cache = glass ? &game.glassdeep_cache[4] : &game.ashen_cache[4];
        cache->valid = 1;
        cache->map = game.map;
        cache->enemy_count = 1;
        cache->enemies[0] = game.enemies[0];
        ok = save_game(&game, slot) && rewrite_version(slot, 143) && load_game(&loaded, slot);
        const Room *last = &previous.rooms[8];
        const LevelCache *restored = glass ? &loaded.glassdeep_cache[4] : &loaded.ashen_cache[4];
        int intact = ok;
        for (int ty = last->y; ty < last->y + last->h; ty++) {
            for (int tx = last->x; tx < last->x + last->w; tx++) {
                intact &= loaded.map.tiles[ty][tx] == previous.tiles[ty][tx] &&
                    restored->map.tiles[ty][tx] == previous.tiles[ty][tx];
            }
        }
        ASSERT("version 143 migration preserves active and cached boss chambers and safely cancels retired prism attacks", intact &&
            loaded.enemies[0].hp == 111 && loaded.enemies[0].move_timer == (glass ? 0 : 2) &&
            loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].attack_target_x == bx - 2 &&
            restored->enemies[0].hp == 111 && restored->enemies[0].move_timer == (glass ? 0 : 2));
    }
    remove("saves/savegame_99210.json");
}

void test_regional_terrain(void) {
    printf("Regional terrain identity tests:\n");
    test_layouts();
    test_heat();
    test_diversions();
    test_light();
    test_saves();
}
