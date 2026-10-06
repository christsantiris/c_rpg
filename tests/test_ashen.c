#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ASHEN_TEST_SLOT 99131
static GameState game;
static GameState loaded;
static Map layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static void walk_edge(int north) {
    int x = north ? game.map.stairs_down_x : game.map.stairs_up_x;
    int y = north ? game.map.stairs_down_y : game.map.stairs_up_y;
    game.player.x = x;
    game.player.y = north ? y + 1 : y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
}

static void test_layouts(void) {
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    for (int level = 1; level <= ASHEN_DEPTH; level++) {
        int connected = 1;
        int roster = 1;
        int landmarks = 1;
        for (int seed = 0; seed < 12; seed++) {
            srand(2810 + seed);
            map_generate_ashen(&game.map, level);
            game.location = LOCATION_ASHEN;
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
            int lava = 0;
            int stones = 0;
            for (int y = 0; y < SWAMP_MAP_H; y++) {
                for (int x = 0; x < SWAMP_MAP_W; x++) {
                    TileType tile = game.map.tiles[y][x];
                    if (tile == TILE_ASHEN_LAVA) {
                        lava++;
                        landmarks &= !map_is_walkable(&game.map, x, y);
                    } else if (tile == TILE_ASHEN_RUIN) {
                        stones++;
                        landmarks &= map_is_walkable(&game.map, x, y);
                    }
                }
            }
            landmarks &= lava > 0 && stones > 0 && game.map.stairs_up_y == SWAMP_MAP_H - 1 && game.map.stairs_down_y == 0;
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
                roster &= enemy->type >= ENEMY_CINDER_IMP && enemy->type <= ENEMY_CINDER_LORD && visited[enemy->y][enemy->x];
                bosses += enemy->is_boss;
            }
            roster &= bosses == (level == ASHEN_DEPTH);
        }
        ASSERT("volcanic entrances, exits, and every clearing remain connected across seeds", connected);
        ASSERT("lava block movement and ruined paving remain walkable", landmarks);
        ASSERT("each stage spawns its own reachable roster and only stage five has a boss", roster);
    }
}

static void test_travel_and_saves(void) {
    game_init(&game);
    srand(4612);
    game_enter_town4(&game);
    game.player.x = RIDGESHIRE_ASHEN_GATE_X;
    game.player.y = 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, 0});
    ASSERT("Ridgeshire's north gate enters Ashen Hollow stage one from the south", game.location == LOCATION_ASHEN && game.level == 1 && game.player.y == SWAMP_MAP_H - 1);
    int x;
    int y;
    map_room_center(&game.map.rooms[0], &x, &y);
    map_mark_explored(&game.map, x, y);
    game.enemies[0].hp = 7;
    layout = game.map;
    walk_edge(1);
    ASSERT("the north exit advances with regular enemies still alive", game.level == 2 && game.max_ashen_level_reached == 2 && game.ashen_cache[0].valid);
    walk_edge(0);
    ASSERT("backtracking preserves maps, exploration, and enemy health", game.level == 1 && memcmp(&layout, &game.map, sizeof(layout)) == 0 && game.enemies[0].hp == 7);
    walk_edge(0);
    ASSERT("the south exit returns at Ridgeshire's north gate", game.location == LOCATION_TOWN4 && game.player.x == RIDGESHIRE_ASHEN_GATE_X && game.player.y == 1);
    game_enter_ashen(&game);
    ASSERT("re-entering Ashen Hollow retains visited stages and progress", game.enemies[0].hp == 7 && game.max_ashen_level_reached == 2 && memcmp(&layout, &game.map, sizeof(layout)) == 0);
    while (game.level < 3) {
        walk_edge(1);
    }
    ASSERT("saving and loading Ashen Hollow preserves independent stage caches", save_game(&game, ASHEN_TEST_SLOT) && load_game(&loaded, ASHEN_TEST_SLOT) && loaded.location == LOCATION_ASHEN && loaded.level == 3 && loaded.max_ashen_level_reached == 3 && loaded.ashen_cache[0].enemies[0].hp == 7 && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0);
    map_room_center(&game.map.rooms[1], &x, &y);
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x] = TILE_ITEM;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_ASHEN_RUIN, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game_open_town_portal(&game);
    ASSERT("Return to Town places a volcanic portal beside Ridgeshire's north gate", game.location == LOCATION_TOWN4 && game.portal_location == LOCATION_ASHEN && game.map.tiles[2][RIDGESHIRE_ASHEN_GATE_X + 1] == TILE_PORTAL && game.portal_origin_tile == TILE_ASHEN_RUIN);
    game.player.x = RIDGESHIRE_ASHEN_GATE_X + 1;
    game.player.y = 3;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, 2});
    ASSERT("stepping onto Ridgeshire's portal resumes the original Ashen stage and position", game.location == LOCATION_ASHEN && game.level == 3 && game.player.x == x && game.player.y == y);
    if (game.location != LOCATION_ASHEN) {
        remove("saves/savegame_99131.json");
        return;
    }
    game_open_town_portal(&game);
    int saved = save_game(&game, ASHEN_TEST_SLOT) && load_game(&loaded, ASHEN_TEST_SLOT);
    if (saved) {
        game = loaded;
        game.player.x = RIDGESHIRE_ASHEN_GATE_X + 1;
        game.player.y = 3;
        action_resolve_player(&game, (Action){ACTION_MOVE, game.player.x, 2});
    }
    ASSERT("stepping onto a saved return portal restores the volcanic stage and underlying paving", saved && game.location == LOCATION_ASHEN && game.level == 3 && game.player.x == x && game.player.y == y && game.map.tiles[y][x] == TILE_ASHEN_RUIN);
    if (game.location != LOCATION_ASHEN) {
        remove("saves/savegame_99131.json");
        return;
    }
    while (game.level < ASHEN_DEPTH) {
        walk_edge(1);
    }
    walk_edge(1);
    ASSERT("the living Cinder Lord blocks the final return exit", game.location == LOCATION_ASHEN && game.level == ASHEN_DEPTH);
    game.portal_level = ASHEN_DEPTH + 1;
    game.portal_location = LOCATION_ASHEN;
    game.location = LOCATION_TOWN4;
    game_use_town_portal(&game);
    ASSERT("out-of-range volcanic portals cannot index past the cache", game.location == LOCATION_TOWN4);
    remove("saves/savegame_99131.json");
}

