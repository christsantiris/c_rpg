#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include "../src/screens/slot_select.h"
#include "../external/cJSON.h"
#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUND_TRIP_SLOT 99001
#define LEGACY_SLOT 99002
#define MIGRATED_SLOT 99003
#define LEGACY_ARMOR_SLOT 99004
#define MIGRATED_ARMOR_SLOT 99005
#define DUAL_WIELD_SLOT 99006
#define LEGACY_OFF_HAND_SLOT 99007
#define LEGACY_GOBLIN_REWARD_SLOT 99008
#define BLOCKED_DUNGEON_SLOT 99009

void test_save_confirmation(void) {
    printf("Save confirmation tests:\n");
    SlotSelect slots;
    slot_select_init(&slots);
    ASSERT("slot selection starts without a pending save",
        !slots.confirming_save && slot_select_handle_key(&slots,
            SDL_SCANCODE_RETURN) == SLOT_SELECTED);

    slot_select_begin_save(&slots, 1);
    ASSERT("occupied save slot requires overwrite confirmation",
        slots.confirming_save && slots.overwriting_save);
    ASSERT("navigation and Enter cannot confirm an overwrite",
        slot_select_handle_key(&slots, SDL_SCANCODE_DOWN) == SLOT_NONE &&
        slots.selected == 0 &&
        slot_select_handle_key(&slots, SDL_SCANCODE_RETURN) == SLOT_NONE &&
        slots.confirming_save);
    ASSERT("N cancels the overwrite and returns to slot selection",
        slot_select_handle_key(&slots, SDL_SCANCODE_N) == SLOT_NONE &&
        !slots.confirming_save &&
        slot_select_handle_key(&slots, SDL_SCANCODE_RETURN) == SLOT_SELECTED);

    slot_select_begin_save(&slots, 0);
    ASSERT("empty save slots also ask for confirmation",
        slots.confirming_save && !slots.overwriting_save);
    ASSERT("Escape cancels without confirming a save",
        slot_select_handle_key(&slots, SDL_SCANCODE_ESCAPE) == SLOT_NONE &&
        !slots.confirming_save);

    slot_select_begin_save(&slots, 1);
    ASSERT("only Y confirms the selected save slot",
        slot_select_handle_key(&slots, SDL_SCANCODE_Y) ==
            SLOT_SAVE_CONFIRMED && !slots.confirming_save &&
            slots.selected == 0);
}

static void format_save_path(int slot, char *path, int size) {
    snprintf(path, size, "saves/savegame_%d.json", slot);
}

static void remove_test_save(int slot) {
    char path[64];
    format_save_path(slot, path, sizeof(path));
    remove(path);
}

static int weapon_fields_match(const Item *a, const Item *b) {
    return a->type == b->type &&
        strcmp(a->name, b->name) == 0 &&
        a->attack_bonus == b->attack_bonus &&
        a->value == b->value &&
        a->is_ranged == b->is_ranged &&
        a->range == b->range &&
        a->weapon_family == b->weapon_family &&
        a->weapon_hands == b->weapon_hands &&
        a->rarity == b->rarity &&
        a->class_mask == b->class_mask &&
        a->visual_id == b->visual_id &&
        a->critical_chance_bonus == b->critical_chance_bonus &&
        a->cleave_percent == b->cleave_percent &&
        a->pierces_targets == b->pierces_targets &&
        a->spell_power_bonus == b->spell_power_bonus &&
        a->max_mp_bonus == b->max_mp_bonus &&
        a->spell_cost_reduction_percent ==
            b->spell_cost_reduction_percent &&
        a->armor_penetration_percent == b->armor_penetration_percent;
}

static int armor_fields_match(const Item *a, const Item *b) {
    return a->type == b->type && strcmp(a->name, b->name) == 0 &&
        a->defense_bonus == b->defense_bonus && a->value == b->value &&
        a->rarity == b->rarity && a->class_mask == b->class_mask &&
        a->visual_id == b->visual_id &&
        a->armor_family == b->armor_family &&
        a->max_hp_bonus == b->max_hp_bonus &&
        a->max_mp_bonus == b->max_mp_bonus &&
        a->evasion_chance == b->evasion_chance &&
        a->spell_cost_reduction_percent ==
            b->spell_cost_reduction_percent;
}

