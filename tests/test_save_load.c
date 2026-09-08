#include "test_utils.h"
#include "../src/game/game.h"
#include "../src/systems/save_load.h"
#include "../external/cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUND_TRIP_SLOT 99001
#define LEGACY_SLOT 99002
#define MIGRATED_SLOT 99003
#define LEGACY_ARMOR_SLOT 99004
#define MIGRATED_ARMOR_SLOT 99005

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
    original.inventory[9] = (Item){0};
    original.inventory[9].active = 1;
    original.inventory[9].type = ITEM_SHIELD;
    strncpy(original.inventory[9].name, "Test Shield",
        sizeof(original.inventory[9].name) - 1);
    original.inventory[9].defense_bonus = 2;
    original.inventory[9].value = 75;
    original.inventory[9].rarity = ITEM_RARITY_COMMON;
    original.inventory[9].class_mask = ITEM_CLASS_WARRIOR;
    original.inventory[9].visual_id = ITEM_VISUAL_SHIELD_GENERIC;
    original.inventory[9].block_chance = 10;
    original.inventory[9].block_reduction_percent = 50;
    original.equipped_main_hand = 0;
    original.equipped_off_hand = 9;
    original.equipped_armor = 6;

    remove_test_save(ROUND_TRIP_SLOT);
    int saved = save_game(&original, ROUND_TRIP_SLOT);
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
    remove_test_save(ROUND_TRIP_SLOT);
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

void test_save_load(void) {
    printf("Save/load tests:\n");
    test_current_weapon_round_trip();
    test_migrated_weapon_round_trip();
    test_migrated_armor_round_trip();
}
