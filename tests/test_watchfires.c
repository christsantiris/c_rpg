#include "test_utils.h"
#include "game/game.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define WATCHFIRE_SLOT 99163
#define WATCHFIRE_SAVE "saves/savegame_99163.json"
static GameState game;
static GameState loaded;
static Map original_map;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    srand(9163);
    game_enter_town4(&game);
}

static void approach_veyra(void) {
    game_enter_town4(&game);
    game_enter_town_hall(&game);
    game.player.x = HALL_VEYRA_X;
    game.player.y = HALL_VEYRA_Y + 1;
}

static void accept(void) {
    approach_veyra();
    game_talk_to_veyra(&game);
    game_handle_quest_offer_key(&game, SDL_SCANCODE_Y, 0);
    game_leave_town_hall(&game);
}

static void visit(Location area) {
    if (game.location != LOCATION_TOWN4) {
        game_return_to_town(&game);
        game_enter_town4(&game);
    }
    if (area == LOCATION_MOUNTAINS) {
        game_enter_mountains(&game);
        game_ascend(&game);
    } else {
        if (area == LOCATION_ASHEN) {
            game_enter_ashen(&game);
        } else {
            game_enter_dragonspine(&game);
        }
        game_descend(&game);
        game_descend(&game);
    }
}

static int position(const GameState *g, int *x, int *y) {
    for (int ty = 0; ty < MAP_H; ty++) {
        for (int tx = 0; tx < MAP_W; tx++) {
            TileType tile = g->map.tiles[ty][tx];
            if (tile == TILE_WATCHFIRE_COLD || tile == TILE_WATCHFIRE_LIT) {
                *x = tx;
                *y = ty;
                return 1;
            }
        }
    }
    return 0;
}

static int guards(const GameState *g, int active_only) {
    int count = 0;
    for (int i = 0; i < g->enemy_count; i++) {
        count += strcmp(g->enemies[i].name, "Watchfire Defender") == 0 && (!active_only || g->enemies[i].active);
    }
    return count;
}

static void stand_at_fire(void) {
    ASSERT("watchfire is present in the assigned region", position(&game, &game.player.x, &game.player.y));
}

static void clear_defenders(void) {
    for (int i = 0; i < game.enemy_count; i++) {
        Enemy *e = &game.enemies[i];
        if (strcmp(e->name, "Watchfire Defender") == 0 || abs(e->x - game.player.x) + abs(e->y - game.player.y) <= 4) {
            e->active = 0;
            e->hp = 0;
        }
    }
}

