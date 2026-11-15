#include "test_utils.h"
#include "castle_route_checks.h"
#include "game/castle.h"
#include "game/castle_apartments.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#define APARTMENTS_SLOT 99368
#define APARTMENTS_SAVE "saves/savegame_99368.json"
static GameState game;
static GameState loaded;
static void setup(void) {
    game_init(&game);
    game.castle_minibosses = 3;
    castle_enter(&game, 5);
}
static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}
static int rune(const GameState *g, int x, int y) {
    if (g->map.tiles[y][x] == TILE_CASTLE_FIRE_RUNE) {
        return 1;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        const FloorItem *f = &g->floor_items[i];
        if (f->active && f->x == x && f->y == y && f->underlying_tile == TILE_CASTLE_FIRE_RUNE) {
            return 1;
        }
    }
    return 0;
}
static int legacy_version(void) {
    FILE *file = fopen(APARTMENTS_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 256);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = text ? fopen(APARTMENTS_SAVE, "wb") : NULL;
    int ok = file != NULL;
    if (file) {
        fputs(text, file);
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_apartment_migration(void) {
    for (int sleeping = 0; sleeping < 2; sleeping++) {
        setup();
        castle_legacy_layout(&game.map, 5);
        for (int i = 1; i < 5; i++) {
            int x = 4 + (i % 2) * 30;
            int y = 4 + (i / 2) * 20;
            for (int dx = 7; dx <= 15; dx++) {
                game.map.tiles[y + 10][x + dx] = TILE_CASTLE_FIRE_RUNE;
            }
        }
        game.player.x = 11;
        game.player.y = 54;
        game.player.hp = 41;
        game.gold = 713;
        game.arcanist_reward_state = 2;
        game.enemies[0].hp = 17;
        game.enemies[1].hp = 0;
        game.enemies[1].active = 0;
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 41, .y = 14, .underlying_tile = TILE_CASTLE_FIRE_RUNE, .item = item_make_health_potion()};
        game.map.tiles[14][41] = TILE_ITEM;
        game.castle_fire_phase[4] = 2;
        game.map.burial_traps[0].spent = 1;
        game.map.tiles[32][17] = TILE_CASTLE_TRAP_OPEN;
        map_mark_explored(&game.map, 11, 54);
        unsigned char explored[MAP_EXPLORED_BYTES];
        memcpy(explored, game.map.explored, sizeof(explored));
        castle_store(&game);
        if (sleeping) {
            game.location = LOCATION_TOWN;
            game.level = 1;
            game.enemy_count = 0;
            game.floor_item_count = 0;
            map_generate_town(&game.map, &game.player.x, &game.player.y);
        }
        int ok = save_game(&game, APARTMENTS_SLOT) && legacy_version() && load_game(&loaded, APARTMENTS_SLOT);
        ASSERT("active and sleeping Archives migrate without resetting progress", ok && loaded.castle_cache[4].map.room_count == 10 && loaded.player.hp == 41 && loaded.gold == 713 && loaded.arcanist_reward_state == 2);
        if (!ok) {
            continue;
        }
        if (sleeping) {
            castle_enter(&loaded, 5);
        } else {
            ASSERT("a player on an old fire rune keeps their exact position", loaded.player.x == 11 && loaded.player.y == 54);
        }
        int footprint = 1;
        for (int y = 0; y < CASTLE_H; y++) {
            for (int x = 0; x < CASTLE_W; x++) {
                int expected = (y == 14 && x >= 41 && x <= 49) || (y == 34 && ((x >= 11 && x <= 19) || (x >= 41 && x <= 49))) || (y == 54 && x >= 11 && x <= 19);
                footprint &= rune(&loaded, x, y) == expected;
            }
        }
        ASSERT("migration retains the exact warned fire footprint and clock without new hazards", footprint && loaded.castle_fire_phase[4] == 2 && castle_fire_warning(&loaded, 41, 14));
        ASSERT("wounds, deaths, exploration and old loot persist without fresh supply grants", loaded.enemies[0].hp == 17 && !loaded.enemies[1].active &&
            !memcmp(explored, loaded.map.explored, sizeof(explored)) && !memcmp(explored, loaded.castle_cache[4].map.explored, sizeof(explored)) &&
            loaded.floor_item_count == 1 && loaded.castle_loot_count[4] == 1 && loaded.floor_items[0].underlying_tile == TILE_CASTLE_FIRE_RUNE && loaded.map.burial_traps[0].spent);
        clear_enemies(&loaded);
        ASSERT("the old fire lane remains connected after remodeling", castle_walk_to(&loaded, 11, 54));
        int hp = loaded.player.hp;
        action_resolve_enemies(&loaded);
        ASSERT("a restored warning resolves through the real damage handler at its saved timing", loaded.castle_fire_phase[4] == 3 && loaded.player.hp < hp);
        ASSERT("migrated positions retain routes to both actual stairs", castle_walk_to(&loaded, 15, 11) && castle_walk_to(&loaded, 45, 55));
        ASSERT("the migrated layout and hazard state survive a current save round trip", save_game(&loaded, APARTMENTS_SLOT) && load_game(&game, APARTMENTS_SLOT) && game.map.room_count == 10 && game.floor_item_count == 1);
    }
    remove(APARTMENTS_SAVE);
}

static void test_apartment_fire(void) {
    setup();
    clear_enemies(&game);
    game.player.x = 45;
    game.player.y = 42;
    int hp = game.player.hp;
    action_resolve_enemies(&game);
    ASSERT("fresh apartment runes start with a two-action warning", game.castle_fire_phase[4] == 1 && castle_fire_warning(&game, 45, 42) && game.player.hp == hp);
    action_resolve_enemies(&game);
    ASSERT("the second warning action still causes no damage", game.castle_fire_phase[4] == 2 && game.player.hp == hp);
    action_resolve_player(&game, (Action){ACTION_MOVE, 45, 41});
    action_resolve_enemies(&game);
    ASSERT("one real perpendicular step dodges the fire eruption", game.castle_fire_phase[4] == 3 && game.player.hp == hp);
    ASSERT("both stair tiles remain free of fire hazards", !rune(&game, 15, 11) && !rune(&game, 45, 55));
}

void test_castle_apartments(void) {
    printf("Castle V2 Royal Apartments tests:\n");
    for (unsigned seed = 1; seed <= 8; seed++) {
        srand(seed);
        setup();
        int placed = game.enemy_count == 13;
        for (int i = 0; i < game.enemy_count; i++) {
            placed &= map_is_walkable(&game.map, game.enemies[i].x, game.enemies[i].y);
        }
        ASSERT("thirteen defenders occupy usable tiles in ten distinct apartment spaces", placed && game.map.room_count == 10);
        clear_enemies(&game);
        int rooms = 1;
        for (int i = 0; i < game.map.room_count; i++) {
            int x;
            int y;
            map_room_center(&game.map.rooms[i], &x, &y);
            rooms &= castle_walk_to(&game, x, y);
        }
        ASSERT("all rooms are explorable with the treasury and household gates closed", rooms);
        ASSERT("the bedroom route reaches the finale stair through the connected guest suite", castle_walk_to(&game, 15, 11) && castle_walk_to(&game, 14, 33) && castle_walk_to(&game, 14, 52) && castle_walk_to(&game, 31, 52) && castle_walk_to(&game, 45, 55));
        ASSERT("the treasury's entrance lever remains reachable", castle_walk_to(&game, 24, 13));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the eastern archive route reaches the finale stair", castle_walk_to(&game, 45, 11) && castle_walk_to(&game, 41, 31) && castle_walk_to(&game, 59, 33) && castle_walk_to(&game, 45, 55));
        ASSERT("optional treasury supplies remain accessible", castle_walk_to(&game, 48, 13));
        int inventory = game.inventory_count;
        action_resolve_player(&game, (Action){ACTION_PICK_UP, 0, 0});
        ASSERT("the treasury grants its Strength Potion once", game.inventory_count == inventory + 1 && !game.floor_items[1].active);
        ASSERT("the household lever is reachable from the recovery suite", castle_walk_to(&game, APARTMENTS_LEVER_X, APARTMENTS_LEVER_Y));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the shortcut opens continuously to the receiving hall", game.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN && game.map.tiles[39][27] == TILE_CASTLE_GATE_OPEN && castle_walk_to(&game, 15, 11));
        castle_store(&game);
        ASSERT("supplies and shortcut persist through save/load", save_game(&game, APARTMENTS_SLOT) && load_game(&loaded, APARTMENTS_SLOT) && loaded.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN && !loaded.floor_items[1].active);
        ASSERT("the real finale stair remains reachable", castle_walk_to(&game, 45, 55));
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the onward stair actually enters the existing finale", game.level == 6);
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("cached retreat preserves the correct apartment staircase and shortcut", game.level == 5 && game.player.x == 45 && game.player.y == 55 && game.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the return stair is continuously reachable", castle_walk_to(&game, 15, 11));
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("the actual return stair enters Crown Chapel", game.level == 4);
        castle_leave(&game, 0);
        castle_enter(&game, 5);
        ASSERT("regrouping preserves the shortcut and cannot replenish the treasury", game.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN && game.floor_item_count == 3 && !game.floor_items[1].active);
        clear_enemies(&game);
        ASSERT("Malakar's recovery ledger is reachable", castle_walk_to(&game, 31, 45));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the ledger establishes a fully recovered Malakar without a collection gate", strstr(game.messages[game.message_count - 2], "full strength") && !game.game_won);
        ASSERT("the royal bedside orders are reachable", castle_walk_to(&game, 12, 28));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("bedside orders explain the King's isolation", strstr(game.messages[game.message_count - 1], "attendants were dismissed"));
        ASSERT("the crown diagrams are reachable among the archive shelves", castle_walk_to(&game, 40, 27));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the diagrams identify the throne chamber conduits", strstr(game.messages[game.message_count - 1], "conduits lead to the throne chamber"));
    }
    test_apartment_migration();
    test_apartment_fire();
    remove(APARTMENTS_SAVE);
}