static int shield_fields_match(const Item *a, const Item *b) {
    return a->type == ITEM_SHIELD && b->type == ITEM_SHIELD &&
        strcmp(a->name, b->name) == 0 &&
        a->defense_bonus == b->defense_bonus &&
        a->value == b->value && a->rarity == b->rarity &&
        a->class_mask == b->class_mask && a->visual_id == b->visual_id &&
        a->block_chance == b->block_chance &&
        a->block_reduction_percent == b->block_reduction_percent;
}

static int rewrite_save_version(int slot, int version) {
    char path[64];
    format_save_path(slot, path, sizeof(path));
    FILE *file = fopen(path, "r");
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
    size_t bytes_read = fread(text, 1, length, file);
    fclose(file);
    text[bytes_read] = '\0';

    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) {
        return 0;
    }
    cJSON *save_version = cJSON_GetObjectItem(root, "save_version");
    if (!save_version) {
        cJSON_Delete(root);
        return 0;
    }
    save_version->valueint = version;
    save_version->valuedouble = version;
    char *updated = cJSON_Print(root);
    cJSON_Delete(root);
    if (!updated) {
        return 0;
    }

    file = fopen(path, "w");
    if (!file) {
        free(updated);
        return 0;
    }
    fputs(updated, file);
    fclose(file);
    free(updated);
    return 1;
}

static void test_current_weapon_round_trip(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);

    original.inventory_count = 10;
    original.inventory[0] = item_make_magic_long_sword();
    original.inventory[1] = item_make_magic_dagger();
    original.inventory[2] = item_make_magic_greatsword();
    original.inventory[3] = item_make_magic_longbow();
    original.inventory[4] = item_make_magic_staff();
    original.inventory[5] = item_make_magic_battle_axe();
    original.inventory[6] = item_make_magic_plate();
    original.inventory[7] = item_make_shadow_armor();
    original.inventory[8] = item_make_archmage_robes();
    original.inventory[9] = item_make_magic_shield();
    original.equipped_main_hand = 0;
    original.equipped_off_hand = 9;
    original.equipped_armor = 6;
    original.player.known_spell_count = 1;
    original.player.known_spells[0] = spell_make_magic_arrow();
    spell_upgrade(&original.player.known_spells[0]);
    original.enemy_count = 1;
    original.enemies[0].active = 1;
    original.enemies[0].frozen_turns = 2;
    original.dungeon_crypt_keys = 2;
    original.player.level = 9;
    original.level = 3;

    remove_test_save(ROUND_TRIP_SLOT);
    int saved = save_game(&original, ROUND_TRIP_SLOT);
    char preview_name[21];
    int preview_level = 0;
    int preview_ok = saved && get_save_preview(ROUND_TRIP_SLOT,
        preview_name, &preview_level);
    ASSERT("save preview shows character level rather than floor depth",
        preview_ok && preview_level == 9 &&
        strcmp(preview_name, original.player.name) == 0);
    int loaded_ok = saved && load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("weapon save can be loaded", loaded_ok);
    if (!loaded_ok) {
        remove_test_save(ROUND_TRIP_SLOT);
        return;
    }

    int fields_match = loaded.inventory_count == original.inventory_count;
    for (int i = 0; i < 6 && fields_match; i++) {
        fields_match = weapon_fields_match(&original.inventory[i],
            &loaded.inventory[i]);
    }
    ASSERT("all weapon traits and metadata survive save/load", fields_match);
    ASSERT("main-hand index survives save/load",
        loaded.equipped_main_hand == original.equipped_main_hand);
    ASSERT("off-hand index survives save/load",
        loaded.equipped_off_hand == original.equipped_off_hand);
    ASSERT("armor index survives save/load",
        loaded.equipped_armor == original.equipped_armor);
    int armor_fields_survive = 1;
    for (int i = 6; i < 9 && armor_fields_survive; i++) {
        armor_fields_survive = armor_fields_match(&loaded.inventory[i],
            &original.inventory[i]);
    }
    ASSERT("all armor traits and metadata survive save/load",
        armor_fields_survive);
    ASSERT("shield traits survive save/load",
        shield_fields_match(&loaded.inventory[9], &original.inventory[9]));
    ASSERT("spell rank survives save/load",
        loaded.player.known_spells[0].rank == 2 &&
        loaded.player.known_spells[0].damage == 25);
    ASSERT("freeze duration survives save/load",
        loaded.enemies[0].frozen_turns == 2);
    ASSERT("crypt keys survive save/load", loaded.dungeon_crypt_keys == 2);
    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_dual_wield_round_trip(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);
    game_unequip_main_hand(&original);
    original.inventory_count = 2;
    original.inventory[0] = item_make_long_sword();
    original.inventory[1] = item_make_short_sword();
    game_equip_main_hand(&original, 0);
    game_equip_off_hand(&original, 1);
    int equipped_attack = original.player.attack;

    remove_test_save(DUAL_WIELD_SLOT);
    int saved = save_game(&original, DUAL_WIELD_SLOT);
    int loaded_ok = saved && load_game(&loaded, DUAL_WIELD_SLOT);
    ASSERT("dual-wield save can be loaded", loaded_ok);
    if (loaded_ok) {
        ASSERT("dual-wield indices survive save/load",
            loaded.equipped_main_hand == 0 &&
            loaded.equipped_off_hand == 1);
        ASSERT("off-hand attack total survives save/load",
            loaded.player.attack == equipped_attack);
    }
    remove_test_save(DUAL_WIELD_SLOT);
}