static void test_quest(void) {
    start();
    game_enter_town_hall(&game);
    ASSERT("Veyra occupies a reachable spot beside the planning table", game.map.tiles[HALL_VEYRA_Y][HALL_VEYRA_X] == TILE_NPC_VEYRA &&
        !map_is_walkable(&game.map, HALL_VEYRA_X, HALL_VEYRA_Y) && map_is_walkable(&game.map, HALL_VEYRA_X, HALL_VEYRA_Y + 1));
    game_talk_to_veyra(&game);
    ASSERT("Veyra cannot offer the quest remotely", !game.dialogue_active && !game.watchfire_quest_state);
    approach_veyra();
    game_talk_to_veyra(&game);
    ASSERT("quest requires an explicit acceptance", !game.watchfire_quest_state && game_quest_offer_active(&game));
    game_handle_quest_offer_key(&game, SDL_SCANCODE_N, 0);
    ASSERT("declining leaves watchfires unassigned", !game.watchfire_quest_state);
    accept();
    QuestJournalEntry entry;
    ASSERT("journal gives each objective its own region and stage", quest_journal_get_entry(&game, QUEST_TAB_ACTIVE, 0, &entry) &&
        entry.objective_count == 3 && entry.stages[0] == 6 && entry.stages[1] == 3 && entry.stages[2] == 3 &&
        strcmp(entry.objective_areas[0], "Goblin Mountains") == 0 && strcmp(entry.objective_areas[1], "Ashen Hollow") == 0 &&
        strcmp(entry.objective_areas[2], "Dragonspine") == 0);
    visit(LOCATION_DRAGONSPINE);
    stand_at_fire();
    ASSERT("Dragonspine receives a Giant, Archer, and Fire Elemental", guards(&game, 1) == 3 &&
        game.enemies[game.enemy_count - 3].type == ENEMY_GIANT && game.enemies[game.enemy_count - 2].type == ENEMY_GOBLIN_ARCHER &&
        game.enemies[game.enemy_count - 1].type == ENEMY_FIRE_ELEMENTAL && game_has_regional_interaction(&game));
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("living defenders prevent lighting", !game.watchfire_quest_progress);
    clear_defenders();
    Enemy *guard = &game.enemies[game.enemy_count - 1];
    guard->active = 1;
    guard->hp = 5;
    guard->x = game.map.stairs_up_x;
    guard->y = game.map.stairs_up_y;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("a defender lured away still prevents lighting", !game.watchfire_quest_progress);
    ASSERT("damaged living guards persist through save/load without duplication", save_game(&game, WATCHFIRE_SLOT) && load_game(&loaded, WATCHFIRE_SLOT) &&
        guards(&loaded, 0) == 3 && guards(&loaded, 1) == 1 && loaded.enemies[loaded.enemy_count - 1].hp == 5 && !loaded.watchfire_quest_progress);
    guard->active = 0;
    guard->hp = 0;
    game.inventory_count = MAX_INVENTORY;
    game_interact_watchfire(&game);
    game.inventory_count = 0;
    ASSERT("eastern watchfire can be restored first without inventory space", game.watchfire_quest_progress == 4 && game.watchfire_quest_state == 1 &&
        game.map.tiles[game.player.y][game.player.x] == TILE_WATCHFIRE_LIT);
    game_ascend(&game);
    game_descend(&game);
    stand_at_fire();
    ASSERT("backtracking retains the flame and defeated defenders", guards(&game, 0) == 3 && !guards(&game, 1) &&
        game.map.tiles[game.player.y][game.player.x] == TILE_WATCHFIRE_LIT);
    visit(LOCATION_MOUNTAINS);
    stand_at_fire();
    ASSERT("Mountains six receives four native defenders", game.level == 6 && guards(&game, 1) == 4 &&
        game.enemies[game.enemy_count - 4].type == ENEMY_GOBLIN_ARCHER && game.enemies[game.enemy_count - 1].type == ENEMY_HOBGOBLIN_GUARD);
    clear_defenders();
    Enemy *ordinary = &game.enemies[0];
    ordinary->active = 1;
    ordinary->hp = 5;
    ordinary->x = game.player.x + 1;
    ordinary->y = game.player.y;
    game_interact_watchfire(&game);
    ASSERT("nearby ordinary enemies also prevent repairs", game.watchfire_quest_progress == 4);
    clear_defenders();
    int x = game.player.x;
    int y = game.player.y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = TILE_WATCHFIRE_COLD, .item = item_make_short_sword()};
    game.map.tiles[y][x] = TILE_ITEM;
    action_resolve_player(&game, (Action){ACTION_INTERACT, 0, 0});
    ASSERT("loot over a watchfire remains intact while its underlay becomes lit", game.watchfire_quest_progress == 5 &&
        game.floor_items[0].active && game.floor_items[0].underlying_tile == TILE_WATCHFIRE_LIT && game.map.tiles[y][x] == TILE_ITEM);
    visit(LOCATION_ASHEN);
    stand_at_fire();
    ASSERT("Ashen three receives an Obsidian Guardian and two Imps", guards(&game, 1) == 3 &&
        game.enemies[game.enemy_count - 3].type == ENEMY_OBSIDIAN_GUARDIAN && game.enemies[game.enemy_count - 1].type == ENEMY_CINDER_IMP);
    clear_defenders();
    game_interact_watchfire(&game);
    ASSERT("all three regions complete without boss victories", game.watchfire_quest_state == 2 && game.watchfire_quest_progress == 7 && !game.defeated_bosses);
    ASSERT("ready quest survives saving", save_game(&game, WATCHFIRE_SLOT) && load_game(&loaded, WATCHFIRE_SLOT) &&
        loaded.watchfire_quest_state == 2 && loaded.watchfire_quest_progress == 7 && loaded.watchfire_quest_encounters == 7);
    game_open_town_portal(&game);
    approach_veyra();
    int gold = game.gold;
    int score = game.score;
    game_talk_to_veyra(&game);
    game_talk_to_veyra(&game);
    ASSERT("Veyra pays 180 gold and 1400 score exactly once", game.watchfire_quest_state == 3 && game.gold == gold + 180 && game.score == score + 1400);
    ASSERT("completed journal retains all region objectives", quest_journal_get_entry(&game, QUEST_TAB_COMPLETED, 0, &entry) &&
        entry.objective_complete[0] && entry.objective_complete[1] && entry.objective_complete[2]);
    ASSERT("claimed reward and portal survive saving", save_game(&game, WATCHFIRE_SLOT) && load_game(&loaded, WATCHFIRE_SLOT) &&
        loaded.watchfire_quest_state == 3 && loaded.gold == game.gold && loaded.portal_origin_tile == TILE_WATCHFIRE_LIT);
    game_use_town_portal(&game);
    ASSERT("portal return retains the restored beacon without respawning guards", game.location == LOCATION_ASHEN && game.level == 3 &&
        !guards(&game, 1) && game_has_watchfire_interaction(&game));
    game_init(&game);
    ASSERT("new games reset watchfire progress", !game.watchfire_quest_state && !game.watchfire_quest_progress && !game.watchfire_quest_encounters);
}

