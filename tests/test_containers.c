#include "test_utils.h"
#include "game/containers.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define CONTAINER_SLOT 99289

static void fixture(GameState *g, Location location, int kind) {
    game_init(g);
    g->location = location;
    g->level = 1;
    g->enemy_count = 0;
    g->player.x = 8;
    g->player.y = 9;
    g->map.tiles[8][8] = kind;
    g->map.tiles[9][8] = TILE_FLOOR;
    g->mangroves.boat_x = 1;
    g->mangroves.boat_y = 1;
}

static void act(GameState *g, ActionType type) {
    action_resolve_player(g, (Action){type, 0, 0});
}

static void edit_save(const char *field, double value, int legacy) {
    char path[96];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", CONTAINER_SLOT);
    FILE *file = fopen(path, "rb");
    ASSERT("container save fixture exists before editing it", file != NULL);
    if (!file) {
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
    if (legacy) {
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 208);
        cJSON_DeleteItemFromObject(root, "loot_containers");
    } else {
        cJSON *container = cJSON_GetArrayItem(cJSON_GetObjectItem(root, "loot_containers"), 0);
        cJSON_ReplaceItemInObject(container, field, cJSON_CreateNumber(value));
    }
    buffer = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(path, "wb");
    fputs(buffer, file);
    fclose(file);
    free(buffer);
}

