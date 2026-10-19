#include "test_utils.h"
#include "game/wilds.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static GameState game;
static GameState loaded;
static Map maps[2];
static unsigned char seen[SWAMP_MAP_H][SWAMP_MAP_W];

static int connected(const Map *m) {
    int queue[SWAMP_MAP_H * SWAMP_MAP_W];
    int head = 0;
    int tail = 0;
    memset(seen, 0, sizeof(seen));
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
    return seen[m->stairs_down_y][m->stairs_down_x];
}

static void test_layouts(void) {
    int routes = 1;
    int anchors = 1;
    int varied = 1;
    int features = 1;
    for (int seed = 0; seed < 128; seed++) {
        for (int level = 1; level <= 7; level++) {
            srand(seed);
            map_generate_swamp(&maps[0], level);
            routes &= connected(&maps[0]);
            for (int i = 0; i < 9; i++) {
                int x, y;
                map_room_center(&maps[0].rooms[i], &x, &y);
                anchors &= seen[y][x];
            }
            int water = 0;
            int reeds = 0;
            for (int y = 0; y < SWAMP_MAP_H; y++) {
                for (int x = 0; x < SWAMP_MAP_W; x++) {
                    water += maps[0].tiles[y][x] == TILE_SWAMP_WATER;
                    reeds += maps[0].tiles[y][x] == TILE_SWAMP_REEDS;
                }
            }
            features &= water > 800 && reeds >= 35;
            if (level > 5) {
                continue;
            }
            srand(seed);
            map_generate_dragonspine(&maps[1], level);
            routes &= connected(&maps[1]);
            for (int i = 0; i < 9; i++) {
                int x, y;
                map_room_center(&maps[1].rooms[i], &x, &y);
                anchors &= seen[y][x];
            }
            int intersection = 0;
            int total = 0;
            int ravines = 0;
            int rocks = 0;
            for (int y = 0; y < SWAMP_MAP_H; y++) {
                for (int x = 0; x < SWAMP_MAP_W; x++) {
                    int a = map_is_walkable(&maps[0], x, y);
                    int b = map_is_walkable(&maps[1], x, y);
                    intersection += a && b;
                    total += a || b;
                    ravines += maps[1].tiles[y][x] == TILE_DRAGON_RAVINE;
                    rocks += maps[1].tiles[y][x] == TILE_DRAGON_BOULDER || maps[1].tiles[y][x] == TILE_DRAGON_ASH_BOULDER;
                }
            }
            varied &= intersection * 100 < total * 75;
            features &= ravines > 150 && rocks == 4;
        }
    }
    ASSERT("1536 wetland and cliff layouts keep every walking tile, clearing and exit connected", routes && anchors);
    ASSERT("Swamp and Dragonspine no longer share the same walking mask", varied);
    ASSERT("every stage includes broad blackwater and reeds or deep ravines and four movable boulders", features);
}

static void board(int swamp) {
    game_init(&game);
    game.location = swamp ? LOCATION_SWAMP : LOCATION_DRAGONSPINE;
    game.level = 1;
    game.enemy_count = 0;
    game.floor_item_count = 0;
    memset(&game.map, 0, sizeof(game.map));
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = swamp ? TILE_SWAMP_WALL : TILE_DRAGON_WALL;
        }
    }
    for (int y = 10; y <= 35; y++) {
        for (int x = 10; x <= 35; x++) {
            game.map.tiles[y][x] = swamp ? TILE_SWAMP_FLOOR : TILE_DRAGON_FLOOR;
        }
    }
    game.map.stairs_up_x = 10;
    game.map.stairs_up_y = 20;
    game.map.stairs_down_x = 35;
    game.map.stairs_down_y = 20;
    game.map.room_count = 2;
    game.map.rooms[0] = (Room){10, 10, 26, 26};
    game.map.rooms[1] = (Room){50, 50, 10, 10};
    game.player.x = 19;
    game.player.y = 20;
    game.player.max_hp = 1000;
    game.player.hp = 1000;
    game.player.defense = 0;
    if (!swamp) {
        game.map.tiles[20][20] = TILE_DRAGON_BOULDER;
    }
}