static void test_explored_area(void) {
    start();
    visit(LOCATION_MOUNTAINS);
    game.enemies[0].hp = 7;
    map_mark_explored(&game.map, game.player.x, game.player.y);
    original_map = game.map;
    game_open_town_portal(&game);
    accept();
    ASSERT("acceptance preserves explored maps, enemy damage, and the return portal", game.mountain_cache[5].valid &&
        game.mountain_cache[5].enemies[0].hp == 7 && game.portal_active && !game.watchfire_quest_encounters &&
        memcmp(&original_map, &game.mountain_cache[5].map, sizeof(Map)) == 0);
    game_use_town_portal(&game);
    ASSERT("previously explored Mountains six gains guards while preserving enemies", guards(&game, 1) == 4 && game.enemies[0].hp == 7);
    stand_at_fire();
    clear_defenders();
    game_open_town_portal(&game);
    game_use_town_portal(&game);
    ASSERT("watchfire at the portal's return position still supports interaction", game_has_regional_interaction(&game));
    game_interact_watchfire(&game);
    ASSERT("lighting after a portal return restores the watchfire", game.watchfire_quest_progress == 1 &&
        game.map.tiles[game.player.y][game.player.x] == TILE_WATCHFIRE_LIT);
    game_open_town_portal(&game);
    ASSERT("the next return portal preserves the lit watchfire", game.portal_origin_tile == TILE_WATCHFIRE_LIT &&
        game.mountain_cache[5].map.tiles[game.portal_y][game.portal_x] == TILE_WATCHFIRE_LIT);

    start();
    visit(LOCATION_MOUNTAINS);
    for (int i = 0; i < game.enemy_count; i++) {
        game.enemies[i].active = 0;
        game.enemies[i].hp = 0;
    }
    game_update_level_progress(&game);
    ASSERT("pre-quest mountain fixture is already cleared", game.level_cleared);
    game_open_town_portal(&game);
    accept();
    game_use_town_portal(&game);
    int active = 0;
    for (int i = 0; i < game.enemy_count; i++) {
        active += !!game.enemies[i].active;
    }
    ASSERT("a cleared stage gains only its four quest defenders without replenishing ordinary enemies", active == 4 &&
        guards(&game, 1) == 4 && !game.level_cleared && game.level == 6);
}

static int legacy_fixture(void) {
    if (!save_game(&game, WATCHFIRE_SLOT)) {
        return 0;
    }
    FILE *file = fopen(WATCHFIRE_SAVE, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 121);
    cJSON_DeleteItemFromObject(root, "watchfire_quest_state");
    cJSON_DeleteItemFromObject(root, "watchfire_quest_progress");
    cJSON_DeleteItemFromObject(root, "watchfire_quest_encounters");
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(WATCHFIRE_SAVE, "wb");
    int written = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return written;
}

