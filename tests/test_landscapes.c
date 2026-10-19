#include "test_utils.h"
#include "game/landscapes.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;
static Map maps[3];
static unsigned char seen[MAP_H][MAP_W];
static const Location locations[3] = {LOCATION_FROSTFELL, LOCATION_DESERT, LOCATION_MOONVEIL};

static void generate(Map *m, int region, int level) {
    if (region == 0) {
        map_generate_frostfell(m, level);
    } else if (region == 1) {
        map_generate_desert(m, level);
    } else {
        map_generate_moonveil(m, level);
    }
}

static int find_tile(const Map *m, TileType tile, int skip_x, int skip_y, int *x, int *y) {
    for (int ty = 0; ty < SWAMP_MAP_H; ty++) {
        for (int tx = 0; tx < SWAMP_MAP_W; tx++) {
            if ((tx != skip_x || ty != skip_y) && m->tiles[ty][tx] == tile) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static int connected(const Map *m, int safe) {
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
                (safe && (m->tiles[y][x] == TILE_FROST_ICE || m->tiles[y][x] == TILE_FROST_THIN_ICE))) {
                continue;
            }
            seen[y][x] = 1;
            queue[tail++] = y * MAP_W + x;
        }
    }
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            TileType tile = m->tiles[y][x];
            if (safe && (tile == TILE_FROST_ICE || tile == TILE_FROST_THIN_ICE)) {
                continue;
            }
            if (map_is_walkable(m, x, y) && !seen[y][x]) {
                printf("  Disconnected landscape tile: %d at %d,%d\n", tile, x, y);
                return 0;
            }
        }
    }
    return seen[m->stairs_down_y][m->stairs_down_x];
}

static void test_layouts(void) {
    int paths = 1;
    int ice_optional = 1;
    int distinct = 1;
    int features = 1;
    for (int seed = 0; seed < 128; seed++) {
        for (int level = 1; level <= 5; level++) {
            for (int region = 0; region < 3; region++) {
                srand(seed);
                generate(&maps[region], region, level);
                paths &= connected(&maps[region], 0);
            }
            ice_optional &= connected(&maps[0], 1);
            for (int a = 0; a < 3; a++) {
                for (int b = a + 1; b < 3; b++) {
                    int intersection = 0;
                    int total = 0;
                    for (int y = 0; y < SWAMP_MAP_H; y++) {
                        for (int x = 0; x < SWAMP_MAP_W; x++) {
                            int aa = map_is_walkable(&maps[a], x, y);
                            int bb = map_is_walkable(&maps[b], x, y);
                            intersection += aa && bb;
                            total += aa || bb;
                        }
                    }
                    distinct &= intersection * 100 < total * 75;
                }
            }
            int sand = 0;
            int mesas = 0;
            int wells = 0;
            int water = 0;
            int stones[2] = {0};
            for (int y = 0; y < SWAMP_MAP_H; y++) {
                for (int x = 0; x < SWAMP_MAP_W; x++) {
                    sand += maps[1].tiles[y][x] == TILE_DESERT_FLOOR;
                    wells += maps[1].tiles[y][x] == TILE_DESERT_OASIS;
                    mesas += maps[1].tiles[y][x] == TILE_DESERT_MESA;
                    water += maps[2].tiles[y][x] == TILE_MOONVEIL_POOL;
                    stones[0] += maps[2].tiles[y][x] == TILE_MOONVEIL_WAYSTONE_A;
                    stones[1] += maps[2].tiles[y][x] == TILE_MOONVEIL_WAYSTONE_B;
                }
            }
            features &= sand > 1400 && mesas > 450 && wells == 1 && water > 200 && stones[0] == 2 && stones[1] == 2;
        }
    }
    ASSERT("1920 independent glacier, dune and garden layouts keep every walking tile reachable", paths);
    ASSERT("all glacier exits and snow routes work without slick or brittle ice", ice_optional);
    ASSERT("Frostfell, Sunscar and Moonveil no longer share near-identical walking masks", distinct);
    ASSERT("each stage has navigable dunes with substantial mesas, one well, a garden pool and complete waystones", features);
}

static void setup(int region) {
    game_init(&game);
    game.location = locations[region];
    game.level = 1;
    generate(&game.map, region, 1);
    game.enemy_count = 0;
    game.floor_item_count = 0;
    game.player.max_hp = 100;
    game.player.hp = 20;
    game.player.max_mp = 40;
    game.player.mp = 4;
}

