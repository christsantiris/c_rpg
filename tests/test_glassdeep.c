#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define GLASSDEEP_TEST_SLOT 99132
static GameState game;
static GameState loaded;
static Map layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static void walk_edge(int south) {
    int x = south ? game.map.stairs_down_x : game.map.stairs_up_x;
    int y = south ? game.map.stairs_down_y : game.map.stairs_up_y;
    game.player.x = x;
    game.player.y = south ? y - 1 : y + 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
}

static void test_layouts(void) {
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    for (int level = 1; level <= GLASSDEEP_DEPTH; level++) {
        int connected = 1;
        int roster = 1;
        int landmarks = 1;
        for (int seed = 0; seed < 12; seed++) {
            srand(2810 + seed);
            map_generate_glassdeep(&game.map, level);
            game.location = LOCATION_GLASSDEEP;
            game.level = level;
            memset(visited, 0, sizeof(visited));
            int head = 0;
            int tail = 0;
            queue[tail++] = game.map.stairs_up_y * MAP_W + game.map.stairs_up_x;
            visited[game.map.stairs_up_y][game.map.stairs_up_x] = 1;
            const int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            while (head < tail) {
                int cell = queue[head++];
                for (int i = 0; i < 4; i++) {
                    int x = cell % MAP_W + directions[i][0];
                    int y = cell / MAP_W + directions[i][1];
                    if (!map_is_walkable(&game.map, x, y) || visited[y][x]) {
                        continue;
                    }
                    visited[y][x] = 1;
                    queue[tail++] = y * MAP_W + x;
                }
            }
            connected &= visited[game.map.stairs_down_y][game.map.stairs_down_x];
            int pools = 0;
            int stones = 0;
            for (int y = 0; y < SWAMP_MAP_H; y++) {
                for (int x = 0; x < SWAMP_MAP_W; x++) {
                    TileType tile = game.map.tiles[y][x];
                    if (tile == TILE_GLASSDEEP_POOL) {
                        pools++;
                        landmarks &= !map_is_walkable(&game.map, x, y);
                    } else if (tile == TILE_GLASSDEEP_RUIN) {
                        stones++;
                        landmarks &= map_is_walkable(&game.map, x, y);
                    }
                }
            }
            landmarks &= pools > 0 && stones > 0 && game.map.stairs_up_y == 0 && game.map.stairs_down_y == SWAMP_MAP_H - 1;
            for (int i = 0; i < game.map.room_count; i++) {
                int x;
                int y;
                map_room_center(&game.map.rooms[i], &x, &y);
                connected &= visited[y][x];
            }
            enemies_spawn(&game);
            int bosses = 0;
            for (int i = 0; i < game.enemy_count; i++) {
                Enemy *enemy = &game.enemies[i];
                roster &= enemy->type >= ENEMY_CRYSTAL_SPIDER && enemy->type <= ENEMY_PRISM_SOVEREIGN && visited[enemy->y][enemy->x];
                bosses += enemy->is_boss;
            }
            roster &= bosses == (level == GLASSDEEP_DEPTH);
        }
        ASSERT("crystal entrances, exits, and every clearing remain connected across seeds", connected);
        ASSERT("pools block movement and ruined paving remain walkable", landmarks);
        ASSERT("each stage spawns its own reachable roster and only stage five has a boss", roster);
    }
}

