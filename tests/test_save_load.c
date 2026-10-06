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

// Deletes a top-level or player.field value to reproduce an older save.
static int remove_save_field(int slot, const char *field) {
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
    cJSON *parent = root;
    if (strncmp(field, "player.", 7) == 0) {
        parent = cJSON_GetObjectItem(root, "player");
        field += 7;
    }
    if (!root || !cJSON_GetObjectItem(parent, field)) {
        cJSON_Delete(root);
        return 0;
    }
    cJSON_DeleteItemFromObject(parent, field);
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

static void test_message_kinds_round_trip(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    original.message_count = 0;
    push_message(&original, "You enter the forest.");
    push_message_kind(&original, "Goblin: 4 dmg", MESSAGE_DAMAGE_TAKEN);
    push_message_kind(&original, "Critical hit Goblin: 18 dmg", MESSAGE_CRITICAL);
    int loaded_ok = save_game(&original, ROUND_TRIP_SLOT) &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("message colours survive save/load",
        loaded_ok && loaded.message_count == 3 &&
        loaded.message_kinds[0] == MESSAGE_NORMAL &&
        loaded.message_kinds[1] == MESSAGE_DAMAGE_TAKEN &&
        loaded.message_kinds[2] == MESSAGE_CRITICAL);
    int legacy_ok = remove_save_field(ROUND_TRIP_SLOT, "message_kinds") &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("saves without message colours load as plain messages",
        legacy_ok && loaded.message_count == 3 &&
        loaded.message_kinds[1] == MESSAGE_NORMAL &&
        loaded.message_kinds[2] == MESSAGE_NORMAL &&
        strcmp(loaded.messages[2], "Critical hit Goblin: 18 dmg") == 0);
    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_key_bindings_round_trip(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    controls_assign(original.key_bindings, CONTROL_INVENTORY, SDL_SCANCODE_K);
    controls_assign(original.key_bindings, CONTROL_TALK, SDL_SCANCODE_W);
    int loaded_ok = save_game(&original, ROUND_TRIP_SLOT) &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("remapped keys survive save/load",
        loaded_ok && memcmp(loaded.key_bindings, original.key_bindings,
            sizeof(original.key_bindings)) == 0);

    int defaults[CONTROL_COUNT];
    controls_reset(defaults);
    controls_assign(loaded.key_bindings, CONTROL_HELP, SDL_SCANCODE_J);
    int legacy_ok = remove_save_field(ROUND_TRIP_SLOT, "key_bindings") &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("a save without controls loads the defaults, not the current keys",
        legacy_ok && memcmp(loaded.key_bindings, defaults, sizeof(defaults)) == 0);

    original.key_bindings[CONTROL_TALK] = original.key_bindings[CONTROL_PICK_UP];
    int broken_ok = save_game(&original, ROUND_TRIP_SLOT) &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("a save with duplicate keys loads the defaults",
        broken_ok && memcmp(loaded.key_bindings, defaults, sizeof(defaults)) == 0);
    remove_test_save(ROUND_TRIP_SLOT);
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
    original.player.known_spell_count = 2;
    original.player.known_spells[0] = spell_make_magic_arrow();
    spell_upgrade(&original.player.known_spells[0]);
    original.player.known_spells[1] = spell_make_heal();
    spell_upgrade(&original.player.known_spells[1]);
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
    ASSERT("Heal rank and reduced mana cost survive save/load",
        loaded.player.known_spells[1].rank == 2 &&
        loaded.player.known_spells[1].mp_cost == 13);
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

static void test_legacy_town3_moat(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    game_leave_crownroad(&original, LOCATION_CASTLE);
    original.map.tiles[11][CROWNROAD_X - 1] = TILE_TOWN_FLOOR;
    original.map.tiles[5][TOWN_MOAT_X] = TILE_ITEM;
    original.player.x = CROWNROAD_X - 1;
    original.player.y = 11;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = TOWN_MOAT_X, .y = 5,
        .underlying_tile = TILE_TOWN_FLOOR,
        .item = item_make_health_potion()
    };
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 60) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy castle save loads", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading surrounds the castle with the moat",
            !map_is_walkable(&loaded.map, CROWNROAD_X - 1, 11) &&
            !map_is_walkable(&loaded.map, TOWN_MOAT_X, 5) &&
            map_is_walkable(&loaded.map, CROWNROAD_X, 11));
        ASSERT("moat migration moves the player onto the plaza",
            loaded.player.x == CROWNROAD_X - 1 && loaded.player.y == 12);
        ASSERT("moat migration moves loot onto the bank",
            loaded.floor_items[0].active &&
            loaded.floor_items[0].x == TOWN_MOAT_X - 1 &&
            loaded.floor_items[0].y == 5 &&
            loaded.map.tiles[5][TOWN_MOAT_X - 1] == TILE_ITEM &&
            loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR);
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_legacy_tavern_move(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    for (int y = TOWN_TAVERN_Y; y < TOWN_TAVERN_Y + TOWN_TAVERN_H; y++) {
        for (int x = TOWN_TAVERN_X; x < TOWN_TAVERN_X + TOWN_TAVERN_W; x++) {
            original.map.tiles[y][x] = TILE_TOWN_FLOOR;
        }
    }
    for (int y = 16; y <= 20; y++) {
        for (int x = 5; x <= 11; x++) {
            original.map.tiles[y][x] = TILE_TAVERN;
        }
    }
    original.map.tiles[20][8] = TILE_TAVERN_DOOR;
    original.player.x = TOWN_TAVERN_X + 1;
    original.player.y = TOWN_TAVERN_Y + 2;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = TOWN_TAVERN_DOOR_X, .y = TOWN_TAVERN_Y,
        .underlying_tile = TILE_TOWN_FLOOR,
        .item = item_make_health_potion()
    };
    original.map.tiles[TOWN_TAVERN_Y][TOWN_TAVERN_DOOR_X] = TILE_ITEM;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 61) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy town save with the south-west Tavern loads", loaded_ok);
    if (loaded_ok) {
        int front_y = TOWN_TAVERN_Y + TOWN_TAVERN_H;
        ASSERT("loading moves the Tavern beside the Blacksmith",
            loaded.map.tiles[TOWN_TAVERN_Y][TOWN_TAVERN_X] == TILE_TAVERN &&
            loaded.map.tiles[TOWN_TAVERN_DOOR_Y][TOWN_TAVERN_DOOR_X] == TILE_TAVERN_DOOR &&
            loaded.map.tiles[16][5] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[20][8] == TILE_TOWN_FLOOR);
        ASSERT("Tavern move puts the player on the square in front of it",
            loaded.player.x == TOWN_TAVERN_X + 1 && loaded.player.y == front_y &&
            map_is_walkable(&loaded.map, loaded.player.x, loaded.player.y));
        ASSERT("Tavern move carries loot from the new lot onto the square",
            loaded.floor_items[0].active &&
            loaded.floor_items[0].x == TOWN_TAVERN_DOOR_X &&
            loaded.floor_items[0].y == front_y &&
            loaded.floor_items[0].underlying_tile == TILE_TOWN_PATH &&
            loaded.map.tiles[front_y][TOWN_TAVERN_DOOR_X] == TILE_ITEM);
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_legacy_town_square(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    const int rows[3] = {11, 13, 14};
    const int cols[4] = {TOWN_BLACKSMITH_X, TOWN_BLACKSMITH_X + 1,
        TOWN_ALCHEMIST_X + 3, TOWN_ALCHEMIST_X + 4};
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            original.map.tiles[rows[r]][cols[c]] = TILE_TOWN_FLOOR;
        }
    }
    for (int y = 13; y <= 21; y++) {
        original.map.tiles[y][4] = TILE_TOWN_PATH;
        original.map.tiles[y][12] = TILE_TOWN_PATH;
    }
    for (int x = 5; x < 12; x++) {
        original.map.tiles[21][x] = TILE_TOWN_PATH;
    }
    original.player.x = 12;
    original.player.y = 16;
    original.floor_item_count = 2;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = 4, .y = 18, .underlying_tile = TILE_TOWN_PATH,
        .item = item_make_health_potion()
    };
    original.floor_items[1] = (FloorItem){
        .active = 1, .x = TOWN_ALCHEMIST_X + 4, .y = 13,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()
    };
    original.map.tiles[18][4] = TILE_ITEM;
    original.map.tiles[13][TOWN_ALCHEMIST_X + 4] = TILE_ITEM;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 62) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy town save with the Tavern loop loads", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading removes the old Tavern walkway loop",
            loaded.map.tiles[13][4] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[20][12] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[21][8] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[18][4] == TILE_ITEM &&
            loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR);
        ASSERT("loading widens the square to the Blacksmith and Alchemist walls",
            loaded.map.tiles[11][TOWN_BLACKSMITH_X] == TILE_TOWN_PATH &&
            loaded.map.tiles[13][12] == TILE_TOWN_PATH &&
            loaded.map.tiles[14][TOWN_ALCHEMIST_X + 4] == TILE_TOWN_PATH &&
            loaded.map.tiles[13][TOWN_ALCHEMIST_X + 4] == TILE_ITEM &&
            loaded.floor_items[1].underlying_tile == TILE_TOWN_PATH);
        ASSERT("square migration keeps Cain and the player in place",
            loaded.map.tiles[TOWN_CAIN_Y][TOWN_CAIN_X] == TILE_NPC_CAIN &&
            loaded.player.x == 12 && loaded.player.y == 16 &&
            map_is_walkable(&loaded.map, 12, 16));
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_legacy_inn_move(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    game_enter_town2(&original);
    for (int y = TOWN_INN_Y; y < TOWN_INN_Y + TOWN_INN_H; y++) {
        for (int x = TOWN_INN_X; x < TOWN_INN_X + TOWN_INN_W; x++) {
            original.map.tiles[y][x] = TILE_TOWN_FLOOR;
        }
    }
    for (int y = 16; y <= 20; y++) {
        for (int x = 5; x <= 11; x++) {
            original.map.tiles[y][x] = TILE_TAVERN;
        }
    }
    original.map.tiles[20][8] = TILE_TAVERN_DOOR;
    for (int y = 14; y <= 21; y++) {
        original.map.tiles[y][12] = TILE_TOWN_PATH;
    }
    for (int x = 8; x < 12; x++) {
        original.map.tiles[21][x] = TILE_TOWN_PATH;
    }
    original.player.x = TOWN_INN_X + 1;
    original.player.y = TOWN_INN_Y + 2;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = TOWN_INN_DOOR_X, .y = TOWN_INN_Y,
        .underlying_tile = TILE_TOWN_FLOOR,
        .item = item_make_health_potion()
    };
    original.map.tiles[TOWN_INN_Y][TOWN_INN_DOOR_X] = TILE_ITEM;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 63) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy Town 2 save with the south-west Inn loads", loaded_ok);
    if (loaded_ok) {
        int front_y = TOWN_INN_Y + TOWN_INN_H;
        ASSERT("loading moves the Inn beside the Healer",
            loaded.location == LOCATION_TOWN2 &&
            loaded.map.tiles[TOWN_INN_Y][TOWN_INN_X] == TILE_TAVERN &&
            loaded.map.tiles[TOWN_INN_DOOR_Y][TOWN_INN_DOOR_X] == TILE_TAVERN_DOOR &&
            loaded.map.tiles[16][5] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[18][12] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[21][9] == TILE_TOWN_FLOOR);
        ASSERT("Inn move puts the player and loot on the square in front of it",
            loaded.player.x == TOWN_INN_X + 1 && loaded.player.y == front_y &&
            loaded.floor_items[0].active &&
            loaded.floor_items[0].x == TOWN_INN_DOOR_X &&
            loaded.floor_items[0].y == front_y &&
            loaded.floor_items[0].underlying_tile == TILE_TOWN_PATH &&
            loaded.map.tiles[front_y][TOWN_INN_DOOR_X] == TILE_ITEM);
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_legacy_apothecary(void) {
    static GameState original;
    static GameState loaded;
    const int old_apothecary_x = 31;
    const int old_apothecary_y = 8;
    const int old_apothecary_w = 5;
    const int old_apothecary_h = 4;
    const int old_apothecary_door_x = old_apothecary_x + 2;
    const int old_apothecary_door_y = old_apothecary_y + old_apothecary_h - 1;
    const int old_apothecary_lane_y = old_apothecary_y + old_apothecary_h;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    game_leave_crownroad(&original, LOCATION_TOWN3);
    for (int y = old_apothecary_y; y < old_apothecary_lane_y; y++) {
        for (int x = old_apothecary_x; x < old_apothecary_x + old_apothecary_w; x++) {
            original.map.tiles[y][x] = TILE_TOWN_FLOOR;
        }
    }
    for (int x = 29; x < old_apothecary_x + old_apothecary_w; x++) {
        original.map.tiles[old_apothecary_lane_y][x] = TILE_TOWN_FLOOR;
    }
    original.player.x = old_apothecary_x + 1;
    original.player.y = old_apothecary_y + 1;
    original.floor_item_count = 2;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = old_apothecary_door_x, .y = old_apothecary_y,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()
    };
    original.floor_items[1] = (FloorItem){
        .active = 1, .x = 30, .y = old_apothecary_lane_y,
        .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_mana_potion()
    };
    original.map.tiles[old_apothecary_y][old_apothecary_door_x] = TILE_ITEM;
    original.map.tiles[old_apothecary_lane_y][30] = TILE_ITEM;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 64) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy Town 3 save with the Apothecary on a side road loads", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading moves the Apothecary onto the north edge of the square",
            loaded.map.tiles[TOWN_APOTHECARY_Y][TOWN_APOTHECARY_X] == TILE_SHOP_ALCHEMIST &&
            loaded.map.tiles[TOWN_APOTHECARY_DOOR_Y][TOWN_APOTHECARY_DOOR_X] ==
                TILE_ALCHEMIST_DOOR &&
            loaded.map.tiles[TOWN_APOTHECARY_DOOR_Y + 1][TOWN_APOTHECARY_DOOR_X] ==
                TILE_TOWN_PATH &&
            loaded.map.tiles[old_apothecary_y][old_apothecary_x] == TILE_TOWN_FLOOR &&
            loaded.map.tiles[CASTLE_ROAD_Y][6] == TILE_TOWN_PATH);
        ASSERT("Apothecary migration moves the player out of its former lot",
            loaded.player.x == old_apothecary_x + 1 &&
            loaded.player.y == old_apothecary_lane_y &&
            map_is_walkable(&loaded.map, loaded.player.x, loaded.player.y));
        ASSERT("Apothecary migration keeps old-lot loot visible on cleared ground",
            loaded.floor_items[0].x == old_apothecary_door_x &&
            loaded.floor_items[0].y == old_apothecary_lane_y &&
            loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR &&
            loaded.map.tiles[old_apothecary_lane_y][old_apothecary_door_x] == TILE_ITEM &&
            loaded.floor_items[1].underlying_tile == TILE_TOWN_FLOOR &&
            loaded.map.tiles[old_apothecary_lane_y][30] == TILE_ITEM);
    }
    remove_test_save(LEGACY_SLOT);
}

