#include "test_utils.h"
#include "game/game.h"
#include "game/castle.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define CATALOG_SLOT 99225

static int same_catalog_item(const Item *a, const Item *b) {
    return a->type == b->type && strcmp(a->name, b->name) == 0 &&
        a->accessory_family == b->accessory_family && a->class_mask == b->class_mask &&
        a->attack_bonus == b->attack_bonus && a->defense_bonus == b->defense_bonus &&
        a->max_hp_bonus == b->max_hp_bonus && a->max_mp_bonus == b->max_mp_bonus && a->value == b->value;
}

static int write_legacy_catalog_save(void) {
    FILE *file = fopen("saves/savegame_99225.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = malloc((size_t)size + 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t bytes = fread(data, 1, (size_t)size, file);
    fclose(file);
    data[bytes] = '\0';
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_ReplaceItemInObject(root, "save_version", cJSON_CreateNumber(222));
    cJSON *inventory = cJSON_GetObjectItem(root, "inventory");
    for (int i = 0; i < cJSON_GetArraySize(inventory); i++) {
        cJSON_DeleteItemFromObject(cJSON_GetArrayItem(inventory, i), "accessory_family");
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99225.json", "wb");
    int ok = file && json && fwrite(json, 1, strlen(json), file) == strlen(json);
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

void test_equipment_catalog(void) {
    printf("Starter helmet and accessory catalog:\n");
    const Item items[] = {item_make_leather_cap(), item_make_leather_gauntlets(), item_make_leather_bracers(), item_make_ring_of_vigor(), item_make_copper_amulet()};
    const int prices[] = {80, 120, 80, 160, 160};
    int classes = 1;
    int prices_match = 1;
    for (int i = 0; i < 5; i++) {
        prices_match &= shop_buy_price(&items[i]) == prices[i];
        for (int player_class = CLASS_WARRIOR; player_class <= CLASS_ROGUE; player_class++) {
            classes &= item_class_allowed(&items[i], player_class);
        }
    }
    ASSERT("starter equipment is available to every class at the approved prices", classes && prices_match);
    ASSERT("jewelry classification separates rings and amulets from Blacksmith equipment", !item_is_jewelry(&items[0]) && !item_is_jewelry(&items[1]) && !item_is_jewelry(&items[2]) && item_is_jewelry(&items[3]) && item_is_jewelry(&items[4]));
    ASSERT("the accessory category rule permits two rings and one of every other category", item_accessory_limit(&items[0]) == 0 && item_accessory_limit(&items[1]) == 1 && item_accessory_limit(&items[2]) == 1 && item_accessory_limit(&items[3]) == 2 && item_accessory_limit(&items[4]) == 1);
    ASSERT("new types append without changing existing serialized item IDs", ITEM_ARROWS == 12 && ITEM_HELMET == 13 && ITEM_ACCESSORY == 14);

    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("catalog save fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("catalog fixture save slot is unused", !save_exists(CATALOG_SLOT));
    if (save_exists(CATALOG_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    game_init(g);
    g->inventory_count = 5;
    memcpy(g->inventory, items, sizeof(items));
    g->equipped_main_hand = -1;
    g->equipped_off_hand = -1;
    g->equipped_armor = -1;
    g->floor_item_count = 5;
    for (int i = 0; i < 5; i++) {
        int x = 17 + i;
        int y = 17;
        g->floor_items[i] = (FloorItem){.active = 1, .x = x, .y = y, .underlying_tile = g->map.tiles[y][x], .item = items[i]};
        g->map.tiles[y][x] = TILE_ITEM;
        g->castle_loot[0][i] = (FloorItem){.active = 1, .x = 15 + i, .y = 11, .underlying_tile = TILE_CASTLE_FLOOR, .item = items[i]};
    }
    g->castle_cache[0].valid = 1;
    castle_generate(&g->castle_cache[0].map, 1);
    g->castle_loot_count[0] = 5;
    g->gold = 413;
    g->player.hp = 41;
    g->defeated_bosses = 1 << LOCATION_DUNGEON;
    int ok = save_game(g, CATALOG_SLOT) && load_game(loaded, CATALOG_SLOT);
    ASSERT("new catalog items save and load", ok);
    if (ok) {
        int preserved = 1;
        for (int i = 0; i < 5; i++) {
            preserved &= same_catalog_item(&items[i], &loaded->inventory[i]) && same_catalog_item(&items[i], &loaded->floor_items[i].item) && same_catalog_item(&items[i], &loaded->castle_loot[0][i].item);
        }
        ASSERT("categories and bonuses persist through bag, live loot and cached-loot serialization", preserved);
        ASSERT("catalog metadata loading preserves wounds, currency and boss victories", loaded->player.hp == 41 && loaded->gold == 413 && loaded->defeated_bosses == g->defeated_bosses);
    }
    game_init(g);
    Item sword = g->inventory[g->equipped_main_hand];
    int attack = g->player.attack;
    g->gold = 327;
    g->player.hp = 37;
    memset(loaded, 0, sizeof(*loaded));
    for (int i = 0; i < MAX_INVENTORY; i++) {
        loaded->inventory[i].accessory_family = (AccessoryFamily)999;
    }
    ok = save_game(g, CATALOG_SLOT) && write_legacy_catalog_save() && load_game(loaded, CATALOG_SLOT);
    ASSERT("version-222 equipment without accessory metadata remains usable", ok && loaded->equipped_main_hand == g->equipped_main_hand && same_catalog_item(&sword, &loaded->inventory[loaded->equipped_main_hand]) && loaded->player.attack == attack && loaded->player.hp == 37 && loaded->gold == 327);
    if (ok) {
        int defaults = 1;
        for (int i = 0; i < loaded->inventory_count; i++) {
            defaults &= loaded->inventory[i].accessory_family == ACCESSORY_FAMILY_NONE;
        }
        ASSERT("legacy items explicitly default to no accessory category", defaults);
    }
    remove("saves/savegame_99225.json");
    free(g);
    free(loaded);
}