static void test_legacy_off_hand_migration(void) {
    static GameState legacy;
    static GameState migrated;
    memset(&legacy, 0, sizeof(legacy));
    memset(&migrated, 0, sizeof(migrated));
    legacy.player.player_class = CLASS_WARRIOR;
    game_init(&legacy);
    game_unequip_main_hand(&legacy);
    legacy.inventory_count = 2;
    legacy.inventory[0] = item_make_long_sword();
    legacy.inventory[1] = item_make_short_sword();
    game_equip_main_hand(&legacy, 0);
    legacy.equipped_off_hand = 1;
    int legacy_attack = legacy.player.attack;

    remove_test_save(LEGACY_OFF_HAND_SLOT);
    int saved = save_game(&legacy, LEGACY_OFF_HAND_SLOT);
    int version_changed = saved &&
        rewrite_save_version(LEGACY_OFF_HAND_SLOT, 39);
    int loaded_ok = version_changed &&
        load_game(&migrated, LEGACY_OFF_HAND_SLOT);
    ASSERT("version 39 off-hand save migrates", loaded_ok);
    if (loaded_ok) {
        ASSERT("legacy stale off-hand index is cleared",
            migrated.equipped_off_hand == -1);
        ASSERT("legacy main-hand attack remains unchanged",
            migrated.player.attack == legacy_attack);
    }
    remove_test_save(LEGACY_OFF_HAND_SLOT);
}

static void test_legacy_goblin_reward_migration(void) {
    static GameState legacy;
    static GameState migrated;
    memset(&legacy, 0, sizeof(legacy));
    memset(&migrated, 0, sizeof(migrated));
    legacy.player.player_class = CLASS_WARRIOR;
    game_init(&legacy);
    game_unequip_main_hand(&legacy);
    int base_attack = legacy.player.attack;
    legacy.inventory_count = 1;
    legacy.inventory[0] = item_make_goblin_king_greatsword();
    game_equip_main_hand(&legacy, 0);

    remove_test_save(LEGACY_GOBLIN_REWARD_SLOT);
    int saved = save_game(&legacy, LEGACY_GOBLIN_REWARD_SLOT);
    int version_changed = saved &&
        rewrite_save_version(LEGACY_GOBLIN_REWARD_SLOT, 42);
    int loaded_ok = version_changed &&
        load_game(&migrated, LEGACY_GOBLIN_REWARD_SLOT);
    ASSERT("legacy Goblin King reward save migrates", loaded_ok);
    if (loaded_ok) {
        ASSERT("legacy Goblin King greatsword becomes shield",
            strcmp(migrated.inventory[0].name,
                "Goblin King's Shield") == 0 &&
            migrated.inventory[0].type == ITEM_SHIELD);
        ASSERT("replaced Goblin King weapon is safely unequipped",
            migrated.equipped_main_hand == -1 &&
            migrated.player.attack == base_attack);
    }
    remove_test_save(LEGACY_GOBLIN_REWARD_SLOT);
}