static void test_migration(void) {
    start();
    visit(LOCATION_ASHEN);
    game.enemies[0].hp = 7;
    game_open_town_portal(&game);
    approach_veyra();
    game.player.y = HALL_VEYRA_Y;
    game.floor_item_count = 1;
    game.floor_items[0] = (FloorItem){.active = 1, .x = HALL_VEYRA_X, .y = HALL_VEYRA_Y,
        .underlying_tile = TILE_TAVERN_FLOOR, .item = item_make_short_sword()};
    game.map.tiles[HALL_VEYRA_Y][HALL_VEYRA_X] = TILE_ITEM;
    game.gold = 617;
    game.emberforge_quest_state = 1;
    game.emberforge_progress = 1;
    game.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
    ASSERT("version 121 saves gain an available watchfire quest", legacy_fixture() && load_game(&loaded, WATCHFIRE_SLOT) &&
        !loaded.watchfire_quest_state && !loaded.watchfire_quest_progress && !loaded.watchfire_quest_encounters);
    ASSERT("migration preserves other quests, gold, boss victories, explored enemies, and portals", loaded.gold == 617 &&
        loaded.emberforge_quest_state == 1 && loaded.emberforge_progress == 1 && loaded.portal_active &&
        loaded.ashen_cache[2].valid && loaded.ashen_cache[2].enemies[0].hp == 7 && (loaded.defeated_bosses & (1 << LOCATION_MOUNTAINS)));
    ASSERT("migration adds Veyra and safely moves overlapping player and loot", loaded.map.tiles[HALL_VEYRA_Y][HALL_VEYRA_X] == TILE_NPC_VEYRA &&
        loaded.player.y == HALL_VEYRA_Y + 1 && loaded.floor_items[0].y == HALL_VEYRA_Y + 1 && loaded.floor_items[0].active);
    game.watchfire_quest_state = 1;
    game.watchfire_quest_progress = 8;
    ASSERT("invalid watchfire progress is rejected", save_game(&game, WATCHFIRE_SLOT) && !load_game(&loaded, WATCHFIRE_SLOT));
}

static void test_access(void) {
    static unsigned char seen[MAP_H][MAP_W];
    static int queue[MAP_W * MAP_H];
    const Location areas[3] = {LOCATION_MOUNTAINS, LOCATION_ASHEN, LOCATION_DRAGONSPINE};
    const int offsets[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    int reachable = 1;
    int safe = 1;
    for (int seed = 0; seed < 32; seed++) {
        start();
        srand((unsigned int)seed);
        accept();
        for (int area = 0; area < 3; area++) {
            visit(areas[area]);
            int x;
            int y;
            if (!position(&game, &x, &y)) {
                reachable = 0;
                continue;
            }
            memset(seen, 0, sizeof(seen));
            int head = 0;
            int tail = 0;
            queue[tail++] = game.player.y * MAP_W + game.player.x;
            seen[game.player.y][game.player.x] = 1;
            while (head < tail) {
                int cell = queue[head++];
                for (int side = 0; side < 4; side++) {
                    int tx = cell % MAP_W + offsets[side][0];
                    int ty = cell / MAP_W + offsets[side][1];
                    if (tx < 0 || tx >= MAP_W || ty < 0 || ty >= MAP_H || seen[ty][tx]) {
                        continue;
                    }
                    TileType tile = game.map.tiles[ty][tx];
                    // Mountain gates and rockfalls can be opened with the normal A interaction.
                    if (!map_is_walkable(&game.map, tx, ty) && tile != TILE_MOUNTAIN_GATE && tile != TILE_MOUNTAIN_ROCKFALL) {
                        continue;
                    }
                    seen[ty][tx] = 1;
                    queue[tail++] = ty * MAP_W + tx;
                }
            }
            reachable &= seen[y][x];
            safe &= guards(&game, 1) == (area == 0 ? 4 : 3);
            for (int i = 0; i < game.enemy_count; i++) {
                const Enemy *e = &game.enemies[i];
                if (strcmp(e->name, "Watchfire Defender") == 0) {
                    safe &= seen[e->y][e->x] && (e->x != game.player.x || e->y != game.player.y);
                }
            }
        }
    }
    ASSERT("all three watchfires remain reachable across 32 generated expeditions", reachable);
    ASSERT("all watchfire defenders occupy reachable tiles away from arrivals", safe);
}

void test_watchfires(void) {
    printf("Watchfires of Ridgeshire tests:\n");
    int unused = !save_exists(WATCHFIRE_SLOT);
    ASSERT("watchfire test fixture does not overwrite a player save", unused);
    if (!unused) {
        return;
    }
    test_quest();
    test_explored_area();
    test_migration();
    test_access();
    remove(WATCHFIRE_SAVE);
}