static void test_legacy_fireball_scroll(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);
    Item legacy_scroll = item_make_scroll_fireball();
    legacy_scroll.class_mask = 0;
    int slot = original.inventory_count;
    original.inventory[original.inventory_count++] = legacy_scroll;
    int x = original.player.x;
    int y = original.player.y;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = x, .y = y, .underlying_tile = original.map.tiles[y][x],
        .item = legacy_scroll
    };
    original.map.tiles[y][x] = TILE_ITEM;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 65) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("legacy save with an unrestricted Fireball scroll loads", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading makes carried and dropped Fireball scrolls Mage-only",
            loaded.inventory[slot].class_mask == ITEM_CLASS_MAGE &&
            loaded.floor_item_count == 1 &&
            loaded.floor_items[0].item.class_mask == ITEM_CLASS_MAGE);
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

static void test_retired_dungeon_gates_removed(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    original.location = LOCATION_DUNGEON;
    original.level = 3;
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
    m->tiles[5][4] = TILE_DUNGEON_SWITCH_ON;
    m->tiles[5][5] = TILE_DUNGEON_GATE;
    m->tiles[5][7] = TILE_DUNGEON_SWITCH_OFF;
    m->tiles[5][8] = TILE_STAIRS_DOWN;
    original.level_cache[2].valid = 1;
    original.level_cache[2].map = *m;
    // A potion dropped on the used switch keeps the switch as its underlay.
    m->tiles[5][4] = TILE_ITEM;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = 4, .y = 5, .underlying_tile = TILE_DUNGEON_SWITCH_ON,
        .item = item_make_health_potion()
    };

    remove_test_save(BLOCKED_DUNGEON_SLOT);
    int loaded_ok = save_game(&original, BLOCKED_DUNGEON_SLOT) &&
        rewrite_save_version(BLOCKED_DUNGEON_SLOT, 67) &&
        load_game(&loaded, BLOCKED_DUNGEON_SLOT);
    ASSERT("a save from before the portcullis was retired still loads", loaded_ok);
    if (loaded_ok) {
        ASSERT("loading turns the floor's gate and switches into plain floor",
            loaded.map.tiles[5][5] == TILE_FLOOR &&
            loaded.map.tiles[5][7] == TILE_FLOOR &&
            map_is_walkable(&loaded.map, 5, 5) &&
            loaded.map.tiles[5][4] == TILE_ITEM &&
            loaded.floor_items[0].underlying_tile == TILE_FLOOR);
        ASSERT("loading clears the gate and switches from cached floors",
            loaded.level_cache[2].valid &&
            loaded.level_cache[2].map.tiles[5][4] == TILE_FLOOR &&
            loaded.level_cache[2].map.tiles[5][5] == TILE_FLOOR &&
            loaded.level_cache[2].map.tiles[5][7] == TILE_FLOOR);
    }
    remove_test_save(BLOCKED_DUNGEON_SLOT);
}