static void test_travel_and_saves(void) {
    game_init(&game);
    srand(4612);
    game_enter_town2(&game);
    game.player.x = STILLBURY_GLASSDEEP_GATE_X;
    game.player.y = TOWN_H - 2;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, TOWN_H - 1});
    ASSERT("Stillbury's south gate enters Glassdeep Caverns stage one from the north", game.location == LOCATION_GLASSDEEP && game.level == 1 && game.player.y == 0);
    int x;
    int y;
    map_room_center(&game.map.rooms[0], &x, &y);
    map_mark_explored(&game.map, x, y);
    game.enemies[0].hp = 7;
    layout = game.map;
    walk_edge(1);
    ASSERT("the south exit advances with regular enemies still alive", game.level == 2 && game.max_glassdeep_level_reached == 2 && game.glassdeep_cache[0].valid);
    walk_edge(0);
    ASSERT("backtracking preserves maps, exploration, and enemy health", game.level == 1 && memcmp(&layout, &game.map, sizeof(layout)) == 0 && game.enemies[0].hp == 7);
    walk_edge(0);
    ASSERT("the north exit returns at Stillbury's south gate", game.location == LOCATION_TOWN2 && game.player.x == STILLBURY_GLASSDEEP_GATE_X && game.player.y == TOWN_H - 2);
    game_enter_glassdeep(&game);
    ASSERT("re-entering Glassdeep Caverns retains visited stages and progress", game.enemies[0].hp == 7 && game.max_glassdeep_level_reached == 2 && memcmp(&layout, &game.map, sizeof(layout)) == 0);
    while (game.level < 3) {
        walk_edge(1);
    }
    ASSERT("saving and loading Glassdeep Caverns preserves independent stage caches", save_game(&game, GLASSDEEP_TEST_SLOT) && load_game(&loaded, GLASSDEEP_TEST_SLOT) && loaded.location == LOCATION_GLASSDEEP && loaded.level == 3 && loaded.max_glassdeep_level_reached == 3 && loaded.glassdeep_cache[0].enemies[0].hp == 7 && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0);
    map_room_center(&game.map.rooms[1], &x, &y);
    game.player.x = x;
    game.player.y = y;
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].x == x && game.enemies[i].y == y) {
            game.enemies[i].active = 0;
        }
    }
    game.map.tiles[y][x] = TILE_ITEM;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_GLASSDEEP_RUIN, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game_open_town_portal(&game);
    ASSERT("Return to Town places a crystal portal beside Stillbury's south gate", game.location == LOCATION_TOWN2 && game.portal_location == LOCATION_GLASSDEEP && game.map.tiles[TOWN_H - 3][STILLBURY_GLASSDEEP_GATE_X + 1] == TILE_PORTAL && game.portal_origin_tile == TILE_GLASSDEEP_RUIN);
    game.player.x = STILLBURY_GLASSDEEP_GATE_X + 1;
    game.player.y = TOWN_H - 4;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, TOWN_H - 3});
    ASSERT("stepping onto Stillbury's portal resumes the original Glassdeep stage and position", game.location == LOCATION_GLASSDEEP && game.level == 3 && game.player.x == x && game.player.y == y);
    if (game.location != LOCATION_GLASSDEEP) {
        remove("saves/savegame_99132.json");
        return;
    }
    game_open_town_portal(&game);
    int saved = save_game(&game, GLASSDEEP_TEST_SLOT) && load_game(&loaded, GLASSDEEP_TEST_SLOT);
    if (saved) {
        game = loaded;
        game.player.x = STILLBURY_GLASSDEEP_GATE_X + 1;
        game.player.y = TOWN_H - 4;
        action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, TOWN_H - 3});
    }
    ASSERT("stepping onto a saved return portal restores the crystal stage and underlying paving", saved && game.location == LOCATION_GLASSDEEP && game.level == 3 && game.player.x == x && game.player.y == y && game.map.tiles[y][x] == TILE_GLASSDEEP_RUIN);
    if (game.location != LOCATION_GLASSDEEP) {
        remove("saves/savegame_99132.json");
        return;
    }
    while (game.level < GLASSDEEP_DEPTH) {
        walk_edge(1);
    }
    walk_edge(1);
    ASSERT("the living Prism Sovereign blocks the final return exit", game.location == LOCATION_GLASSDEEP && game.level == GLASSDEEP_DEPTH);
    game.portal_level = GLASSDEEP_DEPTH + 1;
    game.portal_location = LOCATION_GLASSDEEP;
    game.location = LOCATION_TOWN2;
    game_use_town_portal(&game);
    ASSERT("out-of-range crystal portals cannot index past the cache", game.location == LOCATION_TOWN2);
    remove("saves/savegame_99132.json");
}