static void test_reeds(void) {
    for (int cls = 0; cls < 3; cls++) {
        board(1);
        game.player.player_class = cls;
        game.player.x = 20;
        game.map.tiles[20][20] = TILE_SWAMP_REEDS;
        game.enemy_count = 1;
        game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_BANDIT, .x = 26, .y = 20, .hp = 40, .max_hp = 40, .attack = 8};
        action_resolve_enemies(&game);
        ASSERT("every class can hide in reeds from an unhurt distant enemy", game.enemies[0].x == 26 && game.enemies[0].y == 20 && game.enemies[0].move_timer == 0);
        game.enemies[0].hp--;
        action_resolve_enemies(&game);
        ASSERT("wounded foes keep pursuing into reed cover", game.enemies[0].x < 26);
        game.enemies[0].hp = 40;
        game.enemies[0].x = 23;
        action_resolve_enemies(&game);
        ASSERT("nearby foes still notice a player in reeds", game.enemies[0].x < 23);
    }
    game.enemies[0].x = 26;
    game.enemies[0].y = 20;
    game.enemies[0].move_timer = 0;
    game.map.tiles[20][20] = TILE_ITEM;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 20, .y = 20, .underlying_tile = TILE_SWAMP_REEDS, .item = item_make_health_potion()};
    action_resolve_enemies(&game);
    ASSERT("loot does not remove reed concealment", game.enemies[0].x == 26 && game.enemies[0].move_timer == 0);
    game.floor_item_count = 0;
    game.map.tiles[20][20] = TILE_PORTAL;
    game.portal_active = 1;
    game.portal_location = LOCATION_SWAMP;
    game.portal_level = 1;
    game.portal_x = 20;
    game.portal_y = 20;
    game.portal_origin_tile = TILE_SWAMP_REEDS;
    ASSERT("portal terrain retains reed cover and bosses never lose awareness to it", wilds_conceals(&game, &game.enemies[0]));
    game.enemies[0].is_boss = 1;
    ASSERT("reed cover grants no concealment from bosses", !wilds_conceals(&game, &game.enemies[0]));
    game.enemies[0].is_boss = 0;
    game.portal_active = 0;
    game.map.tiles[20][20] = TILE_SWAMP_REEDS;
    action_resolve_player(&game, (Action){ACTION_MOVE, 20, 21});
    action_resolve_enemies(&game);
    ASSERT("stepping out of the reeds restores ordinary pursuit", game.enemies[0].x < 26 || game.enemies[0].y != 20);
}

