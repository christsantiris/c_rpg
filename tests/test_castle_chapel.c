#include "test_utils.h"
#include "castle_route_checks.h"
#include "game/castle.h"
#include "game/castle_chapel.h"
#include "game/royal_arcanist.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define CHAPEL_SLOT 99366
#define CHAPEL_SAVE "saves/savegame_99366.json"
static GameState game;
static GameState loaded;

static Enemy *boss(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        if (g->enemies[i].type == ENEMY_ROYAL_ARCANIST) {
            return &g->enemies[i];
        }
    }
    return NULL;
}

static void clear_enemies(GameState *g) {
    for (int i = 0; i < g->enemy_count; i++) {
        g->enemies[i].active = 0;
    }
}

static void setup(void) {
    game_init(&game);
    castle_enter(&game, 4);
}

static int legacy_version(void) {
    FILE *file = fopen(CHAPEL_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 255);
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = text ? fopen(CHAPEL_SAVE, "wb") : NULL;
    int ok = file != NULL;
    if (file) {
        fputs(text, file);
        fclose(file);
    }
    free(text);
    return ok;
}

static void test_chapel_migration(void) {
    for (int sleeping = 0; sleeping < 2; sleeping++) {
        setup();
        Map arena = game.map;
        castle_legacy_layout(&game.map, 4);
        for (int y = 44; y < 58; y++) {
            memcpy(&game.map.tiles[y][34], &arena.tiles[y][34], 22 * sizeof(TileType));
        }
        game.player.x = 45;
        game.player.y = 52;
        game.player.hp = 41;
        game.gold = 713;
        game.map.tiles[54][37] = TILE_CASTLE_WARD_SPENT;
        game.map.tiles[47][50] = TILE_CASTLE_GATE_OPEN;
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 53, .y = 54, .underlying_tile = TILE_CASTLE_WARD, .item = item_make_health_potion()};
        game.map.tiles[54][53] = TILE_ITEM;
        Enemy *e = boss(&game);
        e->hp = 237;
        e->attack_phase = 2;
        royal_arcanist_tick(&game, e);
        e->frozen_turns = 3;
        if (sleeping) {
            game.castle_minibosses |= 2;
            game.arcanist_reward_state = 2;
            e->active = 0;
            e->hp = 0;
        }
        game.map.burial_traps[0].spent = 1;
        game.map.tiles[32][17] = TILE_CASTLE_TRAP_OPEN;
        map_mark_explored(&game.map, 37, 54);
        map_mark_explored(&game.map, 15, 11);
        Map before = game.map;
        Enemy old = *e;
        castle_store(&game);
        if (sleeping) {
            game.location = LOCATION_TOWN;
            game.level = 1;
            game.enemy_count = 0;
            game.floor_item_count = 0;
            map_generate_town(&game.map, &game.player.x, &game.player.y);
        }
        int ok = save_game(&game, CHAPEL_SLOT) && legacy_version() && load_game(&loaded, CHAPEL_SLOT);
        ASSERT("active and cached legacy chapels gain ten rooms without changing player progress", ok && loaded.castle_cache[3].map.room_count == 10 && loaded.player.hp == 41 && loaded.gold == 713);
        if (!ok) {
            continue;
        }
        if (sleeping) {
            castle_enter(&loaded, 4);
        }
        int identical = 1;
        for (int y = 44; y < 58; y++) {
            identical &= !memcmp(&loaded.map.tiles[y][34], &before.tiles[y][34], 22 * sizeof(TileType));
            identical &= !memcmp(&loaded.castle_cache[3].map.tiles[y][34], &before.tiles[y][34], 22 * sizeof(TileType));
        }
        ASSERT("migration preserves the exact active and cached arena including wards, cover and gates", identical);
        e = boss(&loaded);
        ASSERT("the saved fight retains wounds, phase, warning mask, timers and freeze", e && e->hp == old.hp && e->active == old.active &&
            e->attack_phase == old.attack_phase && e->boss_attack == old.boss_attack && e->boss_aux_x == old.boss_aux_x &&
            e->boss_aux_y == old.boss_aux_y && e->revive_timer == old.revive_timer && e->move_timer == old.move_timer && e->frozen_turns == 3);
        ASSERT("exploration, spent trapdoors and loot-covered wards survive migration", !memcmp(loaded.map.explored, before.explored, sizeof(before.explored)) &&
            loaded.map.burial_traps[0].spent && loaded.map.tiles[32][17] == TILE_CASTLE_TRAP_OPEN && loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_CASTLE_WARD);
        if (!sleeping) {
            int timer = e->move_timer;
            castle_tick(&loaded);
            ASSERT("a resumed frozen preparation remains paused on the rebuilt floor", e->move_timer == timer && e->boss_aux_x == old.boss_aux_x && e->frozen_turns == 2);
        } else {
            ASSERT("historical victories and claimed vestments remain complete without another payout", (loaded.castle_minibosses & 2) && loaded.arcanist_reward_state == 2 && loaded.gold == 713);
        }
        clear_enemies(&loaded);
        ASSERT("migrated arena occupants retain continuous retreat to the actual entrance stair", castle_walk_to(&loaded, 15, 11));
        ASSERT("the migrated floor survives a current-format save round trip", save_game(&loaded, CHAPEL_SLOT) && load_game(&game, CHAPEL_SLOT) && game.map.room_count == 10);
    }
    remove(CHAPEL_SAVE);
}