static void test_boss_victories(void) {
    for (int attack = 0; attack < 4; attack++) {
        game_init(&game);
        srand(6100 + attack);
        game.location = LOCATION_GLASSDEEP;
        game.level = GLASSDEEP_DEPTH;
        map_generate_glassdeep(&game.map, game.level);
        enemies_spawn(&game);
        Enemy *boss = &game.enemies[0];
        boss->hp = 1;
        game.player.x = boss->x - (attack == 0 ? 1 : 2);
        game.player.y = boss->y;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.mp = 100;
        game.inventory[0] = attack == 3 ? item_make_bow() : item_make_staff();
        game.inventory_count = 1;
        game.equipped_main_hand = 0;
        game.player.known_spells[0] = attack == 2 ? spell_make_fireball() : spell_make_magic_arrow();
        game.player.known_spell_count = 1;
        game.player.equipped_spell = 0;
        Action action = attack == 0 ? (Action){ACTION_MOVE, boss->x, boss->y} :
            (Action){attack == 3 ? ACTION_RANGED_ATTACK : ACTION_CAST_SPELL, 0, 0};
        action_resolve_player(&game, action);
        int rewards = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            rewards += game.floor_items[i].active && game.floor_items[i].item.type == ITEM_POTION_STRENGTH;
        }
        BossJournalEntry entry;
        ASSERT("melee, Magic Arrow, Fireball, and bow victories grant the Prism Sovereign reward and journal completion", !boss->active && (game.defeated_bosses & (1 << LOCATION_GLASSDEEP)) && rewards == 1 && quest_journal_get_boss(&game, 11, &entry) && entry.defeated && strcmp(entry.name, "Prism Sovereign") == 0);
        walk_edge(1);
        ASSERT("defeating the Prism Sovereign allows return while other enemies remain", game.location == LOCATION_TOWN2 && game.glassdeep_cache[GLASSDEEP_DEPTH - 1].enemy_count > 1);
        game_enter_glassdeep(&game);
        while (game.level < GLASSDEEP_DEPTH) {
            walk_edge(1);
        }
        int alive = 0;
        for (int i = 0; i < game.enemy_count; i++) {
            alive += game.enemies[i].active && game.enemies[i].type == ENEMY_PRISM_SOVEREIGN;
        }
        ASSERT("the defeated Prism Sovereign remains defeated on later visits", alive == 0);
    }
}

static void test_migration(void) {
    game_init(&game);
    game_enter_town2(&game);
    game.player.hp = 89;
    game.gold = 37;
    game.score = 1234;
    game.sunscar_lamp_quest_state = 1;
    game.defeated_bosses = 1 << LOCATION_SWAMP;
    game.inventory[0] = item_make_staff();
    game.inventory_count = 1;
    game.ashen_cache[0].valid = 1;
    map_generate_ashen(&game.ashen_cache[0].map, 1);
    game.ashen_cache[0].enemy_count = 1;
    game.ashen_cache[0].enemies[0] = (Enemy){.type = ENEMY_ASH_HOUND, .active = 1, .hp = 7, .max_hp = 35};
    for (int x = STILLBURY_GLASSDEEP_GATE_X - 2; x <= STILLBURY_GLASSDEEP_GATE_X + 2; x++) {
        game.map.tiles[TOWN_H - 1][x] = TILE_WALL;
    }
    int saved = save_game(&game, GLASSDEEP_TEST_SLOT);
    FILE *file = fopen("saves/savegame_99132.json", "rb");
    if (!saved || !file) {
        ASSERT("legacy Glassdeep Caverns fixture can be written", 0);
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *buffer = calloc((size_t)length + 1, 1);
    fread(buffer, 1, (size_t)length, file);
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_DeleteItemFromObject(root, "glassdeep_cache");
    cJSON_DeleteItemFromObject(root, "max_glassdeep_level_reached");
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 81);
    buffer = cJSON_PrintUnformatted(root);
    file = fopen("saves/savegame_99132.json", "wb");
    fputs(buffer, file);
    fclose(file);
    free(buffer);
    cJSON_Delete(root);
    int ok = load_game(&loaded, GLASSDEEP_TEST_SLOT);
    int fresh = ok && loaded.max_glassdeep_level_reached == 1;
    for (int i = 0; i < GLASSDEEP_DEPTH; i++) {
        fresh &= !loaded.glassdeep_cache[i].valid && loaded.glassdeep_cache[i].enemy_count == 0;
    }
    ASSERT("version 81 saves receive fresh independent Glassdeep Caverns progression", fresh);
    ASSERT("migration opens Stillbury's south gate and preserves character and quest progress", ok && loaded.map.tiles[TOWN_H - 1][STILLBURY_GLASSDEEP_GATE_X] == TILE_TOWN_EXIT && loaded.player.hp == 89 && loaded.gold == 37 && loaded.score == 1234 && loaded.sunscar_lamp_quest_state == 1 && loaded.defeated_bosses == game.defeated_bosses && loaded.inventory_count == 1);
    ASSERT("migration preserves previously explored Ashen Hollow maps and enemies", ok && loaded.ashen_cache[0].valid && loaded.ashen_cache[0].enemies[0].hp == 7 && memcmp(&loaded.ashen_cache[0].map, &game.ashen_cache[0].map, sizeof(Map)) == 0);
    ASSERT("migrated saves rewrite and reload successfully", save_game(&loaded, GLASSDEEP_TEST_SLOT) && load_game(&game, GLASSDEEP_TEST_SLOT));
    game_init(&game);
    ASSERT("starting a new game resets Glassdeep Caverns progress", game.max_glassdeep_level_reached == 1 && !game.glassdeep_cache[0].valid);
    remove("saves/savegame_99132.json");
}

