#include "test_utils.h"
#include "game/approach.h"
#include "game/town_life.h"
#include "game/town_road.h"
#include "screens/quest_journal.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define RETIRE_SLOT 99147
#define RETIRE_PATH "saves/savegame_99147.json"

static cJSON *read_retirement_save(void) {
    FILE *file = fopen(RETIRE_PATH, "rb");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *text = malloc(size + 1);
    if (!text) {
        fclose(file);
        return NULL;
    }
    size_t count = fread(text, 1, size, file);
    fclose(file);
    text[count] = '\0';
    cJSON *root = cJSON_Parse(text);
    free(text);
    return root;
}

static int write_retirement_save(const cJSON *root) {
    char *text = cJSON_PrintUnformatted(root);
    if (!text) {
        return 0;
    }
    FILE *file = fopen(RETIRE_PATH, "wb");
    int ok = file && fputs(text, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(text);
    return ok;
}

static int make_pirate_save(const GameState *g, int version) {
    if (!save_game(g, RETIRE_SLOT)) {
        return 0;
    }
    cJSON *root = read_retirement_save();
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    cJSON *loot = cJSON_DetachItemFromObject(root, "retired_coast_loot");
    cJSON *stages = cJSON_CreateArray();
    for (int stage = 0; stage < 5; stage++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "seed", 100 + stage);
        cJSON *items = cJSON_DetachItemFromArray(loot, 0);
        if (g->location == (Location)44 && g->level == stage + 1) {
            cJSON_Delete(items);
            items = cJSON_Duplicate(cJSON_GetObjectItem(root, "floor_items"), 1);
        }
        cJSON_AddItemToObject(entry, "loot", items);
        cJSON_AddItemToArray(stages, entry);
    }
    cJSON_Delete(loot);
    cJSON_AddItemToObject(root, "pirate_cache", stages);
    cJSON_AddNumberToObject(root, "pirate_defeated", 1);
    cJSON_AddNumberToObject(root, "pirate_quest_state", 2);
    int ok = write_retirement_save(root);
    cJSON_Delete(root);
    return ok;
}

static void prepare_retirement(GameState *g) {
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    map_generate_dawnmere(&g->map, &g->player.x, &g->player.y);
    g->location = LOCATION_DAWNMERE;
    g->player.last_dx = 1;
    g->player.last_dy = 0;
    g->gold = 432;
    g->score = 9876;
    g->alder_quest_state = 1;
    g->alder_wardens_rescued = 3;
    g->defeated_bosses = 1 << LOCATION_DUNGEON;
    g->inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->inventory[0] = item_make_ranger_cloak();
    strcpy(g->inventory[0].name, "Mariner's Cloak");
    g->inventory[0].defense_bonus = 3;
    g->inventory[0].evasion_chance = 5;
    g->equipped_main_hand = -1;
    g->equipped_off_hand = -1;
    g->equipped_armor = 0;
    g->retired_coast_item_count = 5 * MAX_FLOOR_ITEMS;
    for (int i = 0; i < g->retired_coast_item_count; i++) {
        g->retired_coast_items[i] = item_make_long_sword();
        g->retired_coast_items[i].attack_bonus += i;
        g->retired_coast_items[i].sharpened = 1;
    }
    g->floor_item_count = 1;
    g->floor_items[0] = (FloorItem){
        .active = 1, .x = 9, .y = 10, .underlying_tile = TILE_LOCAL_DOOR,
        .item = item_make_buckler()
    };
    g->map.tiles[10][9] = TILE_ITEM;
    g->map.tiles[TOWN_H - 1][20] = TILE_TOWN_EXIT;
    map_mark_explored(&g->map, 3, 12);
}