static void test_boulders(void) {
    for (int cls = 0; cls < 3; cls++) {
        board(0);
        game.player.player_class = cls;
        game.map.tiles[20][21] = TILE_DRAGON_ASH;
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("every class can push a boulder from snow to ash with the interact action", game.map.tiles[20][20] == TILE_DRAGON_FLOOR && game.map.tiles[20][21] == TILE_DRAGON_ASH_BOULDER);
        game.player.x = 20;
        wilds_interact(&game);
        ASSERT("pushing again restores the exact ash beneath the boulder", game.map.tiles[20][21] == TILE_DRAGON_ASH && game.map.tiles[20][22] == TILE_DRAGON_BOULDER);
    }
    for (int obstacle = 0; obstacle < 6; obstacle++) {
        board(0);
        if (obstacle == 0) {
            game.map.tiles[20][21] = TILE_DRAGON_RAVINE;
        } else if (obstacle == 1) {
            game.enemy_count = 1;
            game.enemies[0] = (Enemy){.active = 1, .x = 21, .y = 20};
        } else if (obstacle == 2) {
            game.floor_item_count = 1;
            game.floor_items[0] = (FloorItem){.active = 1, .x = 21, .y = 20, .underlying_tile = TILE_DRAGON_FLOOR};
        } else if (obstacle == 3) {
            game.portal_active = 1;
            game.portal_location = LOCATION_DRAGONSPINE;
            game.portal_level = 1;
            game.portal_x = 21;
            game.portal_y = 20;
        } else if (obstacle == 4) {
            game.map.tiles[20][21] = TILE_DRAGON_TREASURE;
        } else {
            game.map.rooms[1] = (Room){21, 19, 8, 8};
        }
        maps[0] = game.map;
        wilds_interact(&game);
        ASSERT("a push cannot cover ravines, enemies, loot, portals, quest objects or the final clearing", memcmp(&maps[0], &game.map, sizeof(Map)) == 0);
    }
    board(0);
    for (int y = 10; y <= 35; y++) {
        for (int x = 10; x <= 35; x++) {
            game.map.tiles[y][x] = TILE_DRAGON_WALL;
        }
    }
    for (int y = 18; y <= 22; y++) {
        for (int x = 16; x <= 20; x++) {
            game.map.tiles[y][x] = TILE_DRAGON_FLOOR;
        }
    }
    for (int y = 20; y <= 22; y++) {
        for (int x = 22; x <= 26; x++) {
            game.map.tiles[y][x] = TILE_DRAGON_FLOOR;
        }
    }
    game.map.tiles[18][21] = TILE_DRAGON_FLOOR;
    game.map.tiles[19][21] = TILE_DRAGON_BOULDER;
    game.map.tiles[20][21] = TILE_DRAGON_FLOOR;
    game.player.x = 21;
    game.player.y = 18;
    maps[0] = game.map;
    wilds_interact(&game);
    ASSERT("a push that would isolate a walking route is rolled back completely", memcmp(&maps[0], &game.map, sizeof(Map)) == 0);

    board(0);
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_FIRE_ELEMENTAL, .x = 24, .y = 20, .hp = 40, .max_hp = 40, .attack = 12, .move_timer = 1};
    EnemyProjectiles shots = {0};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("boulders block actual Fire Elemental attacks", shots.count == 0 && game.player.hp == 1000);
    game.player.x = 20;
    game.player.y = 19;
    wilds_interact(&game);
    game.player.x = 19;
    game.player.y = 20;
    game.enemies[0].x = 24;
    game.enemies[0].y = 20;
    game.enemies[0].move_timer = 1;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("moving a boulder out of the lane opens the line of fire again", shots.count == 1 && game.player.hp < 1000);
}

static int old_version(int slot) {
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 147);
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

static void legacy(Map *m, int dragon) {
    memset(m, 0, sizeof(*m));
    TileType floor = dragon ? TILE_DRAGON_FLOOR : TILE_SWAMP_FLOOR;
    TileType wall = dragon ? TILE_DRAGON_WALL : TILE_SWAMP_WALL;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = wall;
        }
    }
    m->room_count = 9;
    for (int i = 0; i < 9; i++) {
        Room *room = &m->rooms[i];
        *room = (Room){5 + i % 3 * 22, 5 + i / 3 * 20, 14, 10};
        for (int y = room->y; y < room->y + room->h; y++) {
            for (int x = room->x; x < room->x + room->w; x++) {
                m->tiles[y][x] = floor;
            }
        }
    }
    for (int i = 1; i < 9; i++) {
        int x, y, tx, ty;
        map_room_center(&m->rooms[i - 1], &x, &y);
        map_room_center(&m->rooms[i], &tx, &ty);
        while (x != tx || y != ty) {
            m->tiles[y][x] = floor;
            if (x != tx) {
                x += x < tx ? 1 : -1;
            } else {
                y += y < ty ? 1 : -1;
            }
        }
    }
    map_room_center(&m->rooms[0], &m->stairs_up_x, &m->stairs_up_y);
    map_room_center(&m->rooms[8], &m->stairs_down_x, &m->stairs_down_y);
}