static void test_wells(void) {
    int x, y;
    for (int cls = 0; cls < 3; cls++) {
        setup(1);
        game.player.player_class = cls;
        find_tile(&game.map, TILE_DESERT_OASIS, -1, -1, &x, &y);
        game.player.x = x + 1;
        game.player.y = y;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("every class can drink a quarter-health and mana refill from an adjacent well", game_has_regional_interaction(&game) && game.player.hp == 45 && game.player.mp == 14 && game.map.tiles[y][x] == TILE_DESERT_OASIS_DRY);
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the same well cannot be farmed repeatedly in one expedition", game.player.hp == 45 && game.player.mp == 14);
    }
    setup(1);
    find_tile(&game.map, TILE_DESERT_OASIS, -1, -1, &x, &y);
    game.player.x = x;
    game.player.y = y;
    game.player.hp = 100;
    game.player.mp = 40;
    landscape_interact(&game);
    ASSERT("full health and mana do not waste the well's water", game.map.tiles[y][x] == TILE_DESERT_OASIS);
    game.player.hp = 99;
    game.player.mp = 39;
    landscape_interact(&game);
    ASSERT("oasis recovery respects health and mana caps", game.player.hp == 100 && game.player.mp == 40);
    game_descend(&game);
    game_ascend(&game);
    ASSERT("backtracking keeps the well dry", game.map.tiles[y][x] == TILE_DESERT_OASIS_DRY);
    game_return_to_town(&game);
    game_enter_desert(&game);
    ASSERT("a new expedition from town refills previously used wells", game.map.tiles[y][x] == TILE_DESERT_OASIS);

    game.player.x = x;
    game.player.y = y;
    game.player.hp = 20;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_DESERT_OASIS, .item = item_make_health_potion()};
    game.map.tiles[y][x] = TILE_ITEM;
    landscape_interact(&game);
    ASSERT("drinking beneath loot preserves the loot and updates its saved terrain", game.map.tiles[y][x] == TILE_ITEM && game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_DESERT_OASIS_DRY);
}

static void test_ice_replay(void) {
    game_init(&game);
    game_enter_frostfell(&game);
    game_descend(&game);
    game_descend(&game);
    game.enemy_count = 0;
    int x, y;
    if (!find_tile(&game.map, TILE_FROST_THIN_ICE, -1, -1, &x, &y)) {
        ASSERT("stage three provides a brittle crossing for replay", 0);
        return;
    }
    game.player.x = x;
    game.player.y = y - 1;
    int end_y = y;
    while (game.map.tiles[end_y][x] == TILE_FROST_THIN_ICE) {
        action_resolve_player(&game, (Action){ACTION_MOVE, x, end_y});
        end_y++;
    }
    action_resolve_player(&game, (Action){ACTION_MOVE, x, end_y});
    ASSERT("walking across brittle ice collapses the crossing for the rest of the expedition", game.player.y == end_y && game.map.tiles[y][x] == TILE_FROST_BROKEN_ICE);
    game_return_to_town(&game);
    game_enter_frostfell(&game);
    game_descend(&game);
    game_descend(&game);
    ASSERT("a real new expedition refreezes the cached crossing so it can be replayed", game.level == 3 && game.map.tiles[y][x] == TILE_FROST_THIN_ICE);
}

static void test_waystones(void) {
    int x, y, tx, ty;
    for (int cls = 0; cls < 3; cls++) {
        for (int pair = 0; pair < 2; pair++) {
            setup(2);
            game.player.player_class = cls;
            TileType tile = pair ? TILE_MOONVEIL_WAYSTONE_B : TILE_MOONVEIL_WAYSTONE_A;
            find_tile(&game.map, tile, -1, -1, &x, &y);
            find_tile(&game.map, tile, x, y, &tx, &ty);
            game.player.x = x + 1;
            game.player.y = y;
            action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
            ASSERT("every class can cross between matching waystones without spending mana", game.player.x == tx && game.player.y == ty && game.player.mp == 4 && map_is_explored(&game.map, tx, ty));
            action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
            ASSERT("both ends of each waystone pair work for the return trip", game.player.x == x && game.player.y == y);
        }
    }
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_GIANT_MOTH, .x = tx, .y = ty, .hp = 12};
    landscape_interact(&game);
    ASSERT("waystones never place the player on a living enemy", game.player.x == x && game.player.y == y && game.enemies[0].hp == 12);
    game.enemies[0].active = 0;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = tx, .y = ty, .underlying_tile = TILE_MOONVEIL_WAYSTONE_B, .item = item_make_health_potion()};
    game.map.tiles[ty][tx] = TILE_ITEM;
    landscape_interact(&game);
    ASSERT("waystone links survive loot covering their destination", game.player.x == tx && game.player.y == ty && game.floor_items[0].active && game.map.tiles[ty][tx] == TILE_ITEM);
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