static void test_migrated_armor_round_trip(void) {
    static GameState legacy;
    static GameState migrated;
    static GameState reloaded;
    memset(&legacy, 0, sizeof(legacy));
    memset(&migrated, 0, sizeof(migrated));
    memset(&reloaded, 0, sizeof(reloaded));
    legacy.player.player_class = CLASS_WARRIOR;
    game_init(&legacy);
    legacy.inventory_count = 1;
    legacy.inventory[0] = item_make_chain_mail();
    legacy.inventory[0].defense_bonus = 5;
    legacy.inventory[0].value = 40;
    legacy.inventory[0].armor_family = ARMOR_FAMILY_NONE;
    legacy.inventory[0].class_mask = 0;
    legacy.inventory[0].visual_id = ITEM_VISUAL_NONE;
    legacy.equipped_main_hand = -1;
    legacy.equipped_armor = 0;
    legacy.player.defense = 11;
    legacy.floor_item_count = 1;
    legacy.floor_items[0].active = 1;
    legacy.floor_items[0].item = item_make_leather_armor();
    legacy.floor_items[0].item.value = 15;
    legacy.floor_items[0].item.armor_family = ARMOR_FAMILY_NONE;
    legacy.floor_items[0].item.class_mask = 0;
    legacy.floor_items[0].item.visual_id = ITEM_VISUAL_NONE;

    remove_test_save(LEGACY_ARMOR_SLOT);
    remove_test_save(MIGRATED_ARMOR_SLOT);
    int saved = save_game(&legacy, LEGACY_ARMOR_SLOT);
    int version_changed = saved &&
        rewrite_save_version(LEGACY_ARMOR_SLOT, 38);
    int migrated_ok = version_changed &&
        load_game(&migrated, LEGACY_ARMOR_SLOT);
    ASSERT("version 38 armor save migrates", migrated_ok);
    if (!migrated_ok) {
        remove_test_save(LEGACY_ARMOR_SLOT);
        remove_test_save(MIGRATED_ARMOR_SLOT);
        return;
    }

    Item expected_armor = item_make_chain_mail();
    Item expected_drop = item_make_leather_armor();
    ASSERT("legacy equipped armor receives current metadata",
        armor_fields_match(&migrated.inventory[0], &expected_armor));
    ASSERT("armor migration corrects equipped defense",
        migrated.player.defense == 6 + expected_armor.defense_bonus);
    ASSERT("legacy floor armor receives current metadata",
        armor_fields_match(&migrated.floor_items[0].item, &expected_drop));

    int migrated_saved = save_game(&migrated, MIGRATED_ARMOR_SLOT);
    int reloaded_ok = migrated_saved &&
        load_game(&reloaded, MIGRATED_ARMOR_SLOT);
    ASSERT("migrated armor save loads again", reloaded_ok);
    if (reloaded_ok) {
        ASSERT("migrated armor remains stable after another save/load",
            armor_fields_match(&reloaded.inventory[0], &expected_armor) &&
            armor_fields_match(&reloaded.floor_items[0].item,
                &expected_drop));
        ASSERT("migrated armor index and defense remain stable",
            reloaded.equipped_armor == 0 &&
            reloaded.player.defense == migrated.player.defense);
    }
    remove_test_save(LEGACY_ARMOR_SLOT);
    remove_test_save(MIGRATED_ARMOR_SLOT);
}

