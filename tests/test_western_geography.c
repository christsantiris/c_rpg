#include "expedition_fixture.h"
#include "test_utils.h"
#include "game/giants_crown.h"
#include "game/approach.h"
#include "game/town_road.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define GEOGRAPHY_SLOT 99307
#define GEOGRAPHY_SAVE "saves/savegame_99307.json"

static cJSON *read_geography_save(void) {
    FILE *file = fopen(GEOGRAPHY_SAVE, "rb");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc(size + 1, 1);
    if (!data) {
        fclose(file);
        return NULL;
    }
    fread(data, 1, size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    return root;
}

static int write_geography_save(cJSON *root) {
    char *data = cJSON_PrintUnformatted(root);
    FILE *file = fopen(GEOGRAPHY_SAVE, "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    cJSON_Delete(root);
    return ok;
}

static cJSON *legacy_geography_save(const GameState *g) {
    if (!save_game(g, GEOGRAPHY_SLOT)) {
        return NULL;
    }
    cJSON *root = read_geography_save();
    if (root) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 196);
    test_mark_legacy_portal(root);
        cJSON *recovery = cJSON_GetObjectItem(root, "retired_coast_loot");
        cJSON_DeleteItemFromArray(recovery, 5);
    }
    return root;
}

void test_western_geography(void) {
    ASSERT("geography migration slot is unused", !save_exists(GEOGRAPHY_SLOT));
    if (save_exists(GEOGRAPHY_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("geography fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    TownApproach route;
    ASSERT("Rosemoor west reaches Giant's Peaks while Ridgeshire east reaches Ironfang",
        town_road_connection(LOCATION_TOWN3, TOWN_SIDE_WEST, &route) && route.area == LOCATION_GIANTS_CROWN &&
        town_road_connection(LOCATION_TOWN4, TOWN_SIDE_EAST, &route) && route.area == LOCATION_IRONFANG && !approach_enter_region(g, 10));
    ASSERT("removing the garden leaves surviving serialized IDs unchanged", LOCATION_ASHEN == 25 &&
        TILE_ASHEN_FLOOR == 160 && ENEMY_CINDER_IMP == 74 && ENEMY_STONE_GIANT == 5);

    game_enter_town3(g);
    game_enter_ashen(g);
    g->player.hp = 77;
    g->gold = 431;
    g->emberforge_quest_state = 1;
    g->emberforge_progress = 1;
    g->defeated_bosses = (1 << 24) | (1 << LOCATION_DUNGEON);
    g->inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 12, .y = 12, .underlying_tile = 154, .item = item_make_short_sword()};
    g->floor_items[0].item.sharpened = 1;
    cJSON *root = legacy_geography_save(g);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "location"), 24);
    cJSON_AddNumberToObject(root, "moonveil_quest_state", 2);
    int ok = write_geography_save(root) && load_game(loaded, GEOGRAPHY_SLOT);
    ASSERT("removed garden saves return safely to Rosemoor without losing character or other quests", ok &&
        loaded->location == LOCATION_TOWN3 && map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) &&
        loaded->player.hp == 77 && loaded->gold == 431 && loaded->inventory_count == MAX_INVENTORY &&
        loaded->emberforge_progress == 1 && loaded->defeated_bosses == (1 << LOCATION_DUNGEON));
    ASSERT("dropped upgraded gear is reachable even with a full pack", ok && loaded->floor_items[0].active &&
        loaded->floor_items[0].item.sharpened && loaded->map.tiles[loaded->floor_items[0].y][loaded->floor_items[0].x] == TILE_ITEM &&
        loaded->floor_items[0].underlying_tile == TILE_TOWN_PATH &&
        map_is_walkable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y));
    save_game(loaded, GEOGRAPHY_SLOT);
    root = read_geography_save();
    ASSERT("new saves omit removed region caches, quests and boss reward fields", root &&
        !cJSON_GetObjectItem(root, "moonveil_cache") && !cJSON_GetObjectItem(root, "moonveil_quest_state") &&
        !cJSON_GetObjectItem(root, "thorn_regent_reward_pending"));
    cJSON_Delete(root);

    root = legacy_geography_save(g);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "location"), LOCATION_APPROACH);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "approach_route"), 10);
    ASSERT("the retired garden approach also returns to Rosemoor with dropped gear", write_geography_save(root) &&
        load_game(loaded, GEOGRAPHY_SLOT) && loaded->location == LOCATION_TOWN3 && loaded->floor_items[0].active);

    game_init(g);
    game_enter_town3(g);
    g->portal_active = 1;
    g->portal_location = 24;
    g->portal_level = 3;
    g->map.tiles[15][2] = TILE_PORTAL;
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 2, .y = 15, .underlying_tile = TILE_PORTAL, .item = item_make_staff()};
    g->map.tiles[15][2] = TILE_ITEM;
    g->map.tiles[13][18] = 206;
    g->inventory_count = MAX_INVENTORY;
    g->retired_coast_item_count = 5 * MAX_FLOOR_ITEMS;
    for (int i = 0; i < g->retired_coast_item_count; i++) {
        g->retired_coast_items[i] = item_make_staff();
    }
    root = legacy_geography_save(g);
    cJSON_AddNumberToObject(root, "thorn_regent_reward_pending", 3);
    ok = write_geography_save(root) && load_game(loaded, GEOGRAPHY_SLOT);
    ASSERT("retired portals close beneath loot while earned rewards survive a full pack and recovery queue", ok &&
        !loaded->portal_active && loaded->floor_items[0].active && loaded->floor_items[0].underlying_tile == TILE_TOWN_FLOOR &&
        loaded->retired_coast_item_count == 102);
    ASSERT("removing the retired botanist restores Rosemoor's paved plaza", ok && loaded->map.tiles[13][18] == TILE_TOWN_PATH);
    ASSERT("retired rewards persist once across a current-format save", save_game(loaded, GEOGRAPHY_SLOT) &&
        load_game(g, GEOGRAPHY_SLOT) && g->retired_coast_item_count == 102);

    game_init(g);
    approach_enter_region(g, 13);
    int y = g->map.stairs_up_y;
    // Recreate the version-196 eastbound approach without a retired runtime enum.
    g->map.stairs_up_x = 0;
    g->player.x = 7;
    g->player.y = y;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    map_mark_explored(&g->map, 7, y);
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){.active = 1, .x = 8, .y = y, .underlying_tile = TILE_TOWN_PATH, .item = item_make_staff()};
    g->ashen_cache[2].valid = 1;
    map_generate_ashen(&g->ashen_cache[2].map, 3);
    g->ashen_cache[2].enemy_count = 1;
    g->ashen_cache[2].enemies[0] = (Enemy){.active = 1, .type = ENEMY_CINDER_IMP, .hp = 7, .max_hp = 40};
    root = legacy_geography_save(g);
    ok = write_geography_save(root) && load_game(loaded, GEOGRAPHY_SLOT);
    ASSERT("legacy western Ashen approaches move to the castle while keeping dropped gear and wounds", ok &&
        approach_route(loaded) && approach_route(loaded)->town == LOCATION_CASTLE &&
        map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) &&
        loaded->floor_items[0].active && loaded->ashen_cache[2].enemies[0].hp == 7);
    ASSERT("relocated approaches do not mirror twice", save_game(loaded, GEOGRAPHY_SLOT) && load_game(g, GEOGRAPHY_SLOT) &&
        g->player.x == loaded->player.x && g->floor_items[0].x == loaded->floor_items[0].x);

    free(g);
    free(loaded);
    remove(GEOGRAPHY_SAVE);
}