static void test_boss_victories(void) {
    for (int attack = 0; attack < 3; attack++) {
        game_init(&game);
        srand(6100 + attack);
        game.location = LOCATION_ASHEN;
        game.level = ASHEN_DEPTH;
        map_generate_ashen(&game.map, game.level);
        enemies_spawn(&game);
        Enemy *boss = &game.enemies[0];
        boss->hp = 1;
        game.player.x = boss->x - (attack == 0 ? 1 : 2);
        game.player.y = boss->y;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.mp = 100;
        game.inventory[0] = attack == 2 ? item_make_bow() : item_make_staff();
        game.inventory_count = 1;
        game.equipped_main_hand = 0;
        game.player.known_spells[0] = spell_make_magic_arrow();
        game.player.known_spell_count = 1;
        game.player.equipped_spell = 0;
        Action action = attack == 0 ? (Action){ACTION_MOVE, boss->x, boss->y} :
            (Action){attack == 1 ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
        action_resolve_player(&game, action);
        int rewards = 0;
        for (int i = 0; i < game.floor_item_count; i++) {
            rewards += game.floor_items[i].active && game.floor_items[i].item.type == ITEM_POTION_STRENGTH;
        }
        BossJournalEntry entry;
        ASSERT("melee, spell, and bow victories grant the Cinder Lord reward and journal completion", !boss->active && (game.defeated_bosses & (1 << LOCATION_ASHEN)) && rewards == 1 && quest_journal_get_boss(&game, 10, &entry) && entry.defeated && strcmp(entry.name, "Cinder Lord") == 0);
        walk_edge(1);
        ASSERT("defeating the Cinder Lord allows return while other enemies remain", game.location == LOCATION_TOWN4 && game.ashen_cache[ASHEN_DEPTH - 1].enemy_count > 1);
        game_enter_ashen(&game);
        while (game.level < ASHEN_DEPTH) {
            walk_edge(1);
        }
        int alive = 0;
        for (int i = 0; i < game.enemy_count; i++) {
            alive += game.enemies[i].active && game.enemies[i].type == ENEMY_CINDER_LORD;
        }
        ASSERT("the defeated Cinder Lord remains defeated on later visits", alive == 0);
    }
}

static void test_migration(void) {
    game_init(&game);
    game_enter_town4(&game);
    game.player.hp = 89;
    game.gold = 37;
    game.score = 1234;
    game.sunscar_lamp_quest_state = 1;
    game.defeated_bosses = 1 << LOCATION_SWAMP;
    game.inventory[0] = item_make_staff();
    game.inventory_count = 1;
    for (int x = RIDGESHIRE_ASHEN_GATE_X - 2; x <= RIDGESHIRE_ASHEN_GATE_X + 2; x++) {
        game.map.tiles[0][x] = TILE_WALL;
    }
    int saved = save_game(&game, ASHEN_TEST_SLOT);
    FILE *file = fopen("saves/savegame_99131.json", "rb");
    if (!saved || !file) {
        ASSERT("legacy Ashen Hollow fixture can be written", 0);
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
    cJSON_DeleteItemFromObject(root, "ashen_cache");
    cJSON_DeleteItemFromObject(root, "max_ashen_level_reached");
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 80);
    buffer = cJSON_PrintUnformatted(root);
    file = fopen("saves/savegame_99131.json", "wb");
    fputs(buffer, file);
    fclose(file);
    free(buffer);
    cJSON_Delete(root);
    int ok = load_game(&loaded, ASHEN_TEST_SLOT);
    int fresh = ok && loaded.max_ashen_level_reached == 1;
    for (int i = 0; i < ASHEN_DEPTH; i++) {
        fresh &= !loaded.ashen_cache[i].valid && loaded.ashen_cache[i].enemy_count == 0;
    }
    ASSERT("version 80 saves receive fresh independent Ashen Hollow progression", fresh);
    ASSERT("migration opens Ridgeshire's north gate and preserves character and quest progress", ok && loaded.map.tiles[0][RIDGESHIRE_ASHEN_GATE_X] == TILE_TOWN_EXIT && loaded.player.hp == 89 && loaded.gold == 37 && loaded.score == 1234 && loaded.sunscar_lamp_quest_state == 1 && loaded.defeated_bosses == game.defeated_bosses && loaded.inventory_count == 1);
    ASSERT("migrated saves rewrite and reload successfully", save_game(&loaded, ASHEN_TEST_SLOT) && load_game(&game, ASHEN_TEST_SLOT));
    game_init(&game);
    ASSERT("starting a new game resets Ashen Hollow progress", game.max_ashen_level_reached == 1 && !game.ashen_cache[0].valid);
    remove("saves/savegame_99131.json");
}

static void setup_actor(EnemyType type) {
    game_init(&game);
    game.location = LOCATION_ASHEN;
    game.level = ASHEN_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_ASHEN_WALL;
        }
    }
    for (int x = 20; x <= 40; x++) {
        game.map.tiles[22][x] = TILE_ASHEN_FLOOR;
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){18, 18, 17, 10};
    game.player.x = 28;
    game.player.y = 22;
    game.player.hp = 100;
    game.player.defense = 2;
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.type = type, .active = 1, .x = 24, .y = 22, .hp = 100, .max_hp = 100, .attack = 12, .is_boss = type == ENEMY_CINDER_LORD};
}