static void legacy(Map *m, int region) {
    map_generate_swamp(m, 5);
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W / 2; x++) {
            TileType tmp = m->tiles[y][x];
            m->tiles[y][x] = m->tiles[y][SWAMP_MAP_W - x - 1];
            m->tiles[y][SWAMP_MAP_W - x - 1] = tmp;
        }
    }
    for (int i = 0; i < m->room_count; i++) {
        m->rooms[i].x = SWAMP_MAP_W - m->rooms[i].x - m->rooms[i].w;
    }
    m->stairs_up_x = SWAMP_MAP_W - m->stairs_up_x - 1;
    m->stairs_down_x = SWAMP_MAP_W - m->stairs_down_x - 1;
    const TileType floors[3] = {TILE_FROST_FLOOR, TILE_DESERT_FLOOR, TILE_MOONVEIL_FLOOR};
    const TileType walls[3] = {TILE_FROST_WALL, TILE_DESERT_WALL, TILE_MOONVEIL_WALL};
    const TileType entries[3] = {TILE_FROST_ENTRANCE, TILE_DESERT_ENTRANCE, TILE_MOONVEIL_ENTRANCE};
    const TileType exits[3] = {TILE_FROST_EXIT, TILE_DESERT_EXIT, TILE_MOONVEIL_EXIT};
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            TileType old = m->tiles[y][x];
            m->tiles[y][x] = old == TILE_SWAMP_FLOOR ? floors[region] : old == TILE_SWAMP_ENTRANCE ? entries[region] : old == TILE_SWAMP_EXIT ? exits[region] : walls[region];
        }
    }
}

static void test_saves(void) {
    const int slot = 99211;
    if (save_exists(slot)) {
        ASSERT("landscape test save slot is unused", 0);
        return;
    }
    for (int region = 0; region < 3; region++) {
        setup(region);
        if (region == 1) {
            find_tile(&game.map, TILE_DESERT_OASIS, -1, -1, &game.player.x, &game.player.y);
            landscape_interact(&game);
        }
        LevelCache *cache = region == 0 ? game.frostfell_cache : region == 1 ? game.desert_cache : game.moonveil_cache;
        cache[0].valid = 1;
        cache[0].map = game.map;
        int ok = save_game(&game, slot) && load_game(&loaded, slot);
        const LevelCache *restored = region == 0 ? loaded.frostfell_cache : region == 1 ? loaded.desert_cache : loaded.moonveil_cache;
        ASSERT("new terrain and consumed wells round trip in active and cached maps", ok && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0 && memcmp(&cache[0].map, &restored[0].map, sizeof(Map)) == 0);

        game_init(&game);
        game.location = locations[region];
        game.level = 5;
        legacy(&game.map, region);
        map_room_center(&game.map.rooms[0], &game.player.x, &game.player.y);
        game.gold = 432;
        game.frostfell_quest_state = 3;
        game.frostfell_quest_progress = 3;
        game.moonveil_quest_state = 3;
        game.moonveil_quest_progress = 7;
        game.sunscar_lamp_quest_state = 3;
        game.defeated_bosses |= 1 << LOCATION_DUNGEON;
        game.enemy_count = 2;
        int bx, by;
        map_room_center(&game.map.rooms[8], &bx, &by);
        game.enemies[0] = (Enemy){.active = 1, .is_boss = 1, .type = region == 0 ? ENEMY_POLAR_KRAKEN : region == 1 ? ENEMY_DESERT_PHARAOH : ENEMY_THORN_REGENT, .x = bx, .y = by, .hp = 111, .max_hp = 250, .attack_phase = 1, .move_timer = 2, .attack_target_x = bx - 2, .attack_target_y = by};
        game.enemies[1] = (Enemy){.active = 0, .hp = 0, .type = ENEMY_SKELETON};
        game.portal_active = 1;
        game.portal_location = game.location;
        game.portal_level = 5;
        game.portal_x = game.player.x;
        game.portal_y = game.player.y;
        game.portal_origin_tile = game.map.tiles[game.player.y][game.player.x];
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = game.player.x, .y = game.player.y, .underlying_tile = game.portal_origin_tile, .item = item_make_health_potion()};
        game.map.tiles[game.player.y][game.player.x] = TILE_ITEM;
        map_mark_explored(&game.map, game.player.x, game.player.y);
        cache = region == 0 ? game.frostfell_cache : region == 1 ? game.desert_cache : game.moonveil_cache;
        cache[4].valid = 1;
        cache[4].map = game.map;
        cache[4].enemy_count = game.enemy_count;
        memcpy(cache[4].enemies, game.enemies, sizeof(cache[4].enemies));
        maps[0] = game.map;
        ok = save_game(&game, slot) && rewrite_version(slot, 145) && load_game(&loaded, slot);
        int preserved = ok;
        const Room *last = &maps[0].rooms[8];
        restored = region == 0 ? loaded.frostfell_cache : region == 1 ? loaded.desert_cache : loaded.moonveil_cache;
        for (int y = 0; y < SWAMP_MAP_H; y++) {
            for (int x = 0; x < SWAMP_MAP_W; x++) {
                if (region != 1 && map_is_walkable(&maps[0], x, y)) {
                    preserved &= map_is_walkable(&loaded.map, x, y);
                }
                if (x >= last->x && x < last->x + last->w && y >= last->y && y < last->y + last->h) {
                    preserved &= loaded.map.tiles[y][x] == maps[0].tiles[y][x] && restored[4].map.tiles[y][x] == maps[0].tiles[y][x];
                }
            }
        }
        ASSERT("old maps retain active/cached boss chambers and routes around added desert cover", preserved && connected(&loaded.map, 0));
        ASSERT("migration preserves quests, wounds, deaths, pending attacks, loot and portals", ok && loaded.gold == 432 && loaded.player.x == game.player.x && loaded.player.y == game.player.y && loaded.enemies[0].hp == 111 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].attack_target_x == bx - 2 && !loaded.enemies[1].active && loaded.floor_items[0].active && loaded.portal_origin_tile == game.portal_origin_tile && loaded.frostfell_quest_progress == 3 && loaded.moonveil_quest_progress == 7 && loaded.sunscar_lamp_quest_state == 3 && loaded.defeated_bosses == game.defeated_bosses && map_is_explored(&loaded.map, game.player.x, game.player.y));
        maps[1] = loaded.map;
        landscape_migrate(&loaded);
        ASSERT("landscape migration is stable when repeated", memcmp(&maps[1], &loaded.map, sizeof(Map)) == 0);
    }
    remove("saves/savegame_99211.json");
}