static void test_saves(void) {
    const int slot = 99214;
    if (save_exists(slot)) {
        ASSERT("wild terrain save slot is unused", 0);
        return;
    }
    board(0);
    game.map.tiles[20][21] = TILE_DRAGON_ASH;
    wilds_interact(&game);
    game.dragonspine_cache[0].valid = 1;
    game.dragonspine_cache[0].map = game.map;
    int ok = save_game(&game, slot) && load_game(&loaded, slot);
    ASSERT("pushed boulders and their snow/ash underlays persist in active and cached maps", ok && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0 && memcmp(&game.dragonspine_cache[0].map, &loaded.dragonspine_cache[0].map, sizeof(Map)) == 0);
    for (int dragon = 0; dragon < 2; dragon++) {
        game_init(&game);
        game.location = dragon ? LOCATION_DRAGONSPINE : LOCATION_SWAMP;
        game.level = dragon ? 5 : 4;
        legacy(&game.map, dragon);
        map_room_center(&game.map.rooms[0], &game.player.x, &game.player.y);
        game.gold = 432;
        game.innkeeper_quest_state = 3;
        game.dragon_treasure_quest_state = 3;
        game.defeated_bosses |= 1 << LOCATION_DUNGEON;
        int bx, by;
        map_room_center(&game.map.rooms[8], &bx, &by);
        game.enemy_count = 2;
        game.enemies[0] = (Enemy){.active = 1, .is_boss = 1, .type = dragon ? ENEMY_RED_DRAGON : ENEMY_SWAMP_DEMON, .x = bx, .y = by, .hp = 111, .max_hp = 250, .move_timer = 2, .attack_phase = 1, .boss_attack = 1, .attack_target_x = bx - 2, .attack_target_y = by};
        game.enemies[1] = (Enemy){.type = ENEMY_GIANT_RAT, .active = 0, .hp = 0};
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = game.player.x, .y = game.player.y, .underlying_tile = game.map.tiles[game.player.y][game.player.x], .item = item_make_health_potion()};
        game.map.tiles[game.player.y][game.player.x] = TILE_ITEM;
        game.portal_active = 1;
        game.portal_location = game.location;
        game.portal_level = game.level;
        game.portal_x = game.player.x;
        game.portal_y = game.player.y;
        game.portal_origin_tile = game.floor_items[0].underlying_tile;
        map_mark_explored(&game.map, game.player.x, game.player.y);
        LevelCache *cache = dragon ? &game.dragonspine_cache[4] : &game.swamp_cache[3];
        cache->valid = 1;
        cache->map = game.map;
        cache->enemy_count = game.enemy_count;
        memcpy(cache->enemies, game.enemies, sizeof(cache->enemies));
        maps[0] = game.map;
        ok = save_game(&game, slot) && old_version(slot) && load_game(&loaded, slot);
        const LevelCache *restored = dragon ? &loaded.dragonspine_cache[4] : &loaded.swamp_cache[3];
        const Room *last = &maps[0].rooms[8];
        int preserved = ok;
        int scenery = 0;
        for (int y = 0; y < SWAMP_MAP_H; y++) {
            for (int x = 0; x < SWAMP_MAP_W; x++) {
                if (map_is_walkable(&maps[0], x, y)) {
                    preserved &= map_is_walkable(&loaded.map, x, y);
                }
                if (x >= last->x && x < last->x + last->w && y >= last->y && y < last->y + last->h) {
                    preserved &= loaded.map.tiles[y][x] == maps[0].tiles[y][x] && restored->map.tiles[y][x] == maps[0].tiles[y][x];
                }
                scenery += wilds_tile(loaded.map.tiles[y][x]);
            }
        }
        ASSERT("version 147 maps gain new terrain while preserving old routes and active/cached boss chambers", preserved && scenery > 0 && connected(&loaded.map));
        ASSERT("migration preserves boss warnings, wounds, deaths, loot, portals, quests and exploration", ok && loaded.enemies[0].hp == 111 && loaded.enemies[0].move_timer == 2 && loaded.enemies[0].attack_phase == 1 && loaded.enemies[0].attack_target_x == bx - 2 && !loaded.enemies[1].active && loaded.gold == 432 && loaded.floor_items[0].active && loaded.portal_origin_tile == game.portal_origin_tile && loaded.innkeeper_quest_state == 3 && loaded.dragon_treasure_quest_state == 3 && loaded.defeated_bosses == game.defeated_bosses && map_is_explored(&loaded.map, game.player.x, game.player.y));
        maps[1] = loaded.map;
        wilds_migrate(&loaded);
        ASSERT("wetland and cliff migration is stable when repeated", memcmp(&maps[1], &loaded.map, sizeof(Map)) == 0);
    }
    remove("saves/savegame_99214.json");
}

void test_wilds(void) {
    printf("Swamp islands and Dragonspine cliff tests:\n");
    test_layouts();
    test_reeds();
    test_boulders();
    test_saves();
}