void test_retired_coast(void) {
    printf("Retired coastal content and save migration tests:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("retirement test fixtures allocate on the heap", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    const int locations[] = {44, 45, LOCATION_APPROACH, LOCATION_DAWNMERE, LOCATION_TOWN_ROAD};
    for (int i = 0; i < 5; i++) {
        prepare_retirement(g);
        if (locations[i] == LOCATION_TOWN_ROAD) {
            town_road_enter(g, LOCATION_DAWNMERE, TOWN_SIDE_WEST, 0);
            g->map.tiles[TOWN_ROAD_H - 1][TOWN_ROAD_W / 2] = TILE_TOWN_ROAD_EXIT_SOUTH;
            g->player.x = TOWN_ROAD_W / 2;
            g->player.y = TOWN_ROAD_H - 1;
        }
        g->location = (Location)locations[i];
        g->approach_route = i == 2 ? 17 : -1;
        g->level = i == 0 ? 3 : 1;
        g->portal_active = 1;
        g->portal_location = (Location)44;
        g->portal_level = 3;
        g->map.tiles[14][20] = TILE_PORTAL;
        int ok = make_pirate_save(g, 184 + i) && load_game(loaded, RETIRE_SLOT);
        ASSERT("versions 184-188 load from removed coast, merchant, approach, town and exterior", ok);
        if (!ok) {
            continue;
        }
        ASSERT("removed destinations return to Dawnmere and close obsolete portals", loaded->location == (i == 4 ? LOCATION_TOWN_ROAD : LOCATION_DAWNMERE) && !loaded->portal_active && loaded->portal_location == LOCATION_DUNGEON && loaded->approach_route == -1);
        ASSERT("migration preserves gold, score, earned equipment, victories and other quests", loaded->gold == 432 && loaded->score == 9876 && loaded->inventory_count == MAX_INVENTORY && loaded->equipped_armor == 0 && !strcmp(loaded->inventory[0].name, "Mariner's Cloak") && loaded->inventory[0].evasion_chance == 5 && loaded->defeated_bosses == (1 << LOCATION_DUNGEON) && loaded->alder_quest_state == 1 && loaded->alder_wardens_rescued == 3);
        ASSERT("full backpacks retain all cached gear without duplicating the active stage", loaded->retired_coast_item_count == (i == 0 ? 80 : 100));
        ASSERT("migration leaves a walkable player with the replacement southern route", map_is_walkable(&loaded->map, loaded->player.x, loaded->player.y) && (i == 4 ? loaded->map.tiles[TOWN_ROAD_H - 1][TOWN_ROAD_W / 2] == TILE_TOWN_ROAD_EXIT_SOUTH : loaded->map.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT));
        if (i < 4) {
            ASSERT("retired floor gear stays recoverable beside the restored merchant entrance", loaded->floor_item_count == 1 && loaded->floor_items[0].active && loaded->floor_items[0].item.type == ITEM_SHIELD && map_is_walkable(&loaded->map, loaded->floor_items[0].x, loaded->floor_items[0].y) && loaded->floor_items[0].underlying_tile != TILE_LOCAL_DOOR && loaded->map.tiles[10][9] == TILE_LOCAL_DOOR);
        }
        if (i == 3) {
            ASSERT("saved town exploration and valid position/facing survive retirement", map_is_explored(&loaded->map, 3, 12) && loaded->player.x == g->player.x && loaded->player.y == g->player.y && loaded->player.last_dx == g->player.last_dx);
        }
        int pending = loaded->retired_coast_item_count;
        ASSERT("recovery overflow survives the new save format", save_game(loaded, RETIRE_SLOT) && load_game(g, RETIRE_SLOT) && g->retired_coast_item_count == pending);
        cJSON *root = read_retirement_save();
        ASSERT("new saves omit the pirate region, victory and treasure quest", root && !cJSON_GetObjectItem(root, "pirate_cache") && !cJSON_GetObjectItem(root, "pirate_defeated") && !cJSON_GetObjectItem(root, "pirate_quest_state"));
        cJSON_Delete(root);
        Item expected = g->retired_coast_items[pending - 1];
        if (g->location == LOCATION_TOWN_ROAD) {
            game_enter_town_gate(g, LOCATION_DAWNMERE, 0, 12);
        }
        game_remove_inventory_item(g, MAX_INVENTORY - 1);
        ASSERT("a free pack slot reclaims the original upgraded weapon exactly once", g->inventory_count == MAX_INVENTORY && g->retired_coast_item_count == pending - 1 && !memcmp(&g->inventory[MAX_INVENTORY - 1], &expected, sizeof(expected)));
    }

    prepare_retirement(g);
    g->location = LOCATION_APPROACH;
    g->approach_route = 17;
    g->portal_active = 1;
    g->portal_location = LOCATION_DUNGEON;
    g->portal_level = 1;
    g->return_portal_town = LOCATION_DAWNMERE;
    int ok = make_pirate_save(g, 188) && load_game(loaded, RETIRE_SLOT);
    int portal = 0;
    for (int y = 0; ok && y < TOWN_H; y++) {
        for (int x = 0; x < TOWN_W; x++) {
            portal |= loaded->map.tiles[y][x] == TILE_PORTAL;
        }
    }
    ASSERT("returning from the retired approach retains an unrelated dungeon portal", ok && loaded->portal_active && loaded->portal_location == LOCATION_DUNGEON && portal);

    TownApproach connection;
    ASSERT("Dawnmere retains Saltwind south and has Stormwatch east", town_road_connection(LOCATION_DAWNMERE, TOWN_SIDE_SOUTH, &connection) && connection.area == LOCATION_MANGROVES && town_road_connection(LOCATION_DAWNMERE, TOWN_SIDE_EAST, &connection) && connection.area == LOCATION_STORMWATCH);
    ASSERT("the pirate merchant interior and boss journal entry are removed", !town_life_building((Location)45) && JOURNAL_BOSS_COUNT == 19);
    prepare_retirement(g);
    g->location = LOCATION_JAIL;
    game_remove_inventory_item(g, MAX_INVENTORY - 1);
    ASSERT("recovery cannot refill a confiscated pack inside jail", g->inventory_count == MAX_INVENTORY - 1 && g->retired_coast_item_count == 100);
    g->location = LOCATION_DAWNMERE;
    game_recover_retired_items(g);
    ASSERT("pending recovery resumes safely in town", g->inventory_count == MAX_INVENTORY && g->retired_coast_item_count == 99);
    remove(RETIRE_PATH);
    free(g);
    free(loaded);
}
