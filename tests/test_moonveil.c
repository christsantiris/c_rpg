#include "test_utils.h"
#include "game/game.h"
#include "systems/save_load.h"
#include "screens/quest_journal.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define MOONVEIL_TEST_SLOT 99130
static GameState game;
static GameState loaded;
static Map layout;
static unsigned char visited[MAP_H][MAP_W];
static int queue[MAP_W * MAP_H];

static void walk_edge(int west) {
    int x = west ? game.map.stairs_down_x : game.map.stairs_up_x;
    int y = west ? game.map.stairs_down_y : game.map.stairs_up_y;
    game.player.x = west ? x + 1 : x - 1;
    game.player.y = y;
    action_resolve_player(&game, (Action){ACTION_MOVE, x, y});
}

static void test_layouts(void) {
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    for (int level = 1; level <= MOONVEIL_DEPTH; level++) {
        int connected = 1;
        int roster = 1;
        int landmarks = 1;
        for (int seed = 0; seed < 12; seed++) {
            srand(2810 + seed);
            map_generate_moonveil(&game.map, level);
            game.location = LOCATION_MOONVEIL;
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
                    if (tile == TILE_MOONVEIL_POOL) {
                        pools++;
                        landmarks &= !map_is_walkable(&game.map, x, y);
                    } else if (tile == TILE_MOONVEIL_CIRCLE) {
                        stones++;
                        landmarks &= map_is_walkable(&game.map, x, y);
                    }
                }
            }
            landmarks &= pools > 0 && stones > 0 && game.map.stairs_up_x == SWAMP_MAP_W - 1 && game.map.stairs_down_x == 0;
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
                roster &= enemy->type >= ENEMY_FEY_TRICKSTER && enemy->type <= ENEMY_THORN_REGENT && visited[enemy->y][enemy->x];
                bosses += enemy->is_boss;
            }
            roster &= bosses == (level == MOONVEIL_DEPTH);
        }
        ASSERT("garden entrances, exits, and every clearing remain connected across seeds", connected);
        ASSERT("pools block movement and stone circles remain walkable", landmarks);
        ASSERT("each stage spawns its own reachable roster and only stage five has a boss", roster);
    }
}

static void test_travel_and_saves(void) {
    game_init(&game);
    game_enter_town3(&game);
    game.player.x = 1;
    game.player.y = ROSEMOOR_MOONVEIL_GATE_Y;
    action_resolve_player(&game, (Action){ACTION_MOVE, 0, game.player.y});
    ASSERT("Rosemoor's west gate enters Moonveil stage one from the east", game.location == LOCATION_MOONVEIL && game.level == 1 && game.player.x == SWAMP_MAP_W - 1);
    int x;
    int y;
    map_room_center(&game.map.rooms[0], &x, &y);
    map_mark_explored(&game.map, x, y);
    game.enemies[0].hp = 7;
    layout = game.map;
    walk_edge(1);
    ASSERT("the west exit advances with regular enemies still alive", game.level == 2 && game.max_moonveil_level_reached == 2 && game.moonveil_cache[0].valid);
    walk_edge(0);
    ASSERT("backtracking preserves maps, exploration, and enemy health", game.level == 1 && memcmp(&layout, &game.map, sizeof(layout)) == 0 && game.enemies[0].hp == 7);
    walk_edge(0);
    ASSERT("the east exit returns at Rosemoor's west gate", game.location == LOCATION_TOWN3 && game.player.x == 1 && game.player.y == ROSEMOOR_MOONVEIL_GATE_Y);
    game_enter_moonveil(&game);
    ASSERT("re-entering Moonveil retains visited stages and progress", game.enemies[0].hp == 7 && game.max_moonveil_level_reached == 2 && memcmp(&layout, &game.map, sizeof(layout)) == 0);
    while (game.level < 3) {
        walk_edge(1);
    }
    ASSERT("saving and loading Moonveil preserves independent stage caches", save_game(&game, MOONVEIL_TEST_SLOT) && load_game(&loaded, MOONVEIL_TEST_SLOT) && loaded.location == LOCATION_MOONVEIL && loaded.level == 3 && loaded.max_moonveil_level_reached == 3 && loaded.moonveil_cache[0].enemies[0].hp == 7 && memcmp(&game.map, &loaded.map, sizeof(Map)) == 0);
    map_room_center(&game.map.rooms[1], &x, &y);
    // The player cannot stand on a tile occupied by a randomly spawned enemy.
    for (int i = 0; i < game.enemy_count; i++) {
        if (game.enemies[i].x == x && game.enemies[i].y == y) {
            game.enemies[i].active = 0;
        }
    }
    game.player.x = x;
    game.player.y = y;
    game.map.tiles[y][x] = TILE_ITEM;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_MOONVEIL_CIRCLE, .item = item_make_health_potion()};
    game.floor_item_count = 1;
    game_open_town_portal(&game);
    ASSERT("Return to Town places a garden portal beside Rosemoor's west gate", game.location == LOCATION_TOWN3 && game.portal_location == LOCATION_MOONVEIL && game.map.tiles[ROSEMOOR_MOONVEIL_GATE_Y + 1][2] == TILE_PORTAL && game.portal_origin_tile == TILE_MOONVEIL_CIRCLE);
    int saved = save_game(&game, MOONVEIL_TEST_SLOT) && load_game(&loaded, MOONVEIL_TEST_SLOT);
    if (saved) {
        game = loaded;
        game.player.x = 2;
        game.player.y = ROSEMOOR_MOONVEIL_GATE_Y + 2;
        action_resolve_player(&game, (Action){ACTION_MOVE, 2, game.player.y - 1});
    }
    ASSERT("stepping onto a saved return portal restores the garden stage and underlying stone", saved && game.location == LOCATION_MOONVEIL && game.level == 3 && game.player.x == x && game.player.y == y && game.map.tiles[y][x] == TILE_MOONVEIL_CIRCLE);
    if (game.location != LOCATION_MOONVEIL) {
        remove("saves/savegame_99130.json");
        return;
    }
    while (game.level < MOONVEIL_DEPTH) {
        walk_edge(1);
    }
    walk_edge(1);
    ASSERT("the living Thorn Regent blocks the final return exit", game.location == LOCATION_MOONVEIL && game.level == MOONVEIL_DEPTH);
    game.portal_level = MOONVEIL_DEPTH + 1;
    game.portal_location = LOCATION_MOONVEIL;
    game.location = LOCATION_TOWN3;
    game_use_town_portal(&game);
    ASSERT("out-of-range garden portals cannot index past the cache", game.location == LOCATION_TOWN3);
    remove("saves/savegame_99130.json");
}