static void test_migrated_weapon_round_trip(void) {
    static GameState legacy;
    static GameState migrated;
    static GameState reloaded;
    memset(&legacy, 0, sizeof(legacy));
    memset(&migrated, 0, sizeof(migrated));
    memset(&reloaded, 0, sizeof(reloaded));
    legacy.player.player_class = CLASS_WARRIOR;
    game_init(&legacy);

    legacy.inventory_count = 1;
    legacy.inventory[0] = item_make_magic_greatsword();
    legacy.inventory[0].attack_bonus = 14;
    legacy.inventory[0].value = 700;
    legacy.inventory[0].cleave_percent = 50;
    legacy.equipped_main_hand = 0;
    legacy.equipped_off_hand = -1;
    legacy.equipped_armor = -1;
    legacy.player.attack = 28;
    legacy.floor_item_count = 1;
    legacy.floor_items[0].active = 1;
    legacy.floor_items[0].x = 2;
    legacy.floor_items[0].y = 2;
    legacy.floor_items[0].underlying_tile = TILE_TOWN_FLOOR;
    legacy.floor_items[0].item = item_make_magic_longbow();
    legacy.floor_items[0].item.attack_bonus = 10;
    legacy.floor_items[0].item.value = 800;
    legacy.floor_items[0].item.range = 10;
    legacy.floor_items[0].item.pierces_targets = 0;

    remove_test_save(LEGACY_SLOT);
    remove_test_save(MIGRATED_SLOT);
    int legacy_saved = save_game(&legacy, LEGACY_SLOT);
    int version_changed = legacy_saved &&
        rewrite_save_version(LEGACY_SLOT, 37);
    int migrated_ok = version_changed && load_game(&migrated, LEGACY_SLOT);
    ASSERT("version 37 weapon save migrates", migrated_ok);
    if (!migrated_ok) {
        remove_test_save(LEGACY_SLOT);
        remove_test_save(MIGRATED_SLOT);
        return;
    }

    Item expected_weapon = item_make_magic_greatsword();
    Item expected_drop = item_make_magic_longbow();
    ASSERT("migrated equipped weapon uses current fields",
        weapon_fields_match(&migrated.inventory[0], &expected_weapon));
    ASSERT("migration preserves equipped attack total",
        migrated.player.attack == 14 + expected_weapon.attack_bonus);
    ASSERT("migrated floor weapon uses current fields",
        weapon_fields_match(&migrated.floor_items[0].item, &expected_drop));

    int migrated_saved = save_game(&migrated, MIGRATED_SLOT);
    int reloaded_ok = migrated_saved && load_game(&reloaded, MIGRATED_SLOT);
    ASSERT("migrated save can be loaded again", reloaded_ok);
    if (reloaded_ok) {
        ASSERT("migrated weapon fields remain stable after another save/load",
            weapon_fields_match(&reloaded.inventory[0], &expected_weapon) &&
            weapon_fields_match(&reloaded.floor_items[0].item,
                &expected_drop));
        ASSERT("migrated equipment and attack remain stable",
            reloaded.equipped_main_hand == 0 &&
            reloaded.player.attack == migrated.player.attack);
    }

    remove_test_save(LEGACY_SLOT);
    remove_test_save(MIGRATED_SLOT);
}

static void test_harbor_relocation(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    for (int y = TOWN_HARBOR_Y; y < TOWN_H - 1; y++) {
        for (int x = TOWN_HARBOR_X; x < TOWN_W - 1; x++) {
            original.map.tiles[y][x] = TILE_TOWN_FLOOR;
        }
    }
    for (int y = 16; y <= 21; y++) {
        for (int x = 28; x <= 32; x++) {
            original.map.tiles[y][x] = TILE_WATCHTOWER;
        }
    }
    original.player.x = TOWN_HARBOR_X;
    original.player.y = TOWN_HARBOR_Y;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = TOWN_W - 2, .y = TOWN_H - 2,
        .underlying_tile = TILE_TOWN_FLOOR,
        .item = item_make_health_potion()
    };
    original.map.tiles[TOWN_H - 2][TOWN_W - 2] = TILE_ITEM;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 45) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy town harbor layout can be loaded", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading moves the harbor and clears its former lot",
            loaded.map.tiles[TOWN_HARBOR_Y][TOWN_HARBOR_X] ==
                TILE_WATCHTOWER &&
            loaded.map.tiles[TOWN_H - 2][TOWN_W - 2] == TILE_WATCHTOWER &&
            loaded.map.tiles[16][28] == TILE_TOWN_FLOOR);
        ASSERT("harbor relocation keeps the player on accessible ground",
            loaded.player.x == 28 && loaded.player.y == 16 &&
            map_is_walkable(&loaded.map, loaded.player.x, loaded.player.y));
        ASSERT("harbor relocation preserves covered loot in the former lot",
            loaded.floor_items[0].active && loaded.floor_items[0].x == 32 &&
            loaded.floor_items[0].y == 19 && loaded.map.tiles[19][32] == TILE_ITEM &&
            loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR);
        ASSERT("relocated harbor and loot survive another save/load",
            save_game(&loaded, MIGRATED_SLOT) && load_game(&original, MIGRATED_SLOT) &&
            original.map.tiles[TOWN_H - 2][TOWN_W - 2] == TILE_WATCHTOWER &&
            original.floor_items[0].x == 32 && original.floor_items[0].y == 19);
    }
    remove_test_save(LEGACY_SLOT);
    remove_test_save(MIGRATED_SLOT);
}