static void test_chapel_combat(void) {
    setup();
    Enemy *e = boss(&game);
    for (int i = 0; i < game.enemy_count; i++) {
        if (&game.enemies[i] != e) {
            game.enemies[i].active = 0;
        }
    }
    game.player.x = 36;
    game.player.y = 54;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    game.player.x = 52;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("real interaction disables both sanctuary wards in the rebuilt chapel", game.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && game.map.tiles[54][53] == TILE_CASTLE_WARD_SPENT);
    e->hp = 1;
    game.player.x = 44;
    game.player.y = 49;
    game.player.attack = 1000;
    action_resolve_player(&game, (Action){ACTION_MOVE, 45, 49});
    ASSERT("actual combat retains the Arcanist victory, gold and reserved vestments", !e->active && (game.castle_minibosses & 2) && game.gold == 100 && game.arcanist_reward_state == 1 && !game.game_won);
    ASSERT("the reserved reward chest remains physically reachable", castle_walk_to(&game, 53, 56));
    ASSERT("the sacristy lever remains reachable after the fight", castle_walk_to(&game, CHAPEL_LEVER_X, CHAPEL_LEVER_Y));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = 26, .y = 39, .underlying_tile = TILE_CASTLE_GATE_OPEN, .item = item_make_health_potion()};
    game.map.tiles[39][26] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    castle_store(&game);
    ASSERT("loot, spent wards, shortcut and reserved reward survive save/load", save_game(&game, CHAPEL_SLOT) && load_game(&loaded, CHAPEL_SLOT) &&
        loaded.floor_items[0].active && loaded.floor_items[0].underlying_tile == TILE_CASTLE_GATE_OPEN && loaded.map.tiles[39][26] == TILE_ITEM &&
        loaded.map.tiles[54][37] == TILE_CASTLE_WARD_SPENT && loaded.arcanist_reward_state == 1);
    ASSERT("the loot-covered shortcut remains usable", castle_walk_to(&loaded, 26, 39));
    action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("collecting the drop leaves the service passage open", !loaded.floor_items[0].active && loaded.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN);
    ASSERT("the chapel register remains reachable", castle_walk_to(&loaded, 12, 27));
    action_resolve_player(&loaded, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("the register explains Malakar's crown orders without assigning a quest", strstr(loaded.messages[loaded.message_count - 2], "Malakar ordered the crown"));
    remove(CHAPEL_SAVE);
}

void test_castle_chapel(void) {
    printf("Castle V2 Crown Chapel tests:\n");
    for (unsigned seed = 1; seed <= 8; seed++) {
        srand(seed);
        setup();
        int placed = game.enemy_count == 12 && boss(&game) != NULL;
        for (int i = 0; i < game.enemy_count; i++) {
            placed &= map_is_walkable(&game.map, game.enemies[i].x, game.enemies[i].y);
        }
        ASSERT("twelve defenders and the established encounter fit the ten-room chapel", placed && game.map.room_count == 10);
        clear_enemies(&game);
        int rooms = 1;
        for (int i = 0; i < game.map.room_count; i++) {
            int x;
            int y;
            map_room_center(&game.map.rooms[i], &x, &y);
            rooms &= castle_walk_to(&game, x, y);
        }
        ASSERT("all chapel rooms are usable with the service passage initially sealed", rooms);
        // Remove the central nave to prove both side approaches are real routes.
        Map original = game.map;
        for (int y = 24; y < 37; y++) {
            for (int x = 24; x < 54; x++) {
                game.map.tiles[y][x] = TILE_CASTLE_WALL;
            }
        }
        ASSERT("the cloister route reaches the arena without entering the nave", castle_walk_to(&game, 15, 11) && castle_walk_to(&game, 13, 31) && castle_walk_to(&game, 15, 50) && castle_walk_to(&game, 45, 55));
        ASSERT("the vestry approach reaches the entrance lever", castle_walk_to(&game, 24, 13));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the choir gallery provides an eastern approach to the arena", castle_walk_to(&game, 42, 11) && castle_walk_to(&game, 59, 30) && castle_walk_to(&game, 45, 55));
        game.map = original;
        for (int state = 0; state < 4; state++) {
            for (int y = 47; y <= 49; y++) {
                game.map.tiles[y][40] = state & 1 ? TILE_CASTLE_GATE : TILE_CASTLE_GATE_OPEN;
                game.map.tiles[y][50] = state & 2 ? TILE_CASTLE_GATE : TILE_CASTLE_GATE_OPEN;
            }
            ASSERT("all supported arena gate states retain actual access to both stairs", castle_walk_to(&game, 15, 11) && castle_walk_to(&game, 45, 55));
        }
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the Arcanist checkpoint still identifies and blocks onward travel", game.level == 4);
        ASSERT("the sacristy lever is reachable without a boss victory", castle_walk_to(&game, CHAPEL_LEVER_X, CHAPEL_LEVER_Y));
        action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
        ASSERT("the royal service shortcut opens a continuous retreat", game.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN && game.map.tiles[39][27] == TILE_CASTLE_GATE_OPEN && castle_walk_to(&game, 15, 11));
        game.castle_minibosses |= 2;
        ASSERT("the onward staircase remains reachable after victory", castle_walk_to(&game, 45, 55));
        action_resolve_player(&game, (Action){ACTION_ASCEND, 0, 0});
        ASSERT("the actual onward stair enters the Royal Archives", game.level == 5);
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("cached retreat keeps the shortcut and correct arrival staircase", game.level == 4 && game.player.x == 45 && game.player.y == 55 && game.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN);
        ASSERT("the return stair is continuously reachable", castle_walk_to(&game, 15, 11));
        action_resolve_player(&game, (Action){ACTION_DESCEND, 0, 0});
        ASSERT("actual retreat enters the Court and Household", game.level == 3);
        castle_leave(&game, 0);
        castle_enter(&game, 4);
        ASSERT("defender regrouping retains the permanent royal service shortcut", game.map.tiles[39][26] == TILE_CASTLE_GATE_OPEN);
    }
    test_chapel_migration();
    test_chapel_combat();
}