static void test_boss_victories(void) {
    for (int attack = 0; attack < 3; attack++) {
        game_init(&game);
        game.location = LOCATION_MOONVEIL;
        game.level = MOONVEIL_DEPTH;
        map_generate_moonveil(&game.map, game.level);
        enemies_spawn(&game);
        Enemy *boss = &game.enemies[0];
        boss->hp = 1;
        game.player.x = boss->x - (attack == 0 ? 1 : 2);
        game.player.y = boss->y;
        game.player.last_dx = 1;
        game.player.last_dy = 0;
        game.player.mp = 100;
        game.inventory[0] = attack == 2 ? item_make_bow() : item_make_staff();
        game.player.arrows = MAX_ARROWS;
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
        ASSERT("melee, spell, and bow victories grant the Regent reward and journal completion", !boss->active && (game.defeated_bosses & (1 << LOCATION_MOONVEIL)) && rewards == 1 && quest_journal_get_boss(&game, 9, &entry) && entry.defeated && strcmp(entry.name, "Thorn Regent") == 0);
        walk_edge(1);
        ASSERT("defeating the Regent allows return while other enemies remain", game.location == LOCATION_TOWN3 && game.moonveil_cache[MOONVEIL_DEPTH - 1].enemy_count > 1);
        game_enter_moonveil(&game);
        while (game.level < MOONVEIL_DEPTH) {
            walk_edge(1);
        }
        int alive = 0;
        for (int i = 0; i < game.enemy_count; i++) {
            alive += game.enemies[i].active && game.enemies[i].type == ENEMY_THORN_REGENT;
        }
        ASSERT("the defeated Regent remains defeated on later visits", alive == 0);
    }
}