static void test_legacy_coast_sluice_removal(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    original.location = LOCATION_COAST;
    original.level = 2;
    map_generate_coast(&original.map, original.level);
    Room *room = &original.map.rooms[original.map.room_count / 2];
    int cx;
    int cy;
    map_room_center(room, &cx, &cy);
    for (int y = room->y; y < room->y + room->h; y++) {
        original.map.tiles[y][cx] = TILE_COAST_WALL;
    }
    original.map.tiles[cy - 2][cx] = TILE_COAST_DEEP_WATER;
    original.map.tiles[cy + 2][cx] = TILE_COAST_CHANNEL_DRY;
    original.map.tiles[cy][cx + 2] = TILE_COAST_SLUICE_CONTROL;
    original.player.x = cx + 2;
    original.player.y = cy;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = cx + 2, .y = cy,
        .underlying_tile = TILE_COAST_SLUICE_CONTROL,
        .item = item_make_health_potion()
    };
    original.coast_cache[1].valid = 1;
    original.coast_cache[1].map = original.map;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 47) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy Coast save with a sluice loads", loaded_ok);
    if (loaded_ok) {
        int open = 1;
        for (int y = room->y; y < room->y + room->h; y++) {
            open &= loaded.map.tiles[y][cx] == TILE_COAST_FLOOR;
        }
        ASSERT("loading clears the active sluice wall and switch",
            open && loaded.map.tiles[cy][cx + 2] == TILE_ITEM);
        ASSERT("loading clears the cached sluice wall and switch",
            loaded.coast_cache[1].map.tiles[cy][cx] == TILE_COAST_FLOOR &&
            loaded.coast_cache[1].map.tiles[cy][cx + 2] == TILE_COAST_FLOOR);
        ASSERT("sluice migration preserves nearby loot and its floor",
            loaded.floor_items[0].active &&
            loaded.floor_items[0].underlying_tile == TILE_COAST_FLOOR &&
            loaded.map.tiles[cy][cx + 2] == TILE_ITEM);
        int control_x;
        int control_y;
        map_room_center(&loaded.map.rooms[0], &control_x, &control_y);
        ASSERT("sluice migration retains the main tide control",
            loaded.map.tiles[control_y][control_x] == TILE_COAST_TIDE_CONTROL);
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_forest_enemy_repair(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    original.location = LOCATION_FOREST;
    original.level = 2;
    map_generate_forest(&original.map, original.level);
    original.player.x = original.map.stairs_up_x;
    original.player.y = original.map.stairs_up_y;
    int wall_x = -1;
    int wall_y = -1;
    for (int y = 1; y < MAP_H - 1 && wall_x < 0; y++) {
        for (int x = 1; x < MAP_W - 1; x++) {
            if (original.map.tiles[y][x] == TILE_FOREST_WALL &&
                map_is_walkable(&original.map, x - 1, y)) {
                wall_x = x;
                wall_y = y;
                break;
            }
        }
    }
    ASSERT("forest test finds a tree beside a path", wall_x >= 0);
    if (wall_x < 0) {
        return;
    }
    int pocket_x = -1;
    int pocket_y = -1;
    for (int y = 1; y < MAP_H - 1 && pocket_x < 0; y++) {
        for (int x = 1; x < MAP_W - 1; x++) {
            if (original.map.tiles[y][x] == TILE_FOREST_WALL &&
                original.map.tiles[y - 1][x] == TILE_FOREST_WALL &&
                original.map.tiles[y + 1][x] == TILE_FOREST_WALL &&
                original.map.tiles[y][x - 1] == TILE_FOREST_WALL &&
                original.map.tiles[y][x + 1] == TILE_FOREST_WALL) {
                pocket_x = x;
                pocket_y = y;
                break;
            }
        }
    }
    ASSERT("forest test finds an isolated tree pocket", pocket_x >= 0);
    if (pocket_x < 0) {
        return;
    }
    original.map.tiles[pocket_y][pocket_x] = TILE_FOREST_FLOOR;
    original.enemy_count = 3;
    for (int i = 0; i < original.enemy_count; i++) {
        original.enemies[i].active = 1;
        original.enemies[i].type = ENEMY_PIXIE;
        original.enemies[i].x = i == 2 ? pocket_x : wall_x;
        original.enemies[i].y = i == 2 ? pocket_y : wall_y;
        original.enemies[i].hp = 7;
        original.enemies[i].max_hp = 7;
    }
    original.forest_cache[1].valid = 1;
    original.forest_cache[1].map = original.map;
    original.forest_cache[1].enemy_count = original.enemy_count;
    memcpy(original.forest_cache[1].enemies, original.enemies,
        sizeof(Enemy) * original.enemy_count);

    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        load_game(&loaded, LEGACY_SLOT);
    ASSERT("forest save with stranded enemies loads", loaded_ok);
    if (loaded_ok) {
        ASSERT("loaded forest enemies move onto separate path tiles",
            loaded.enemies[0].active && loaded.enemies[1].active &&
            loaded.map.tiles[loaded.enemies[0].y][loaded.enemies[0].x] == TILE_FOREST_FLOOR &&
            loaded.map.tiles[loaded.enemies[1].y][loaded.enemies[1].x] == TILE_FOREST_FLOOR &&
            !(loaded.enemies[0].x == loaded.enemies[1].x &&
            loaded.enemies[0].y == loaded.enemies[1].y));
        ASSERT("enemy in an isolated floor pocket moves onto the path",
            loaded.enemies[2].active &&
            !(loaded.enemies[2].x == pocket_x && loaded.enemies[2].y == pocket_y) &&
            loaded.map.tiles[loaded.enemies[2].y][loaded.enemies[2].x] == TILE_FOREST_FLOOR);
        ASSERT("cached forest enemies are repaired for revisits",
            loaded.forest_cache[1].valid &&
            loaded.forest_cache[1].map.tiles
                [loaded.forest_cache[1].enemies[0].y]
                [loaded.forest_cache[1].enemies[0].x] == TILE_FOREST_FLOOR);
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_blocked_dungeon_gate_repair(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    original.location = LOCATION_DUNGEON;
    original.level = 2;
    original.player.x = 2;
    original.player.y = 5;
    Map *m = &original.map;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            m->tiles[y][x] = TILE_WALL;
        }
    }
    for (int x = 2; x <= 8; x++) {
        m->tiles[5][x] = TILE_FLOOR;
    }
    m->stairs_up_x = 2;
    m->stairs_up_y = 5;
    m->stairs_down_x = 8;
    m->stairs_down_y = 5;
    m->tiles[5][2] = TILE_STAIRS_UP;
    m->tiles[5][5] = TILE_DUNGEON_GATE;
    m->tiles[5][7] = TILE_DUNGEON_SWITCH_OFF;
    m->tiles[5][8] = TILE_STAIRS_DOWN;
    original.level_cache[1].valid = 1;
    original.level_cache[1].map = *m;

    remove_test_save(BLOCKED_DUNGEON_SLOT);
    int loaded_ok = save_game(&original, BLOCKED_DUNGEON_SLOT) &&
        load_game(&loaded, BLOCKED_DUNGEON_SLOT);
    ASSERT("blocked dungeon gate save can be loaded", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading opens a gate with no accessible switch",
            loaded.map.tiles[5][5] == TILE_FLOOR &&
            loaded.map.tiles[5][7] == TILE_FLOOR &&
            map_is_walkable(&loaded.map, 5, 5));
        ASSERT("loading repairs the cached blocked dungeon floor",
            loaded.level_cache[1].valid &&
            loaded.level_cache[1].map.tiles[5][5] == TILE_FLOOR &&
            loaded.level_cache[1].map.tiles[5][7] == TILE_FLOOR);
    }
    remove_test_save(BLOCKED_DUNGEON_SLOT);

    m->tiles[5][7] = TILE_FLOOR;
    m->tiles[5][3] = TILE_DUNGEON_SWITCH_OFF;
    ASSERT("gate remains when its switch is reachable before the exit",
        map_repair_dungeon_routes(m) == 0 &&
        m->tiles[5][5] == TILE_DUNGEON_GATE);
}