static void test_sunscar_patrol_replay(void) {
    const int slot = 99212;
    if (save_exists(slot)) {
        ASSERT("Sunscar patrol test save slot is unused", 0);
        return;
    }
    setup(1);
    game.level = 5;
    game.defeated_bosses |= 1 << LOCATION_DESERT;
    map_generate_desert(&game.map, 5);
    enemies_spawn(&game);
    game.enemy_count = 14;
    game.enemies[0].hp = 0;
    game.enemies[0].active = 0;
    snprintf(game.enemies[12].name, sizeof(game.enemies[12].name), "Dunehex Mummy");
    game.enemies[12].hp = 9;
    game.enemies[13] = (Enemy){.type = ENEMY_DESERT_PHARAOH, .is_boss = 1, .active = 0, .hp = 0};
    game_return_to_town(&game);
    game_enter_desert(&game);
    while (game.level < 5) {
        game_descend(&game);
    }
    int regular = 0;
    int bosses = 0;
    int distinct = 1;
    int guard_hp = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        const Enemy *enemy = &game.enemies[i];
        if (strcmp(enemy->name, "Dunehex Mummy") == 0) {
            guard_hp = enemy->hp;
        } else if (enemy->active && !enemy->is_boss) {
            regular++;
        }
        bosses += enemy->active && enemy->is_boss;
        for (int j = 0; j < i && enemy->active; j++) {
            distinct &= !game.enemies[j].active || enemy->x != game.enemies[j].x || enemy->y != game.enemies[j].y;
        }
    }
    ASSERT("a new expedition fills sparse cached Sunscar stages with the larger regular patrols", regular == 38 && distinct && game.desert_cache[4].enemy_count == game.enemy_count);
    ASSERT("larger patrols preserve named quest wounds and never resurrect the defeated Pharaoh", guard_hp == 9 && bosses == 0 && (game.defeated_bosses & (1 << LOCATION_DESERT)));
    int ok = save_game(&game, slot) && load_game(&loaded, slot);
    ASSERT("larger Sunscar rosters round trip beyond the former thirty-enemy cache capacity", ok && loaded.enemy_count == game.enemy_count && loaded.desert_cache[4].enemy_count == game.desert_cache[4].enemy_count && loaded.enemies[12].hp == 9 && !loaded.enemies[13].active);
    remove("saves/savegame_99212.json");
}