void test_containers(void) {
    ASSERT("container fixture slot is unused", !save_exists(CONTAINER_SLOT));
    if (save_exists(CONTAINER_SLOT)) {
        return;
    }
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("allocate container fixtures", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    const Location locations[] = {LOCATION_DUNGEON, LOCATION_MOUNTAINS, LOCATION_COAST, LOCATION_MANGROVES,
        LOCATION_GLASSDEEP, LOCATION_STORMWATCH, LOCATION_IRONFANG, LOCATION_GIANTS_CROWN, LOCATION_CROWNROAD_NORTH};
    const int kinds[] = {TILE_CRYPT_CACHE, TILE_MOUNTAIN_CACHE, TILE_COAST_CACHE, TILE_MANGROVE_CACHE,
        TILE_STONEWAKE_SUPPLIES, TILE_STORM_SUPPLIES, TILE_IRON_CACHE, TILE_GIANTS_CACHE, TILE_CROWN_SUPPLIES};
    for (int n = 0; n < 9; n++) {
        fixture(g, locations[n], kinds[n]);
        int gold = g->gold;
        g->player.y = 8;
        act(g, ACTION_PICK_UP);
        ASSERT("Pick Up never opens a closed regional chest", g->gold == gold && !g->loot_container_count && g->map.tiles[8][8] == kinds[n]);
        g->player.y = 9;
        ASSERT("a cardinally adjacent chest advertises Interact", game_has_regional_interaction(g) && containers_hint(g));
        act(g, ACTION_INTERACT);
        const LootContainer *c = containers_record(g, 8, 8);
        ASSERT("Interact opens a persistent chest and spills gold without awarding it", c && c->opened && c->released && c->gold > 0 && g->gold == gold);
        if (!c) {
            continue;
        }
        int reward = c->gold;
        int score = g->score;
        int score_reward = c->gold_score;
        act(g, ACTION_INTERACT);
        ASSERT("a second Interact closes the same chest and leaves its gold on the ground", !c->opened && c->gold == reward && g->gold == gold);
        act(g, ACTION_PICK_UP);
        ASSERT("Pick Up alone collects released gold even after the chest closes", g->gold == gold + reward && g->score == score + score_reward && !c->gold);
        act(g, ACTION_INTERACT);
        act(g, ACTION_INTERACT);
        act(g, ACTION_INTERACT);
        act(g, ACTION_PICK_UP);
        ASSERT("reopening an emptied chest cannot produce another payout", c->opened && !c->gold && g->gold == gold + reward && g->loot_container_count == 1);
        ASSERT("regional container state is valid", containers_valid(g));
    }

    fixture(g, LOCATION_DUNGEON, TILE_CRYPT_CACHE);
    g->inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[i] = item_make_health_potion();
    }
    g->floor_item_count = MAX_FLOOR_ITEMS;
    for (int i = 0; i < MAX_FLOOR_ITEMS; i++) {
        g->floor_items[i] = (FloorItem){.active = 1, .x = 15 + i, .y = 15, .underlying_tile = TILE_FLOOR, .item = item_make_health_potion()};
    }
    act(g, ACTION_INTERACT);
    act(g, ACTION_PICK_UP);
    ASSERT("full inventory and full floor-item buffers cannot lose chest gold", g->gold == 12 && g->inventory_count == MAX_INVENTORY && g->floor_item_count == MAX_FLOOR_ITEMS);

    fixture(g, LOCATION_DUNGEON, TILE_CRYPT_CACHE);
    act(g, ACTION_INTERACT);
    g->map.tiles[8][8] = TILE_WALL;
    g->map.tiles[9][8] = TILE_WALL;
    g->map.tiles[10][8] = TILE_FLOOR;
    containers_restore(g);
    ASSERT("regenerated quest maps preserve chests and loose gold on walkable ground", map_is_walkable(&g->map, g->loot_containers[0].x, g->loot_containers[0].y) &&
        map_is_walkable(&g->map, g->loot_containers[0].loot_x, g->loot_containers[0].loot_y) && g->loot_containers[0].gold == 12);

    fixture(g, LOCATION_COAST, TILE_COAST_CACHE);
    g->map.tiles[9][8] = TILE_COAST_BEACON_UNLIT;
    ASSERT("standing on a quest beacon takes priority over an adjacent chest", !containers_hint(g) && !containers_interact(g) && !g->loot_container_count);

    fixture(g, LOCATION_COAST, TILE_QUEEN_RELIQUARY);
    g->drowned_queen_reward_state = 1;
    g->inventory_count = MAX_INVENTORY;
    act(g, ACTION_INTERACT);
    ASSERT("opening a boss reward chest releases equipment without claiming it", g->loot_containers[0].rewards == 1 && g->drowned_queen_reward_state == 1 && g->inventory_count == MAX_INVENTORY);
    act(g, ACTION_PICK_UP);
    ASSERT("released boss equipment survives a full pack", g->loot_containers[0].rewards == 1 && g->drowned_queen_reward_state == 1);
    g->inventory_count--;
    act(g, ACTION_PICK_UP);
    ASSERT("Pick Up claims released boss gear exactly once", g->drowned_queen_reward_state == 2 && !g->loot_containers[0].rewards &&
        strcmp(g->inventory[g->inventory_count - 1].name, "Tidecaller Robes") == 0);
    int count = g->inventory_count;
    act(g, ACTION_INTERACT);
    act(g, ACTION_INTERACT);
    act(g, ACTION_PICK_UP);
    ASSERT("boss chest remains toggleable after its reward is claimed", g->loot_containers[0].opened && g->inventory_count == count);

    fixture(g, LOCATION_CASTLE_INTERIOR, TILE_CASTLE_FLOOR);
    g->level = 2;
    g->castellan_reward_state = 1;
    g->player.x = 53;
    g->player.y = 56;
    act(g, ACTION_PICK_UP);
    ASSERT("Pick Up cannot open the castle armory reward chest", g->castellan_reward_state == 1 && !g->loot_container_count);
    act(g, ACTION_INTERACT);
    ASSERT("the castle armory chest opens without awarding gear", g->castellan_reward_state == 1 && g->loot_container_count == 1);
    act(g, ACTION_PICK_UP);
    ASSERT("Pick Up claims released castle equipment", g->castellan_reward_state == 2 && g->inventory[g->inventory_count - 1].visual_id == ITEM_VISUAL_CASTELLAN_BULWARK);

    fixture(g, LOCATION_GIANTS_CROWN, TILE_GIANTS_EMPTY);
    act(g, ACTION_INTERACT);
    act(g, ACTION_INTERACT);
    act(g, ACTION_PICK_UP);
    ASSERT("legacy emptied coffers remain toggleable without paying again", g->loot_containers[0].opened && !g->loot_containers[0].gold && g->gold == 0);

    fixture(g, LOCATION_GLASSDEEP, TILE_STONEWAKE_SUPPLIES);
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){.active = 1, .x = 9, .y = 8};
    ASSERT("guarded supplies keep their existing lock", containers_interact(g) && !g->loot_container_count);
    g->enemies[0].active = 0;
    ASSERT("defeating guards permits opening supplies", containers_interact(g) && g->loot_containers[0].gold == 25);

    fixture(g, LOCATION_DUNGEON, TILE_CRYPT_CACHE);
    act(g, ACTION_INTERACT);
    g->level = 2;
    g->map.tiles[8][8] = TILE_CRYPT_CACHE;
    act(g, ACTION_INTERACT);
    ASSERT("different stages have independent chest contents", g->loot_container_count == 2 && g->loot_containers[0].gold == 12 && g->loot_containers[1].gold == 14);
    g->level = 1;
    act(g, ACTION_PICK_UP);
    ASSERT("backtracking recovers only the current stage's pending gold", g->gold == 12 && !g->loot_containers[0].gold && g->loot_containers[1].gold == 14);
    LootContainer records[2];
    memcpy(records, g->loot_containers, sizeof(records));
    game_init(g);
    memcpy(g->loot_containers, records, sizeof(records));
    g->loot_container_count = 2;
    g->gold = 12;
    ASSERT("save/load retains open chests and uncollected loot in another region", save_game(g, CONTAINER_SLOT) && load_game(loaded, CONTAINER_SLOT) &&
        loaded->loot_container_count == 2 && memcmp(g->loot_containers, loaded->loot_containers, sizeof(records)) == 0 && loaded->gold == 12);
    edit_save("opened", 0.5, 0);
    ASSERT("fractional container states are rejected", !load_game(loaded, CONTAINER_SLOT));
    save_game(g, CONTAINER_SLOT);
    edit_save("kind", -1, 0);
    ASSERT("unknown container kinds are rejected", !load_game(loaded, CONTAINER_SLOT));
    game_init(g);
    g->gold = 321;
    save_game(g, CONTAINER_SLOT);
    edit_save(NULL, 0, 1);
    ASSERT("previous saves migrate without rebuilding maps or resetting wealth", load_game(loaded, CONTAINER_SLOT) && !loaded->loot_container_count && loaded->gold == 321 &&
        memcmp(g->map.tiles, loaded->map.tiles, sizeof(g->map.tiles)) == 0);
    char path[96];
    snprintf(path, sizeof(path), "saves/savegame_%d.json", CONTAINER_SLOT);
    remove(path);
    free(g);
    free(loaded);
}