static void test_cain_save_load(void) {
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    ASSERT("save before Cain's gift succeeds", save_game(&g, ROUND_TRIP_SLOT));
    loaded.cain_scroll_given = 1;
    ASSERT("load preserves the unclaimed gift",
        load_game(&loaded, ROUND_TRIP_SLOT) && !loaded.cain_scroll_given);
    game_talk_to_cain(&loaded);
    int count = loaded.inventory_count;
    ASSERT("save after Cain's gift succeeds", save_game(&loaded, ROUND_TRIP_SLOT));
    ASSERT("load preserves the claimed gift and Cain's dialogue",
        load_game(&g, ROUND_TRIP_SLOT) && g.cain_scroll_given &&
        g.inventory_count == count && g.dialogue_active &&
        strcmp(g.dialogue_speaker, "Cain") == 0 &&
        strcmp(g.dialogue_text, loaded.dialogue_text) == 0 &&
        g.dialogue_x == TOWN_CAIN_X && g.dialogue_y == TOWN_CAIN_Y);
    game_talk_to_cain(&g);
    ASSERT("reloading cannot duplicate Cain's gift", g.inventory_count == count);

    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_harbor_road_save_load(void) {
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.defeated_bosses = (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST) |
        (1 << LOCATION_MOUNTAINS) | (1 << LOCATION_COAST);
    g.elowen_quest_state = 3;
    g.dain_quest_state = 3;
    g.alder_quest_state = 3;
    g.mara_quest_state = 3;
    game_enter_tavern(&g);
    game_leave_tavern(&g);
    ASSERT("unlocked harbor road can be saved", save_game(&g, ROUND_TRIP_SLOT));
    int ok = load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("loading preserves harbor eligibility and the road",
        ok && game_harbor_unlocked(&loaded) &&
        loaded.map.tiles[TOWN_HARBOR_Y + 1][TOWN_HARBOR_X - 1] == TILE_TOWN_PATH &&
        loaded.map.tiles[TOWN_HARBOR_ENTRANCE_Y]
            [TOWN_HARBOR_ENTRANCE_X] == TILE_TOWN_PATH);
    if (ok) {
        ASSERT("saving before the gift leaves the map unclaimed", !game_has_treasure_map(&loaded));
        game_talk_to_rowan(&loaded);
        int count = loaded.inventory_count;
        ASSERT("the newly gifted map can be saved",
            game_has_treasure_map(&loaded) && save_game(&loaded, ROUND_TRIP_SLOT));
        int map_loaded = load_game(&g, ROUND_TRIP_SLOT);
        ASSERT("loading preserves the treasure map item",
            map_loaded && game_has_treasure_map(&g) && g.inventory_count == count &&
            strcmp(g.inventory[count - 1].name, "Island Treasure Map") == 0);
        if (map_loaded) {
            game_talk_to_rowan(&g);
            ASSERT("reloading cannot duplicate Rowan's gift", g.inventory_count == count);
        }
        game_enter_tavern(&loaded);
        game_leave_tavern(&loaded);
        ASSERT("saved progress rebuilds the road on later town visits",
            loaded.map.tiles[TOWN_HARBOR_Y + 1][21] == TILE_TOWN_PATH);
        game_talk_to_rowan(&loaded);
        ASSERT("returning to town keeps the map without repeating the gift",
            game_has_treasure_map(&loaded) && loaded.inventory_count == count);
    }
    remove_test_save(ROUND_TRIP_SLOT);
}

void test_save_load(void) {
    test_harbor_road_save_load();
    test_cain_save_load();
    printf("Save/load tests:\n");
    test_current_weapon_round_trip();
    test_dual_wield_round_trip();
    test_legacy_off_hand_migration();
    test_legacy_goblin_reward_migration();
    test_migrated_weapon_round_trip();
    test_migrated_armor_round_trip();
    test_harbor_relocation();
    test_legacy_coast_sluice_removal();
    test_forest_enemy_repair();
    test_blocked_dungeon_gate_repair();
}