static void test_sunscar_cover_migration(void) {
    const int slot = 99213;
    if (save_exists(slot)) {
        ASSERT("Sunscar cover migration save slot is unused", 0);
        return;
    }
    setup(1);
    game.level = 3;
    map_generate_desert(&game.map, 3);
    int cells[5];
    int found = 0;
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            if (game.map.tiles[y][x] == TILE_DESERT_MESA) {
                if (found < 5) {
                    cells[found++] = y * SWAMP_MAP_W + x;
                }
                game.map.tiles[y][x] = TILE_DESERT_FLOOR;
            }
        }
    }
    if (found < 5) {
        ASSERT("Sunscar has terrain candidates for safe cover migration", 0);
        return;
    }
    game.player.x = cells[0] % SWAMP_MAP_W;
    game.player.y = cells[0] / SWAMP_MAP_W;
    game.enemy_count = 2;
    game.enemies[0] = (Enemy){.type = ENEMY_SCARAB, .active = 1, .hp = 9, .x = cells[1] % SWAMP_MAP_W, .y = cells[1] / SWAMP_MAP_W};
    game.enemies[1] = (Enemy){.type = ENEMY_VIPER, .active = 0, .hp = 0, .x = cells[4] % SWAMP_MAP_W, .y = cells[4] / SWAMP_MAP_W};
    int lx = cells[2] % SWAMP_MAP_W;
    int ly = cells[2] / SWAMP_MAP_W;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = lx, .y = ly, .underlying_tile = TILE_DESERT_FLOOR, .item = item_make_health_potion()};
    game.map.tiles[ly][lx] = TILE_ITEM;
    game.portal_active = 1;
    game.portal_location = LOCATION_DESERT;
    game.portal_level = 3;
    game.portal_x = cells[3] % SWAMP_MAP_W;
    game.portal_y = cells[3] / SWAMP_MAP_W;
    game.portal_origin_tile = TILE_DESERT_FLOOR;
    game.map.tiles[game.portal_y][game.portal_x] = TILE_PORTAL;
    map_mark_explored(&game.map, game.player.x, game.player.y);
    game.desert_cache[2].valid = 1;
    game.desert_cache[2].map = game.map;
    game.desert_cache[2].enemy_count = game.enemy_count;
    memcpy(game.desert_cache[2].enemies, game.enemies, sizeof(game.desert_cache[2].enemies));
    maps[0] = game.map;
    int ok = save_game(&game, slot) && rewrite_version(slot, 146) && load_game(&loaded, slot);
    int mesas = 0;
    for (int y = 0; y < SWAMP_MAP_H; y++) {
        for (int x = 0; x < SWAMP_MAP_W; x++) {
            mesas += loaded.map.tiles[y][x] == TILE_DESERT_MESA;
        }
    }
    ASSERT("version 146 open basins gain substantial connected cover in place", ok && mesas > 450 && connected(&loaded.map, 0) && connected(&loaded.desert_cache[2].map, 0));
    ASSERT("cover migration leaves actors, corpses, loot and portal landings uncovered", ok && loaded.map.tiles[game.player.y][game.player.x] == TILE_DESERT_FLOOR && loaded.map.tiles[game.enemies[0].y][game.enemies[0].x] == TILE_DESERT_FLOOR && loaded.map.tiles[game.enemies[1].y][game.enemies[1].x] == TILE_DESERT_FLOOR && loaded.map.tiles[ly][lx] == TILE_ITEM && loaded.floor_items[0].active && loaded.map.tiles[game.portal_y][game.portal_x] == TILE_DESERT_FLOOR && loaded.portal_active && loaded.portal_x == game.portal_x && loaded.portal_y == game.portal_y && loaded.portal_origin_tile == TILE_DESERT_FLOOR);
    ASSERT("loading cover preserves current expedition deaths, wounds and exploration", ok && loaded.enemy_count == 2 && loaded.enemies[0].hp == 9 && !loaded.enemies[1].active && memcmp(game.map.explored, loaded.map.explored, sizeof(game.map.explored)) == 0);
    maps[1] = loaded.map;
    sunscar_migrate(&loaded);
    ASSERT("cover migration is stable when repeated", memcmp(&maps[1], &loaded.map, sizeof(Map)) == 0);
    remove("saves/savegame_99213.json");
}

void test_landscapes(void) {
    printf("Distinct glacier, dune and garden tests:\n");
    test_layouts();
    test_wells();
    test_ice_replay();
    test_waystones();
    test_saves();
    test_sunscar_patrol_replay();
    test_sunscar_cover_migration();
}