// Version 68 saves come from the old world: Royal Guards and a south swamp gate
// in Town 2, the castle town as Town 3, and a north-south Crownroad.
static void test_legacy_king_road_world(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    game_enter_town2(&original);
    original.map.tiles[4][19] = TILE_NPC_ROYAL_GUARD;
    original.map.tiles[4][21] = TILE_NPC_ROYAL_GUARD;
    original.map.tiles[TOWN_H - 1][20] = TILE_TOWN_EXIT;
    int loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 68) && load_game(&loaded, LEGACY_SLOT);
    int guards = 0;
    for (int y = 0; y < TOWN_H; y++) {
        for (int x = 0; x < TOWN_W; x++) {
            guards += loaded.map.tiles[y][x] == TILE_NPC_ROYAL_GUARD;
        }
    }
    ASSERT("an older Town 2 save loses obsolete guards and opens both region gates",
        loaded_ok && guards == 0 && loaded.map.tiles[TOWN_H - 1][STILLBURY_GLASSDEEP_GATE_X] == TILE_TOWN_EXIT &&
        loaded.map.tiles[0][20] == TILE_TOWN_EXIT && loaded.map.tiles[4][19] == TILE_TOWN_FLOOR);

    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    int spawn_x;
    int spawn_y;
    original.location = LOCATION_TOWN3;
    map_generate_castle(&original.map, &spawn_x, &spawn_y);
    original.player.x = 5;
    original.player.y = 20;
    original.floor_item_count = 1;
    original.floor_items[0] = (FloorItem){
        .active = 1, .x = 6, .y = 20, .underlying_tile = TILE_TOWN_FLOOR,
        .item = item_make_health_potion()
    };
    original.map.tiles[20][6] = TILE_ITEM;
    loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 68) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("an older save in the castle town becomes the new Town 3, keeping player and loot",
        loaded_ok && loaded.location == LOCATION_TOWN3 &&
        loaded.map.tiles[TOWN3_KING_GATE_Y][TOWN_W - 1] == TILE_TOWN_EXIT &&
        loaded.map.tiles[TOWN3_GUARD_NORTH_Y][TOWN3_GUARD_X] == TILE_NPC_ROYAL_GUARD &&
        map_is_walkable(&loaded.map, TOWN_MOAT_X, 5) &&
        loaded.player.x == 5 && loaded.player.y == 20 &&
        loaded.floor_items[0].active && loaded.map.tiles[20][6] == TILE_ITEM &&
        loaded.floor_items[0].underlying_tile == TILE_TOWN_FLOOR);

    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    original.location = LOCATION_TOWN3;
    map_generate_castle(&original.map, &spawn_x, &spawn_y);
    original.player.x = TOWN_APOTHECARY_X + 1;
    original.player.y = TOWN_APOTHECARY_Y + 1;
    loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 68) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("a player left on ground the new Town 3 builds over moves to its south gate",
        loaded_ok && loaded.player.x == 20 && loaded.player.y == TOWN_H - 2);

    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    game_init(&original);
    original.location = LOCATION_CROWNROAD;
    original.level = 1;
    map_generate_crownroad(&original.map);
    original.player.x = 20;
    original.player.y = 40;
    original.enemy_count = 1;
    original.enemies[0].active = 1;
    original.enemies[0].x = 15;
    original.enemies[0].y = 10;
    original.crownroad_cache.valid = 1;
    original.crownroad_cache.enemy_count = 1;
    original.crownroad_cache.enemies[0] = original.enemies[0];
    original.crownroad_cache.enemies[0].x = 23;
    original.crownroad_cache.enemies[0].y = 30;
    loaded_ok = save_game(&original, LEGACY_SLOT) &&
        rewrite_save_version(LEGACY_SLOT, 68) && load_game(&loaded, LEGACY_SLOT);
    ASSERT("an older Crownroad save turns the player and its creatures to run west to east",
        loaded_ok && loaded.location == LOCATION_CROWNROAD &&
        loaded.player.x == CROWNROAD_W - 1 - 40 && loaded.player.y == 20 &&
        loaded.enemies[0].x == CROWNROAD_W - 1 - 10 && loaded.enemies[0].y == 15 &&
        loaded.crownroad_cache.enemies[0].x == CROWNROAD_W - 1 - 30 &&
        loaded.crownroad_cache.enemies[0].y == 23 &&
        loaded.map.tiles[CROWNROAD_Y][0] == TILE_TOWN_EXIT &&
        map_is_walkable(&loaded.map, loaded.player.x, loaded.player.y));

    loaded_ok = save_game(&loaded, LEGACY_SLOT) && load_game(&original, LEGACY_SLOT);
    ASSERT("a current save of Crown Road East is not turned a second time",
        loaded_ok && original.player.x == loaded.player.x &&
        original.player.y == loaded.player.y &&
        original.crownroad_cache.enemies[0].x == loaded.crownroad_cache.enemies[0].x);
    remove_test_save(LEGACY_SLOT);
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
    g.defeated_bosses = 1 << LOCATION_COAST;
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
        game_enter_island(&loaded);
        ASSERT("first voyage uses the map as a one-time key",
            loaded.island_travel_unlocked && !game_has_treasure_map(&loaded) &&
            loaded.inventory_count == count - 1);
        game_leave_island(&loaded);
        ASSERT("unlocked island route can be saved",
            save_game(&loaded, ROUND_TRIP_SLOT));
        int route_loaded = load_game(&g, ROUND_TRIP_SLOT);
        ASSERT("loading preserves map-free island travel",
            route_loaded && g.island_travel_unlocked &&
            !game_has_treasure_map(&g) && game_can_sail_to_island(&g) &&
            g.inventory_count == count - 1);
        if (route_loaded) {
            game_talk_to_rowan(&g);
            ASSERT("reloading cannot cause Rowan to reissue the used map",
                !game_has_treasure_map(&g) && g.inventory_count == count - 1);
        }
    }
    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_spent_island_map_on_load(void) {
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.inventory[g.inventory_count++] = item_make_treasure_map();
    g.equipped_armor = g.inventory_count;
    g.inventory[g.inventory_count++] = item_make_chain_mail();
    int original_count = g.inventory_count;
    g.temple_treasure_state = 3;
    g.island_travel_unlocked = 0;
    ASSERT("save with completed island quest and stale map succeeds",
        save_game(&g, ROUND_TRIP_SLOT));
    int loaded_ok = load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("loading restores the island route and removes its spent map",
        loaded_ok && loaded.island_travel_unlocked &&
        !game_has_treasure_map(&loaded) &&
        loaded.inventory_count == original_count - 1 &&
        loaded.equipped_armor == original_count - 2 &&
        loaded.inventory[loaded.equipped_armor].type == ITEM_ARMOR &&
        game_can_sail_to_island(&loaded));
    if (loaded_ok) {
        loaded.inventory[loaded.inventory_count++] = item_make_treasure_map();
        ASSERT("save with unlocked route and duplicate map succeeds",
            save_game(&loaded, ROUND_TRIP_SLOT));
        ASSERT("loading removes a map left after the route was unlocked",
            load_game(&g, ROUND_TRIP_SLOT) &&
            g.island_travel_unlocked && !game_has_treasure_map(&g) &&
            g.inventory_count == original_count - 1);
    }
    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_desert_state_round_trip(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    memset(&loaded, 0, sizeof(loaded));
    for (int i = 0; i < DESERT_DEPTH; i++) {
        original.desert_cache[i].valid = 1;
        original.desert_cache[i].level_cleared = 1;
        original.desert_cache[i].enemy_count = 1;
    }
    game_init(&original);
    int fresh = original.max_desert_level_reached == 1;
    for (int i = 0; i < DESERT_DEPTH; i++) {
        fresh &= !original.desert_cache[i].valid &&
            !original.desert_cache[i].level_cleared &&
            original.desert_cache[i].enemy_count == 0;
    }
    ASSERT("a new game resets all five desert caches and progress", fresh);

    original.max_desert_level_reached = DESERT_DEPTH;
    for (int i = 0; i < DESERT_DEPTH; i++) {
        LevelCache *cache = &original.desert_cache[i];
        cache->valid = i != 2;
        cache->level_cleared = i == DESERT_DEPTH - 1;
        map_generate_desert(&cache->map, i + 1);
        cache->enemy_count = 1;
        cache->enemies[0] = (Enemy){
            .type = ENEMY_SKELETON, .name = "Skeleton", .x = 5 + i, .y = 5,
            .active = i != DESERT_DEPTH - 1, .hp = 10 + i, .max_hp = 20
        };
        loaded.desert_cache[i].valid = 1;
        loaded.desert_cache[i].enemy_count = 7;
    }
    original.max_swamp_level_reached = 2;
    original.max_frostfell_level_reached = 3;
    int loaded_ok = save_game(&original, ROUND_TRIP_SLOT) &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("desert depth survives save/load independently of swamp and frost",
        loaded_ok && loaded.max_desert_level_reached == DESERT_DEPTH &&
        loaded.max_swamp_level_reached == 2 && loaded.max_frostfell_level_reached == 3);
    int caches_ok = loaded_ok;
    for (int i = 0; i < DESERT_DEPTH && loaded_ok; i++) {
        const LevelCache *cache = &loaded.desert_cache[i];
        caches_ok &= cache->valid == original.desert_cache[i].valid &&
            cache->level_cleared == original.desert_cache[i].level_cleared;
        if (cache->valid) {
            caches_ok &= memcmp(cache->map.tiles, original.desert_cache[i].map.tiles,
                sizeof(cache->map.tiles)) == 0 && cache->enemy_count == 1 &&
                cache->enemies[0].hp == 10 + i && cache->enemies[0].x == 5 + i &&
                cache->enemies[0].active == (i != DESERT_DEPTH - 1);
        } else {
            caches_ok &= cache->enemy_count == 0;
        }
    }
    ASSERT("desert maps, living/dead enemies and cleared flags survive save/load", caches_ok);
    ASSERT("a current-format save with missing desert state is still rejected",
        remove_save_field(ROUND_TRIP_SLOT, "desert_cache") &&
        !load_game(&loaded, ROUND_TRIP_SLOT));
    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_sunscar_save_migration(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    original.player.player_class = CLASS_WARRIOR;
    game_init(&original);
    original.defeated_bosses = (1 << LOCATION_FOREST) | (1 << LOCATION_SWAMP);
    game_enter_town2(&original);
    original.player.level = 10;
    original.player.hp = 115;
    original.gold = 222;
    original.score = 1234;
    original.dain_quest_state = 2;
    original.dain_map_fragments = 3;
    original.max_swamp_level_reached = 4;
    original.max_frostfell_level_reached = 3;
    original.inventory[0] = item_make_long_sword();
    for (int y = 10; y <= 14; y++) {
        original.map.tiles[y][0] = TILE_WALL;
    }
    for (int version = 72; version <= 73; version++) {
        memset(&loaded, 0xff, sizeof(loaded));
        int old_save = save_game(&original, ROUND_TRIP_SLOT) &&
            rewrite_save_version(ROUND_TRIP_SLOT, version) &&
            remove_save_field(ROUND_TRIP_SLOT, "max_desert_level_reached") &&
            remove_save_field(ROUND_TRIP_SLOT, "desert_cache") &&
            remove_save_field(ROUND_TRIP_SLOT, "sandstorm_staff_unclaimed");
        if (version == 72 && old_save) {
            old_save = remove_save_field(ROUND_TRIP_SLOT, "kraken_bow_unclaimed") &&
                remove_save_field(ROUND_TRIP_SLOT, "player.freeze_recovery");
        }
        int migrated = old_save && load_game(&loaded, ROUND_TRIP_SLOT);
        int fresh_desert = migrated && loaded.max_desert_level_reached == 1 &&
            !loaded.sandstorm_staff_unclaimed && !loaded.kraken_bow_unclaimed &&
            !loaded.player.freeze_recovery;
        for (int i = 0; i < DESERT_DEPTH; i++) {
            fresh_desert &= !loaded.desert_cache[i].valid &&
                !loaded.desert_cache[i].level_cleared && loaded.desert_cache[i].enemy_count == 0;
        }
        ASSERT("pre-Sunscar saves initialize fresh desert state instead of resetting the game", fresh_desert);
        ASSERT("migrating an old testing save preserves character, inventory, quests and regional progress",
            migrated && loaded.player.level == original.player.level &&
            loaded.player.hp == original.player.hp && loaded.player.x == original.player.x &&
            loaded.player.y == original.player.y && loaded.gold == original.gold &&
            loaded.score == original.score && loaded.inventory_count == original.inventory_count &&
            weapon_fields_match(&loaded.inventory[0], &original.inventory[0]) &&
            loaded.dain_quest_state == 2 && loaded.dain_map_fragments == 3 &&
            loaded.defeated_bosses == original.defeated_bosses &&
            loaded.max_swamp_level_reached == 4 && loaded.max_frostfell_level_reached == 3);
        int west_gate = migrated;
        for (int y = 10; y <= 14; y++) {
            west_gate &= loaded.map.tiles[y][0] == TILE_TOWN_EXIT;
        }
        ASSERT("loading an old Stillbury map reopens all five west gate tiles", west_gate);
        int resaved = migrated && save_game(&loaded, ROUND_TRIP_SLOT) && load_game(&loaded, ROUND_TRIP_SLOT);
        ASSERT("a migrated testing save can be saved and loaded in the current format",
            resaved && loaded.player.hp == original.player.hp && loaded.max_desert_level_reached == 1);
    }

    original.max_desert_level_reached = DESERT_DEPTH;
    original.desert_cache[0].valid = 1;
    map_generate_desert(&original.desert_cache[0].map, 1);
    original.desert_cache[0].enemy_count = 1;
    original.desert_cache[0].enemies[0] = (Enemy){
        .type = ENEMY_SCARAB, .name = "Scarab", .active = 1, .hp = 11, .max_hp = 24,
        .x = original.desert_cache[0].map.stairs_up_x - 1,
        .y = original.desert_cache[0].map.stairs_up_y
    };
    int migrated = save_game(&original, ROUND_TRIP_SLOT) &&
        rewrite_save_version(ROUND_TRIP_SLOT, 74) &&
        remove_save_field(ROUND_TRIP_SLOT, "sandstorm_staff_unclaimed") &&
        load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("version 74 saves gain the staff flag without losing existing desert exploration and enemies",
        migrated && !loaded.sandstorm_staff_unclaimed && loaded.max_desert_level_reached == DESERT_DEPTH &&
        loaded.desert_cache[0].valid && loaded.desert_cache[0].enemy_count == 1 &&
        loaded.desert_cache[0].enemies[0].type == ENEMY_SCARAB && loaded.desert_cache[0].enemies[0].hp == 11 &&
        memcmp(loaded.desert_cache[0].map.tiles, original.desert_cache[0].map.tiles,
            sizeof(original.desert_cache[0].map.tiles)) == 0);
    original.sunscar_lamp_quest_state = 2;
    int saved = save_game(&original, ROUND_TRIP_SLOT) && load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("the recovered lamp quest state survives save/load", saved && loaded.sunscar_lamp_quest_state == 2);
    int missing = remove_save_field(ROUND_TRIP_SLOT, "sunscar_lamp_quest_state");
    ASSERT("current saves must include lamp quest state", missing && !load_game(&loaded, ROUND_TRIP_SLOT));
    migrated = missing && rewrite_save_version(ROUND_TRIP_SLOT, 75) && load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("version 75 testing saves gain an unassigned lamp quest while preserving desert progress",
        migrated && loaded.sunscar_lamp_quest_state == 0 && loaded.max_desert_level_reached == DESERT_DEPTH &&
        loaded.desert_cache[0].valid && loaded.desert_cache[0].enemies[0].hp == 11 &&
        loaded.player.hp == original.player.hp && loaded.gold == original.gold &&
        loaded.defeated_bosses == original.defeated_bosses);
    remove_test_save(ROUND_TRIP_SLOT);
}

static void test_stacked_item_underlays(void) {
    static GameState original;
    static GameState loaded;
    memset(&original, 0, sizeof(original));
    game_init(&original);
    game_enter_town2(&original);
    original.player.x = 30;
    original.player.y = 12;
    original.inventory[0] = item_make_health_potion();
    original.inventory[1] = item_make_mana_potion();
    original.inventory_count = 2;
    action_resolve_player(&original, (Action){ACTION_DROP_ITEM, 0, 0});
    action_resolve_player(&original, (Action){ACTION_DROP_ITEM, 0, 0});
    ASSERT("stacked dropped items each remember the cobblestone underneath",
        original.floor_item_count == 2 &&
        original.floor_items[0].underlying_tile == TILE_TOWN_PATH &&
        original.floor_items[1].underlying_tile == TILE_TOWN_PATH);
    action_resolve_player(&original, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up the first item leaves the remaining item over cobblestone",
        !original.floor_items[0].active && original.floor_items[1].active &&
        original.map.tiles[12][30] == TILE_ITEM &&
        original.floor_items[1].underlying_tile == TILE_TOWN_PATH);
    int saved = save_game(&original, ROUND_TRIP_SLOT) && load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("stacked drop ground survives saving after the first pickup",
        saved && loaded.floor_items[1].underlying_tile == TILE_TOWN_PATH &&
        loaded.map.tiles[12][30] == TILE_ITEM);
    action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("picking up the last saved item restores the cobblestone tile",
        saved && !loaded.floor_items[1].active && loaded.map.tiles[12][30] == TILE_TOWN_PATH);

    // Older drops recorded the second item's marker as its ground.
    original.floor_items[1].underlying_tile = TILE_ITEM;
    saved = save_game(&original, ROUND_TRIP_SLOT) && load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("loading an existing broken pile recovers its ground from the collected first item",
        saved && loaded.floor_items[1].active &&
        loaded.floor_items[1].underlying_tile == TILE_TOWN_PATH && loaded.map.tiles[12][30] == TILE_ITEM);
    action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("the repaired saved pile restores cobblestone when collected",
        saved && loaded.map.tiles[12][30] == TILE_TOWN_PATH);

    original.floor_items[1].active = 0;
    saved = save_game(&original, ROUND_TRIP_SLOT) && load_game(&loaded, ROUND_TRIP_SLOT);
    ASSERT("loading a saved hole left by an emptied pile restores its cobblestone",
        saved && loaded.map.tiles[12][30] == TILE_TOWN_PATH &&
        loaded.floor_items[1].underlying_tile == TILE_TOWN_PATH);
    remove_test_save(ROUND_TRIP_SLOT);
}

void test_save_load(void) {
    test_harbor_road_save_load();
    test_spent_island_map_on_load();
    test_cain_save_load();
    printf("Save/load tests:\n");
    test_desert_state_round_trip();
    test_sunscar_save_migration();
    test_stacked_item_underlays();
    test_current_weapon_round_trip();
    test_dual_wield_round_trip();
    test_legacy_off_hand_migration();
    test_legacy_goblin_reward_migration();
    test_migrated_weapon_round_trip();
    test_migrated_armor_round_trip();
    test_harbor_relocation();
    test_legacy_coast_sluice_removal();
    test_legacy_town3_moat();
    test_legacy_tavern_move();
    test_legacy_town_square();
    test_legacy_inn_move();
    test_legacy_apothecary();
    test_legacy_fireball_scroll();
    test_message_kinds_round_trip();
    test_key_bindings_round_trip();
    test_forest_enemy_repair();
    test_retired_dungeon_gates_removed();
    test_legacy_king_road_world();
}