static void setup_actor(EnemyType type) {
    game_init(&game);
    game.location = LOCATION_GLASSDEEP;
    game.level = GLASSDEEP_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_GLASSDEEP_WALL;
        }
    }
    for (int y = 21; y <= 23; y++) {
        for (int x = 20; x <= 40; x++) {
            game.map.tiles[y][x] = TILE_GLASSDEEP_FLOOR;
        }
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){18, 18, 17, 10};
    game.player.x = 28;
    game.player.y = 22;
    game.player.hp = 100;
    game.player.defense = 2;
    game.enemies[0] = (Enemy){.type = type, .active = 1, .x = 24, .y = 22, .hp = 100, .max_hp = 100, .attack = 12, .is_boss = type == ENEMY_PRISM_SOVEREIGN, .attack_target_x = -1, .attack_target_y = -1};
    game.enemy_count = 1;
}

static void test_combat_roles(void) {
    setup_actor(ENEMY_CRYSTAL_SPIDER);
    action_resolve_enemies(&game);
    ASSERT("Crystal Spiders close two tiles without attacking on the second move", abs(game.enemies[0].x - 24) + abs(game.enemies[0].y - 22) == 2 && game.player.hp == 100);
    setup_actor(ENEMY_SHARD_GOLEM);
    action_resolve_enemies(&game);
    ASSERT("Shard Golems rest on their first movement turn", game.enemies[0].x == 24 && game.enemies[0].y == 22);
    action_resolve_enemies(&game);
    ASSERT("Shard Golems advance one tile on their next movement turn", abs(game.enemies[0].x - 24) + abs(game.enemies[0].y - 22) == 1);
    setup_actor(ENEMY_BLIND_STALKER);
    game.player.x = 30;
    action_resolve_enemies(&game);
    ASSERT("untouched Blind Stalkers wait while explorers remain distant", game.enemies[0].x == 24 && game.enemies[0].move_timer == 0);
    game.player.x = 27;
    action_resolve_enemies(&game);
    ASSERT("nearby explorers awaken a Blind Stalker's pursuit", game.enemies[0].x != 24 || game.enemies[0].y != 22);
    setup_actor(ENEMY_BLIND_STALKER);
    game.player.x = 30;
    game.enemies[0].hp = 99;
    action_resolve_enemies(&game);
    ASSERT("damaging a distant Blind Stalker also wakes it", game.enemies[0].x != 24 || game.enemies[0].y != 22);
}