static void test_combat_roles(void) {
    EnemyProjectiles shots;
    const EnemyType ranged[1] = {ENEMY_CINDER_IMP};
    for (int i = 0; i < 1; i++) {
        setup_actor(ranged[i]);
        action_resolve_enemies_with_projectiles(&game, &shots);
        ASSERT("Cinder Imps leave an opening between ranged attacks", shots.count == 0 && game.player.hp == 100);
        action_resolve_enemies_with_projectiles(&game, &shots);
        ASSERT("volcanic ranged attacks create projectiles and damage the player", shots.count == 1 && shots.shots[0].type == ranged[i] && game.player.hp < 100);
    }
    setup_actor(ENEMY_ASH_HOUND);
    action_resolve_enemies(&game);
    ASSERT("Ash Hounds close two corridor tiles without attacking on the second move", game.enemies[0].x == 26 && game.player.hp == 100);
    setup_actor(ENEMY_OBSIDIAN_GUARDIAN);
    action_resolve_enemies(&game);
    ASSERT("Obsidian Guardians rest on their first movement turn", game.enemies[0].x == 24);
    action_resolve_enemies(&game);
    ASSERT("Obsidian Guardians advance one tile on their next movement turn", game.enemies[0].x == 25);
    setup_actor(ENEMY_CINDER_LORD);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the Cinder Lord telegraphs its spell and holds its clearing", shots.count == 0 && game.player.hp == 100 && game.enemies[0].x == 24);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the Cinder Lord's next turn fires a damaging fire projectile", shots.count == 1 && shots.shots[0].type == ENEMY_CINDER_LORD && game.player.hp < 100);
    setup_actor(ENEMY_CINDER_LORD);
    game.enemies[0].move_timer = 1;
    game.map.tiles[22][26] = TILE_ASHEN_LAVA;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("lava pools block enemy spells rather than allowing shots through terrain", shots.count == 0 && game.player.hp == 100);
    setup_actor(ENEMY_CINDER_LORD);
    game.player.x = 40;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the untouched Cinder Lord stays dormant outside its final clearing", shots.count == 0 && game.player.hp == 100 && game.enemies[0].move_timer == 0);
}

void test_ashen(void) {
    printf("Ashen Hollow tests:\n");
    ASSERT("Ashen Hollow temporary test slot is unused", !save_exists(ASHEN_TEST_SLOT));
    test_layouts();
    test_travel_and_saves();
    test_boss_victories();
    test_combat_roles();
    test_migration();
}
