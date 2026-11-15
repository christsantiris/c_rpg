#include "test_utils.h"
#include "castle_route_checks.h"
#include "game/castle.h"
#include "game/castle_court.h"
#include "game/royal_evidence.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define COURT_SLOT 99364
#define COURT_SAVE "saves/savegame_99364.json"

static GameState game;
static GameState loaded;

static void setup(void) {
    game_init(&game);
    game.castle_minibosses = 3;
    castle_enter(&game, 3);
}

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static int chamberlain(const Map *m, int *x, int *y) {
    int count = 0;
    for (int ty = 0; ty < CASTLE_H; ty++) {
        for (int tx = 0; tx < CASTLE_W; tx++) {
            if (m->tiles[ty][tx] == TILE_CASTLE_CHAMBERLAIN) {
                *x = tx;
                *y = ty;
                count++;
            }
        }
    }
    return count;
}

static int legacy_version(void) {
    FILE *file = fopen(COURT_SAVE, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *text = malloc(length + 1);
    if (!text) {
        fclose(file);
        return 0;
    }
    size_t read = fread(text, 1, length, file);
    text[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 254);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = text ? fopen(COURT_SAVE, "wb") : NULL;
    int ok = file != NULL;
    if (file) {
        fputs(text, file);
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_household_dialogue(void) {
    setup();
    clear_enemies(&game);
    ASSERT("the chamberlain is a friendly solid NPC rather than a hostile actor", !map_is_walkable(&game.map, 54, 25));
    ASSERT("the chamberlain's office can be reached without regional evidence", castle_walk_to(&game, 53, 25) && court_talk(&game, 54, 25) &&
        court_dialogue(&game) && strstr(game.dialogue_text, "western service rooms"));
    int px = game.player.x;
    int hp = game.player.hp;
    ASSERT("conversation consumes movement keys without advancing the encounter", court_handle_key(&game, SDL_SCANCODE_LEFT, 0) &&
        game.player.x == px && game.player.hp == hp && game.dialogue_active);
    castle_store(&game);
    ASSERT("the new NPC tile and existing dialogue fields survive save/load", save_game(&game, COURT_SLOT) && load_game(&loaded, COURT_SLOT) &&
        loaded.map.tiles[25][54] == TILE_CASTLE_CHAMBERLAIN && loaded.castle_cache[2].map.tiles[25][54] == TILE_CASTLE_CHAMBERLAIN &&
        loaded.dialogue_active && !strcmp(loaded.dialogue_speaker, "Chamberlain Edras") && !strcmp(loaded.dialogue_text, game.dialogue_text));
    ASSERT("Enter closes a resumed conversation", court_handle_key(&loaded, SDL_SCANCODE_RETURN, 0) && !court_dialogue(&loaded));
    court_handle_key(&game, SDL_SCANCODE_ESCAPE, 0);
    game.defeated_bosses |= 1 << LOCATION_DRAGONSPINE;
    ASSERT("Ridgeshire evidence adds context without assigning a quest or reward", court_talk(&game, 54, 25) &&
        strstr(game.dialogue_text, "Your records explain") && !game.gold);
    court_handle_key(&game, SDL_SCANCODE_ESCAPE, 0);
    game.defeated_bosses = 1 << LOCATION_FROSTFELL;
    ASSERT("Rosemoor evidence has its own household interpretation", court_talk(&game, 54, 25) && strstr(game.dialogue_text, "Rosemoor"));
    ASSERT("Escape closes the conversation without changing evidence", court_handle_key(&game, SDL_SCANCODE_ESCAPE, 0) && !game.dialogue_active &&
        game.defeated_bosses == (1 << LOCATION_FROSTFELL));
    ASSERT("the council orders can be read through the real interaction action", castle_walk_to(&game, 29, 7));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("council records identify Malakar's restrictions without mandatory collection", strstr(game.messages[game.message_count - 2], "Malakar's office") && !game.gold);
    remove(COURT_SAVE);
}

static void test_gate_loot(void) {
    setup();
    clear_enemies(&game);
    ASSERT("the service rooms reach the household lever with combat cleared", castle_walk_to(&game, COURT_LEVER_X, COURT_LEVER_Y));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = COURT_GATE_X, .y = COURT_GATE_Y,
        .underlying_tile = TILE_CASTLE_GATE_OPEN, .item = item_make_health_potion()};
    game.map.tiles[COURT_GATE_Y][COURT_GATE_X] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("repeated lever use leaves dropped loot and the opened gate intact", game.floor_items[0].active &&
        game.map.tiles[COURT_GATE_Y][COURT_GATE_X] == TILE_ITEM && game.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN && !game.gold);
    castle_store(&game);
    ASSERT("loot-covered gates retain their state in active and cached save records", save_game(&game, COURT_SLOT) && load_game(&loaded, COURT_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN &&
        loaded.castle_loot[2][0].active && loaded.castle_loot[2][0].underlying_tile == TILE_CASTLE_GATE_OPEN);
    ASSERT("the loot-covered shortcut remains walkable", castle_walk_to(&loaded, COURT_GATE_X, COURT_GATE_Y));
    action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up the item exposes the still-open gate", !loaded.floor_items[0].active && loaded.map.tiles[COURT_GATE_Y][COURT_GATE_X] == TILE_CASTLE_GATE_OPEN);
    castle_leave(&loaded, 0);
    castle_enter(&loaded, 3);
    ASSERT("regrouping cannot replenish the collected item or close its service passage", !loaded.floor_items[0].active && loaded.map.tiles[COURT_GATE_Y][COURT_GATE_X] == TILE_CASTLE_GATE_OPEN);
    remove(COURT_SAVE);
}

static void test_court_migration(void) {
    for (int overlap = 0; overlap < 4; overlap++) {
        setup();
        castle_legacy_layout(&game.map, 3);
        game.player.x = overlap == 0 ? 54 : 25;
        game.player.y = overlap == 0 ? 25 : 17;
        game.player.hp = 41;
        game.gold = 713;
        game.elowen_quest_state = 3;
        for (int i = 0; i < game.enemy_count; i++) {
            game.enemies[i].x = 35 + i * 2;
            game.enemies[i].y = 28;
        }
        game.enemies[0].x = overlap == 1 ? 54 : 35;
        game.enemies[0].y = overlap == 1 ? 25 : 28;
        game.enemies[0].hp = 17;
        game.enemies[1].active = 0;
        game.enemies[1].hp = 0;
        game.floor_item_count = 1;
        int lx = overlap == 2 ? 54 : 24;
        int ly = overlap == 2 ? 25 : 16;
        game.floor_items[0] = (FloorItem){.active = 1, .x = lx, .y = ly, .underlying_tile = TILE_CASTLE_FLOOR, .item = item_make_health_potion()};
        game.map.tiles[ly][lx] = TILE_ITEM;
        game.map.burial_traps[0].spent = 1;
        game.map.tiles[32][17] = TILE_CASTLE_TRAP_OPEN;
        map_mark_explored(&game.map, 15, 11);
        map_mark_explored(&game.map, 54, 25);
        unsigned char explored[MAP_EXPLORED_BYTES];
        memcpy(explored, game.map.explored, sizeof(explored));
        castle_store(&game);
        if (overlap == 3) {
            game.location = LOCATION_TOWN;
            game.level = 1;
            game.enemy_count = 0;
            game.floor_item_count = 0;
            map_generate_town(&game.map, &game.player.x, &game.player.y);
        }
        int ok = save_game(&game, COURT_SLOT) && legacy_version() && load_game(&loaded, COURT_SLOT);
        ASSERT("legacy active and sleeping cached courts migrate without resetting the run", ok && loaded.castle_cache[2].map.room_count == 10 &&
            loaded.player.hp == 41 && loaded.gold == 713 && loaded.castle_minibosses == 3 && loaded.elowen_quest_state == 3);
        int x = -1;
        int y = -1;
        ASSERT("migration retains exactly one reachable chamberlain in the cached household", ok && chamberlain(&loaded.castle_cache[2].map, &x, &y) == 1);
        if (!ok) {
            continue;
        }
        if (overlap == 3) {
            castle_enter(&loaded, 3);
        } else {
            ASSERT("migration preserves the exact active player position", loaded.player.x == game.player.x && loaded.player.y == game.player.y);
        }
        ASSERT("enemy wounds and deaths persist in both active and cached records", loaded.enemies[0].hp == 17 &&
            !loaded.enemies[1].active && !loaded.enemies[1].hp && loaded.castle_cache[2].enemies[0].hp == 17 &&
            loaded.enemies[0].x == game.castle_cache[2].enemies[0].x && loaded.enemies[0].y == game.castle_cache[2].enemies[0].y);
        ASSERT("old exploration, dropped loot and spent trapdoors remain intact", !memcmp(explored, loaded.map.explored, sizeof(explored)) &&
            !memcmp(explored, loaded.castle_cache[2].map.explored, sizeof(explored)) &&
            loaded.floor_item_count == 1 && loaded.floor_items[0].x == lx && loaded.floor_items[0].y == ly &&
            loaded.map.tiles[ly][lx] == TILE_ITEM && loaded.map.burial_traps[0].spent && loaded.map.tiles[32][17] == TILE_CASTLE_TRAP_OPEN);
        ASSERT("the chamberlain moves around existing occupants instead of moving or erasing them", chamberlain(&loaded.map, &x, &y) == 1 &&
            loaded.map.tiles[loaded.player.y][loaded.player.x] != TILE_CASTLE_CHAMBERLAIN &&
            loaded.map.tiles[loaded.enemies[0].y][loaded.enemies[0].x] != TILE_CASTLE_CHAMBERLAIN && loaded.map.tiles[ly][lx] == TILE_ITEM);
        clear_enemies(&loaded);
        ASSERT("the relocated chamberlain remains reachable and usable", castle_walk_to(&loaded, x, y + 1) && court_talk(&loaded, x, y));
        court_handle_key(&loaded, SDL_SCANCODE_ESCAPE, 0);
        ASSERT("migrated old positions retain a continuous route to both actual stair tiles", castle_walk_to(&loaded, 15, 11) && castle_walk_to(&loaded, 45, 55));
        action_resolve_player(&loaded, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the migrated court really enters the Crown Chapel", loaded.level == 4);
        action_resolve_player(&loaded, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("cached retreat reenters the migrated court at its upper stair", loaded.level == 3 && loaded.player.x == 45 && loaded.player.y == 55);
        ASSERT("migrated maps and loot survive a current-format round trip", save_game(&loaded, COURT_SLOT) && load_game(&game, COURT_SLOT) && game.map.room_count == 10 && game.floor_items[0].active);
    }
    remove(COURT_SAVE);
}

void test_castle_court(void) {
    printf("Castle V2 Court and Household tests:\n");
    for (unsigned seed = 1; seed <= 8; seed++) {
        srand(seed);
        setup();
        int placed = game.enemy_count == 10;
        for (int i = 0; i < game.enemy_count; i++) {
            placed &= map_is_walkable(&game.map, game.enemies[i].x, game.enemies[i].y);
        }
        ASSERT("ten court defenders occupy usable tiles around a distinct ten-room household", placed && game.map.room_count == 10);
        clear_enemies(&game);
        Map original = game.map;
        for (int y = 23; y <= 40; y++) {
            for (int x = 22; x <= 43; x++) {
                game.map.tiles[y][x] = TILE_CASTLE_WALL;
            }
        }
        ASSERT("the western service route reaches the upper stair without entering the Great Hall", castle_walk_to(&game, 11, 33) &&
            castle_walk_to(&game, 13, 52) && castle_walk_to(&game, 29, 52) && castle_walk_to(&game, 45, 55));
        ASSERT("the eastern guest route also bypasses the Great Hall", castle_walk_to(&game, 15, 11) && castle_walk_to(&game, 54, 11) &&
            castle_walk_to(&game, 53, 29) && castle_walk_to(&game, 53, 43) && castle_walk_to(&game, 45, 55));
        game.map = original;
        int connected = 1;
        for (int i = 0; i < game.map.room_count; i++) {
            int x;
            int y;
            map_room_center(&game.map.rooms[i], &x, &y);
            connected &= castle_walk_to(&game, x, y);
        }
        ASSERT("real movement reaches every room with the household gate still closed", connected && game.map.tiles[COURT_GATE_Y][COURT_GATE_X] == TILE_CASTLE_GATE);
        ASSERT("the household lever is reachable from within the service rooms", castle_walk_to(&game, COURT_LEVER_X, COURT_LEVER_Y));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the household lever opens a continuous shortcut back to the receiving hall", game.map.tiles[43][19] == TILE_CASTLE_GATE_OPEN &&
            game.map.tiles[43][20] == TILE_CASTLE_GATE_OPEN && castle_walk_to(&game, 15, 11));
        ASSERT("the actual chapel stair remains reachable", castle_walk_to(&game, 45, 55));
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the onward stair actually enters the Crown Chapel", game.level == 4);
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("cached retreat keeps the opened shortcut and correct arrival stair", game.level == 3 && game.player.x == 45 && game.player.y == 55 && game.map.tiles[43][19] == TILE_CASTLE_GATE_OPEN);
        castle_store(&game);
        ASSERT("the service shortcut survives save/load", save_game(&game, COURT_SLOT) && load_game(&loaded, COURT_SLOT) && loaded.map.tiles[43][19] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the receiving stair retains a continuous retreat route", castle_walk_to(&game, 15, 11));
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("the retreat stair really enters the alchemy wing", game.level == 2);
        castle_leave(&game, 0);
        castle_enter(&game, 3);
        int x;
        int y;
        ASSERT("regrouping retains the service shortcut and friendly household resident", game.map.tiles[43][19] == TILE_CASTLE_GATE_OPEN && chamberlain(&game.map, &x, &y) == 1);
    }
    test_household_dialogue();
    test_gate_loot();
    test_court_migration();
    remove(COURT_SAVE);
}