static void test_prism_beam(void) {
    EnemyProjectiles shots;
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the Sovereign marks a beam without damage on its warning turn", shots.count == 0 && game.player.hp == 100 && game.enemies[0].attack_target_x == 32 && game.enemies[0].attack_target_y == 22 && game.enemies[0].x == 24);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the next turn fires the warned beam and clears the warning", shots.count == 1 && shots.shots[0].type == ENEMY_PRISM_SOVEREIGN && shots.shots[0].target_x == 32 && shots.shots[0].target_y == 22 && game.player.hp < 100 && game.enemies[0].attack_target_x == -1);
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    action_resolve_enemies(&game);
    game.player.y = 23;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("sidestepping a marked beam dodges it without changing its aim", shots.count == 1 && shots.shots[0].target_y == 22 && game.player.hp == 100);
    const TileType obstacles[2] = {TILE_GLASSDEEP_POOL, TILE_GLASSDEEP_WALL};
    for (int i = 0; i < 2; i++) {
        setup_actor(ENEMY_PRISM_SOVEREIGN);
        action_resolve_enemies(&game);
        game.map.tiles[22][26] = obstacles[i];
        action_resolve_enemies_with_projectiles(&game, &shots);
        ASSERT("cave walls and pools stop a beam before an explorer behind them", shots.count == 1 && shots.shots[0].target_x == 25 && game.player.hp == 100);
    }
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    action_resolve_enemies(&game);
    game.enemy_count = 2;
    game.enemies[1] = (Enemy){.type = ENEMY_SHARD_GOLEM, .active = 1, .x = 26, .y = 22, .hp = 100, .max_hp = 100, .frozen_turns = 2};
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("another active enemy blocks the Sovereign's line of fire", shots.count == 1 && shots.shots[0].target_x == 25 && game.player.hp == 100);
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    game.player.x = 25;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("adjacent combat retains the beam's warning turn", shots.count == 0 && game.player.hp == 100);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the Sovereign's beam damages a player at melee range", shots.count == 1 && game.player.hp < 100);
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    game.player.x = 24;
    game.player.y = 23;
    action_resolve_enemies(&game);
    ASSERT("north-south beams are marked on the player's column", game.enemies[0].attack_target_x == 24 && game.enemies[0].attack_target_y == 30);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("a vertical beam stops at the next cave wall after hitting", shots.count == 1 && shots.shots[0].target_x == 24 && shots.shots[0].target_y == 23 && game.player.hp < 100);
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    action_resolve_enemies(&game);
    int saved = save_game(&game, GLASSDEEP_TEST_SLOT) && load_game(&loaded, GLASSDEEP_TEST_SLOT);
    ASSERT("save/load preserves a pending beam and its firing turn", saved && loaded.enemies[0].move_timer == 1 && loaded.enemies[0].attack_target_x == 32 && loaded.enemies[0].attack_target_y == 22);
    if (saved) {
        game = loaded;
        game.player.y = 23;
        action_resolve_enemies_with_projectiles(&game, &shots);
    }
    ASSERT("a saved beam remains dodgeable after loading", saved && shots.count == 1 && shots.shots[0].target_y == 22 && game.player.hp == 100);
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    action_resolve_enemies(&game);
    game.enemies[0].frozen_turns = 2;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("freezing the Sovereign pauses its pending warning", shots.count == 0 && game.enemies[0].move_timer == 1 && game.enemies[0].attack_target_x == 32);
    setup_actor(ENEMY_PRISM_SOVEREIGN);
    game.player.x = 40;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the untouched Sovereign stays dormant outside its sanctuary", shots.count == 0 && game.enemies[0].move_timer == 0 && game.enemies[0].attack_target_x == -1);
    remove("saves/savegame_99132.json");
}

void test_glassdeep(void) {
    printf("Glassdeep Caverns tests:\n");
    int unused = !save_exists(GLASSDEEP_TEST_SLOT);
    ASSERT("Glassdeep Caverns temporary test slot is unused", unused);
    if (!unused) {
        return;
    }
    test_layouts();
    test_travel_and_saves();
    test_boss_victories();
    test_combat_roles();
    test_prism_beam();
    test_migration();
}
