#include "test_utils.h"
#include "castle_route_checks.h"
#include "game/castle.h"
#include "game/castle_gatehouse.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define GATEHOUSE_SLOT 99360
#define GATEHOUSE_SAVE "saves/savegame_99360.json"

static GameState game;
static GameState loaded;

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static int set_legacy_version(void) {
    FILE *file = fopen(GATEHOUSE_SAVE, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t read = fread(text, 1, size, file);
    text[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 252);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = text ? fopen(GATEHOUSE_SAVE, "wb") : NULL;
    int ok = file != NULL;
    if (file) {
        fputs(text, file);
        fclose(file);
    }
    free(text);
    return ok;
}

static void legacy_layout(Map *m) {
    // The pre-V2 gatehouse used six 22-by-14 chambers on a regular grid.
    castle_legacy_layout(m, 1);
    m->burial_trap_count = 0;
    m->tiles[32][17] = TILE_CASTLE_FLOOR;
    m->tiles[21][15] = TILE_CASTLE_GATE_OPEN;
    m->tiles[15][10] = TILE_CASTLE_LEVER;
    for (int y = 0; y < CASTLE_H; y++) {
        for (int x = 0; x < CASTLE_W; x++) {
            if (m->tiles[y][x] == TILE_CASTLE_MUSTER || (y >= 47 && y <= 49 && (x == 40 || x == 50))) {
                m->tiles[y][x] = TILE_CASTLE_FLOOR;
            }
        }
    }
}

static void test_gatehouse_migration(void) {
    game_init(&game);
    castle_enter(&game, 1);
    legacy_layout(&game.map);
    game.player.x = 25;
    game.player.y = 17;
    game.player.hp = 41;
    game.gold = 713;
    game.castle_minibosses = 3;
    game.elowen_quest_state = 3;
    game.enemies[0].x = 24;
    game.enemies[0].y = 45;
    game.enemies[0].hp = 17;
    game.enemies[1].active = 0;
    game.enemies[1].hp = 0;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 28, .y = 36, .underlying_tile = TILE_CASTLE_FLOOR,
        .item = item_make_health_potion()};
    game.map.tiles[36][28] = TILE_ITEM;
    map_mark_explored(&game.map, 15, 11);
    map_mark_explored(&game.map, 28, 36);
    castle_store(&game);
    unsigned char explored[MAP_EXPLORED_BYTES];
    memcpy(explored, game.map.explored, sizeof(explored));
    int ok = save_game(&game, GATEHOUSE_SLOT) && set_legacy_version() && load_game(&loaded, GATEHOUSE_SLOT);
    ASSERT("version-252 active and cached gatehouses migrate to ten rooms", ok && loaded.map.room_count == 10 && loaded.castle_cache[0].map.room_count == 10);
    ASSERT("migration preserves player, quests, money and castle victories", ok &&
        loaded.player.x == 25 && loaded.player.y == 17 && loaded.player.hp == 41 && loaded.gold == 713 &&
        loaded.castle_minibosses == 3 && loaded.elowen_quest_state == 3);
    ASSERT("migration retains exploration bits exactly", ok && !memcmp(explored, loaded.map.explored, sizeof(explored)) &&
        !memcmp(explored, loaded.castle_cache[0].map.explored, sizeof(explored)));
    ASSERT("both enemy records retain positions, wounds and deaths", ok &&
        loaded.enemies[0].x == 24 && loaded.enemies[0].y == 45 && loaded.enemies[0].hp == 17 &&
        !loaded.enemies[1].active && !loaded.enemies[1].hp &&
        loaded.castle_cache[0].enemies[0].hp == 17 && !loaded.castle_cache[0].enemies[1].active);
    ASSERT("active and cached dropped loot stays at its old position without extra supplies", ok &&
        loaded.floor_item_count == 1 && loaded.castle_loot_count[0] == 1 &&
        loaded.map.tiles[36][28] == TILE_ITEM && loaded.castle_cache[0].map.tiles[36][28] == TILE_ITEM &&
        loaded.floor_items[0].x == 28 && loaded.floor_items[0].y == 36);
    if (ok) {
        clear_enemies(&loaded);
        ASSERT("an old player position connects to both actual stairs after migration", castle_walk_to(&loaded, 15, 11) && castle_walk_to(&loaded, 45, 55));
        action_resolve_player(&loaded, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the migrated onward stair enters floor two", loaded.location == LOCATION_CASTLE_INTERIOR && loaded.level == 2);
        clear_enemies(&loaded);
        action_resolve_player(&loaded, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("retreat revisits the migrated gatehouse at its onward stair", loaded.level == 1 && loaded.player.x == 45 && loaded.player.y == 55);
        ASSERT("the migrated floor survives a current-format round trip", save_game(&loaded, GATEHOUSE_SLOT) && load_game(&game, GATEHOUSE_SLOT) && game.map.room_count == 10 && game.floor_item_count == 1);
    }
    remove(GATEHOUSE_SAVE);
}

static void test_castle_stair_routes(void) {
    for (unsigned seed = 11; seed <= 14; seed++) {
        srand(seed);
        game_init(&game);
        game.castle_minibosses = 3;
        castle_enter(&game, 1);
        for (int level = 1; level < CASTLE_DEPTH; level++) {
            clear_enemies(&game);
            int ok = castle_walk_to(&game, game.map.stairs_down_x, game.map.stairs_down_y);
            action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
            ASSERT("each castle stage retains its actual onward stair after encounter placement", ok && game.level == level + 1);
        }
        for (int level = CASTLE_DEPTH; level >= 1; level--) {
            clear_enemies(&game);
            int ok = castle_walk_to(&game, game.map.stairs_up_x, game.map.stairs_up_y);
            action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
            ASSERT("every stage retains actual retreat through cached floors to Castle Grounds", ok &&
                (level == 1 ? game.location == LOCATION_CASTLE : game.location == LOCATION_CASTLE_INTERIOR && game.level == level - 1));
        }
    }
    game_init(&game);
    castle_enter(&game, 1);
    clear_enemies(&game);
    game.floor_items[2] = (FloorItem){.active = 1, .x = 45, .y = 55, .underlying_tile = TILE_STAIRS_UP,
        .item = item_make_health_potion()};
    game.floor_item_count = 3;
    game.map.tiles[55][45] = TILE_ITEM;
    ASSERT("dropped loot does not obstruct the continuous route onto the onward stair", castle_walk_to(&game, 45, 55));
    action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
    ASSERT("an item-covered staircase still performs the actual onward transition", game.level == 2);
    clear_enemies(&game);
    action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
    ASSERT("cached revisits retain both the dropped item and its usable staircase", game.level == 1 &&
        game.floor_items[2].active && game.floor_items[2].underlying_tile == TILE_STAIRS_UP);
}

void test_castle_gatehouse(void) {
    printf("Castle V2 gatehouse tests:\n");
    for (unsigned seed = 1; seed <= 8; seed++) {
        srand(seed);
        game_init(&game);
        castle_enter(&game, 1);
        ASSERT("the new gatehouse has ten distinct rooms and ten placed defenders", game.map.room_count == 10 && game.enemy_count == 10);
        int valid = 1;
        for (int i = 0; i < game.enemy_count; i++) {
            Enemy *e = &game.enemies[i];
            valid &= map_is_walkable(&game.map, e->x, e->y);
        }
        ASSERT("all defenders stand on usable tiles after encounter placement", valid);
        clear_enemies(&game);
        ASSERT("the entrance side of the service shortcut starts sealed", game.map.tiles[21][26] == TILE_CASTLE_GATE);
        int east_ok = castle_walk_to(&game, 24, 13);
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        east_ok &= castle_walk_to(&game, 10, 15);
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the east watch and gallery reach the upper stair while south and service gates are closed", east_ok && castle_walk_to(&game, 45, 55));
        ASSERT("the eastern approach retains access to the entrance levers", castle_walk_to(&game, 10, 15));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the entrance still reaches its east gate lever", castle_walk_to(&game, 24, 13));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        int rooms_ok = 1;
        for (int i = 0; i < game.map.room_count; i++) {
            int x;
            int y;
            map_room_center(&game.map.rooms[i], &x, &y);
            rooms_ok &= castle_walk_to(&game, x, y);
        }
        ASSERT("real movement reaches every room with the east and service gates closed", rooms_ok);
        ASSERT("the lower keep reaches its service lever", castle_walk_to(&game, GATEHOUSE_LEVER_X, GATEHOUSE_LEVER_Y));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the service lever permanently opens all three shortcut stones", game.map.tiles[21][25] == TILE_CASTLE_GATE_OPEN && game.map.tiles[21][26] == TILE_CASTLE_GATE_OPEN && game.map.tiles[21][27] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the open shortcut survives a current-format save and load", save_game(&game, GATEHOUSE_SLOT) && load_game(&loaded, GATEHOUSE_SLOT) && loaded.map.tiles[21][26] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the stores' optional supply can be reached", castle_walk_to(&game, 15, 52));
        int inventory = game.inventory_count;
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("the stores supply is collected once", game.inventory_count == inventory + 1 && !game.floor_items[0].active);
        ASSERT("the shortcut leads continuously back to the entrance stair", castle_walk_to(&game, 15, 11));
        ASSERT("both guarded approaches can close without trapping the player away from their levers", castle_walk_to(&game, 10, 15));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the opened shortcut remains usable while both guarded gates are closed", castle_walk_to(&game, 45, 55));
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("actual onward travel enters the alchemy Wing", game.location == LOCATION_CASTLE_INTERIOR && game.level == 2);
        clear_enemies(&game);
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("cached-floor retreat arrives on the correct gatehouse staircase", game.level == 1 && game.player.x == 45 && game.player.y == 55 && game.map.tiles[21][26] == TILE_CASTLE_GATE_OPEN);
        clear_enemies(&game);
        ASSERT("retreat remains a continuous route to Castle Grounds", castle_walk_to(&game, 15, 11));
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("the actual return stair exits to Castle Grounds", game.location == LOCATION_CASTLE);
        castle_enter(&game, 1);
        ASSERT("regrouping preserves the shortcut and does not replenish collected supplies", game.map.tiles[21][26] == TILE_CASTLE_GATE_OPEN && game.floor_item_count == 2 && !game.floor_items[0].active);
    }
    remove(GATEHOUSE_SAVE);
    test_castle_stair_routes();
    test_gatehouse_migration();
}