static void test_migration(void) {
    game_init(&game);
    game_enter_town3(&game);
    game.player.hp = 89;
    game.gold = 37;
    game.score = 1234;
    game.sunscar_lamp_quest_state = 1;
    game.defeated_bosses = 1 << LOCATION_SWAMP;
    game.inventory[0] = item_make_staff();
    game.inventory_count = 1;
    for (int y = ROSEMOOR_MOONVEIL_GATE_Y - 2; y <= ROSEMOOR_MOONVEIL_GATE_Y + 2; y++) {
        game.map.tiles[y][0] = TILE_WALL;
    }
    int saved = save_game(&game, MOONVEIL_TEST_SLOT);
    FILE *file = fopen("saves/savegame_99130.json", "rb");
    if (!saved || !file) {
        ASSERT("legacy Moonveil fixture can be written", 0);
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
    cJSON_DeleteItemFromObject(root, "moonveil_cache");
    cJSON_DeleteItemFromObject(root, "max_moonveil_level_reached");
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 79);
    buffer = cJSON_PrintUnformatted(root);
    file = fopen("saves/savegame_99130.json", "wb");
    fputs(buffer, file);
    fclose(file);
    free(buffer);
    cJSON_Delete(root);
    int ok = load_game(&loaded, MOONVEIL_TEST_SLOT);
    int fresh = ok && loaded.max_moonveil_level_reached == 1;
    for (int i = 0; i < MOONVEIL_DEPTH; i++) {
        fresh &= !loaded.moonveil_cache[i].valid && loaded.moonveil_cache[i].enemy_count == 0;
    }
    ASSERT("version 79 saves receive fresh independent Moonveil progression", fresh);
    ASSERT("migration opens Rosemoor's west gate and preserves character and quest progress", ok && loaded.map.tiles[ROSEMOOR_MOONVEIL_GATE_Y][0] == TILE_TOWN_EXIT && loaded.player.hp == 89 && loaded.gold == 37 && loaded.score == 1234 && loaded.sunscar_lamp_quest_state == 1 && loaded.defeated_bosses == game.defeated_bosses && loaded.inventory_count == 1);
    ASSERT("migrated saves rewrite and reload successfully", save_game(&loaded, MOONVEIL_TEST_SLOT) && load_game(&game, MOONVEIL_TEST_SLOT));
    game_init(&game);
    ASSERT("starting a new game resets Moonveil progress", game.max_moonveil_level_reached == 1 && !game.moonveil_cache[0].valid);
    remove("saves/savegame_99130.json");
}

static void setup_actor(EnemyType type) {
    game_init(&game);
    game.location = LOCATION_MOONVEIL;
    game.level = MOONVEIL_DEPTH;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            game.map.tiles[y][x] = TILE_MOONVEIL_WALL;
        }
    }
    for (int x = 20; x <= 40; x++) {
        game.map.tiles[22][x] = TILE_MOONVEIL_FLOOR;
    }
    game.map.room_count = 1;
    game.map.rooms[0] = (Room){18, 18, 17, 10};
    game.player.x = 28;
    game.player.y = 22;
    game.player.hp = 100;
    game.player.defense = 2;
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.type = type, .active = 1, .x = 24, .y = 22, .hp = 100, .max_hp = 100, .attack = 12, .is_boss = type == ENEMY_THORN_REGENT};
}

static void test_combat_roles(void) {
    EnemyProjectiles shots;
    const EnemyType ranged[2] = {ENEMY_FEY_TRICKSTER, ENEMY_LIVING_FLOWER};
    for (int i = 0; i < 2; i++) {
        setup_actor(ranged[i]);
        action_resolve_enemies_with_projectiles(&game, &shots);
        ASSERT("Tricksters and Carnivorous Flowers leave an opening between ranged attacks", shots.count == 0 && game.player.hp == 100);
        action_resolve_enemies_with_projectiles(&game, &shots);
        ASSERT("garden ranged attacks create projectiles and damage the player", shots.count == 1 && shots.shots[0].type == ranged[i] && game.player.hp < 100);
    }
    setup_actor(ENEMY_GIANT_MOTH);
    action_resolve_enemies(&game);
    ASSERT("Giant Moths close two corridor tiles without attacking on the second move", game.enemies[0].x == 26 && game.player.hp == 100);
    setup_actor(ENEMY_THORN_GUARDIAN);
    action_resolve_enemies(&game);
    ASSERT("Thorn Guardians rest on their first movement turn", game.enemies[0].x == 24);
    action_resolve_enemies(&game);
    ASSERT("Thorn Guardians advance one tile on their next movement turn", game.enemies[0].x == 25);
    setup_actor(ENEMY_THORN_REGENT);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the Regent telegraphs its spell and holds its clearing", shots.count == 0 && game.player.hp == 100 && game.enemies[0].x == 24);
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the Regent's next turn fires a damaging thorn projectile", shots.count == 1 && shots.shots[0].type == ENEMY_THORN_REGENT && game.player.hp < 100);
    setup_actor(ENEMY_THORN_REGENT);
    game.enemies[0].move_timer = 1;
    game.map.tiles[22][26] = TILE_MOONVEIL_POOL;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("moonlit pools block enemy spells rather than allowing shots through terrain", shots.count == 0 && game.player.hp == 100);
    setup_actor(ENEMY_THORN_REGENT);
    game.player.x = 40;
    action_resolve_enemies_with_projectiles(&game, &shots);
    ASSERT("the untouched Regent stays dormant outside its final clearing", shots.count == 0 && game.player.hp == 100 && game.enemies[0].move_timer == 0);
}

void test_moonveil(void) {
    printf("Moonveil Gardens tests:\n");
    ASSERT("Moonveil temporary test slot is unused", !save_exists(MOONVEIL_TEST_SLOT));
    test_layouts();
    test_travel_and_saves();
    test_boss_victories();
    test_combat_roles();
    test_migration();
}
