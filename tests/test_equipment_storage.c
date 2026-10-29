#include "test_utils.h"
#include "game/game.h"
#include "game/jail.h"
#include "screens/inventory.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define STORAGE_SLOT 99226

static void reset_gear(GameState *g) {
    g->player.player_class = CLASS_WARRIOR;
    game_init(g);
    for (int slot = EQUIPMENT_STRONG_HAND; slot < MAX_CARRIED_ITEMS; slot++) {
        game_clear_equipment_slot(g, slot);
    }
    g->inventory_count = 0;
}

static int put_on(GameState *g, Item item, EquipmentSlot slot) {
    if (g->inventory_count >= MAX_INVENTORY) {
        return 0;
    }
    int index = g->inventory_count++;
    g->inventory[index] = item;
    return game_equip_item(g, index, slot);
}

static int make_legacy_save(void) {
    FILE *file = fopen("saves/savegame_99226.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = calloc((size_t)size + 1, 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    fread(data, 1, (size_t)size, file);
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON *bag = cJSON_GetObjectItem(root, "inventory");
    cJSON *worn = cJSON_GetObjectItem(root, "equipment");
    const char *indices[] = {"equipped_main_hand", "equipped_off_hand", NULL, "equipped_armor", NULL, NULL};
    for (int i = 0; i < EQUIPMENT_SLOT_COUNT; i++) {
        cJSON *item = cJSON_GetObjectItem(cJSON_GetArrayItem(worn, i), "item");
        if (indices[i] && cJSON_GetObjectItem(item, "active")->valueint) {
            cJSON_ReplaceItemInObject(root, indices[i], cJSON_CreateNumber(cJSON_GetArraySize(bag)));
            cJSON_AddItemToArray(bag, cJSON_Duplicate(item, 1));
        }
    }
    cJSON_ReplaceItemInObject(root, "inventory_count", cJSON_CreateNumber(cJSON_GetArraySize(bag)));
    cJSON_ReplaceItemInObject(root, "save_version", cJSON_CreateNumber(223));
    cJSON_DeleteItemFromObject(root, "equipment");
    cJSON_DeleteItemFromObject(root, "jail_extra_slots");
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99226.json", "wb");
    int ok = file && json && fwrite(json, 1, strlen(json), file) == strlen(json);
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

void test_equipment_storage(void) {
    printf("Six equipment slots and storage migration:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *loaded = calloc(1, sizeof(*loaded));
    ASSERT("equipment storage fixtures allocate", g && loaded);
    if (!g || !loaded) {
        free(g);
        free(loaded);
        return;
    }
    ASSERT("equipment storage test slot is unused", !save_exists(STORAGE_SLOT));
    if (save_exists(STORAGE_SLOT)) {
        free(g);
        free(loaded);
        return;
    }
    const char *starters[] = {"Rusty Sword", "Worn Staff", "Rusty Dagger"};
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        g->player.player_class = cls;
        game_init(g);
        const Item *weapon = &g->inventory[0];
        ASSERT("each class starts with only one poor weapon", weapon && !strcmp(weapon->name, starters[cls]) && weapon->attack_bonus == 1 && weapon->value == 10 && g->inventory_count == 1);
        int empty = 1;
        for (int slot = EQUIPMENT_STRONG_HAND; slot < MAX_CARRIED_ITEMS; slot++) {
            empty &= game_equipped_item(g, slot) == NULL;
        }
        ASSERT("new characters have no helmet, armor, accessories, spare weapon or scroll", empty && g->player.known_spell_count == 0);
    }
    reset_gear(g);
    int attack = g->player.attack;
    int defense = g->player.defense;
    int hp = g->player.max_hp;
    int mp = g->player.max_mp;
    ASSERT("all six slots can be populated outside the bag", put_on(g, item_make_long_sword(), EQUIPMENT_STRONG_HAND) && put_on(g, item_make_buckler(), EQUIPMENT_WEAK_HAND) && put_on(g, item_make_leather_cap(), EQUIPMENT_HELMET) && put_on(g, item_make_chain_mail(), EQUIPMENT_ARMOR) && put_on(g, item_make_ring_of_vigor(), EQUIPMENT_ACCESSORY_1) && put_on(g, item_make_copper_amulet(), EQUIPMENT_ACCESSORY_2) && g->inventory_count == 0 && game_equipment_valid(g));
    ASSERT("helmet, armor and accessory bonuses affect player stats", g->player.attack == attack + 6 && g->player.defense == defense + 1 + 2 + 3 && g->player.max_hp == hp + 10 && g->player.max_mp == mp + 10);
    g->player.hp = 37;
    g->player.mp = 3;
    g->gold = 321;
    g->defeated_bosses = 1 << LOCATION_DUNGEON;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[g->inventory_count++] = item_make_health_potion();
    }
    ASSERT("ten bag items coexist with six worn items", g->inventory_count == MAX_INVENTORY && game_equipment_valid(g));
    ASSERT("full bags refuse unequipping without losing bonuses or equipment", !game_unequip_item(g, EQUIPMENT_HELMET) && game_equipped_item(g, EQUIPMENT_HELMET) && g->player.defense == defense + 6);
    g->inventory[0] = item_make_leather_gauntlets();
    ASSERT("a full bag permits a one-for-one accessory swap", game_equip_item(g, 0, EQUIPMENT_ACCESSORY_1) && g->inventory_count == MAX_INVENTORY && g->inventory[0].accessory_family == ACCESSORY_FAMILY_RING && g->player.attack == attack + 7 && g->player.max_hp == hp && g->player.hp == 37);
    g->inventory[1] = item_make_leather_gauntlets();
    ASSERT("duplicate non-ring categories are refused atomically", !game_equip_item(g, 1, EQUIPMENT_ACCESSORY_2) && g->inventory[1].accessory_family == ACCESSORY_FAMILY_GAUNTLETS && game_equipped_item(g, EQUIPMENT_ACCESSORY_2)->accessory_family == ACCESSORY_FAMILY_AMULET);
    ASSERT("a ring swaps back without healing HP", game_equip_item(g, 0, EQUIPMENT_ACCESSORY_1) && g->player.max_hp == hp + 10 && g->player.hp == 37);
    g->inventory[1] = item_make_ring_of_vigor();
    ASSERT("two copies of the same ring are allowed and stack", game_equip_item(g, 1, EQUIPMENT_ACCESSORY_2) && g->player.max_hp == hp + 20 && g->player.max_mp == mp && g->player.mp == 3);
    g->inventory[2] = item_make_greatsword();
    Item strong = *game_equipped_item(g, EQUIPMENT_STRONG_HAND);
    ASSERT("two-handed swaps with no room for the weak hand are refused", !game_equip_main_hand(g, 2) && !memcmp(&strong, game_equipped_item(g, EQUIPMENT_STRONG_HAND), sizeof(strong)) && game_equipped_item(g, EQUIPMENT_WEAK_HAND));
    game_remove_inventory_item(g, 9);
    ASSERT("two-handed equip returns both old hand items to the bag", game_equip_main_hand(g, 2) && g->inventory_count == MAX_INVENTORY && !game_equipped_item(g, EQUIPMENT_WEAK_HAND) && game_equipped_item(g, EQUIPMENT_STRONG_HAND)->weapon_hands == WEAPON_HANDS_TWO);
    ASSERT("two-handed weapons disable shields and off-hand weapons", !game_equip_shield(g, 9) && !game_equip_off_hand(g, 0));
    g->inventory[3] = item_make_short_sword();
    ASSERT("switching back to one-handed leaves the weak slot empty", game_equip_main_hand(g, 3) && !game_equipped_item(g, EQUIPMENT_WEAK_HAND));
    int ok = save_game(g, STORAGE_SLOT) && load_game(loaded, STORAGE_SLOT);
    ASSERT("all worn items and a full bag round-trip with wounds and progress", ok && loaded->inventory_count == MAX_INVENTORY && !memcmp(g->inventory, loaded->inventory, sizeof(g->inventory)) && loaded->player.attack == g->player.attack && loaded->player.defense == g->player.defense && loaded->player.hp == 37 && loaded->player.mp == 3 && loaded->gold == 321 && loaded->defeated_bosses == g->defeated_bosses);

    reset_gear(g);
    put_on(g, item_make_long_sword(), EQUIPMENT_STRONG_HAND);
    put_on(g, item_make_buckler(), EQUIPMENT_WEAK_HAND);
    put_on(g, item_make_chain_mail(), EQUIPMENT_ARMOR);
    for (int i = 0; i < 7; i++) {
        g->inventory[g->inventory_count++] = item_make_health_potion();
    }
    g->player.hp = 31;
    g->player.mp = 7;
    g->gold = 713;
    g->inventory[EQUIPMENT_STRONG_HAND].attack_bonus++;
    g->inventory[EQUIPMENT_STRONG_HAND].sharpened = 1;
    g->player.attack++;
    ok = save_game(g, STORAGE_SLOT) && make_legacy_save() && load_game(loaded, STORAGE_SLOT);
    ASSERT("a full version-223 bag migrates worn items without reapplying bonuses", ok && loaded->inventory_count == 7 && loaded->player.attack == g->player.attack && loaded->player.defense == g->player.defense && loaded->player.hp == 31 && loaded->player.mp == 7 && loaded->gold == 713 && game_equipped_item(loaded, EQUIPMENT_STRONG_HAND)->sharpened && game_equipped_item(loaded, EQUIPMENT_WEAK_HAND) && game_equipped_item(loaded, EQUIPMENT_ARMOR) && !game_equipped_item(loaded, EQUIPMENT_HELMET));
    ASSERT("migrated storage stays stable through the next save", ok && save_game(loaded, STORAGE_SLOT) && load_game(g, STORAGE_SLOT) && g->inventory_count == 7 && g->player.hp == 31 && g->player.attack == loaded->player.attack);

    reset_gear(g);
    put_on(g, item_make_short_sword(), EQUIPMENT_STRONG_HAND);
    Item spare = item_make_short_sword();
    ASSERT("a second one-handed weapon uses half attack", put_on(g, spare, EQUIPMENT_WEAK_HAND) && g->player.attack == 14 + 3 + 2);
    ASSERT("unequipping strong hand promotes the weak weapon and preserves both items", game_unequip_item(g, EQUIPMENT_STRONG_HAND) && g->inventory_count == 1 && g->player.attack == 17 && game_equipped_item(g, EQUIPMENT_STRONG_HAND) && !game_equipped_item(g, EQUIPMENT_WEAK_HAND));
    put_on(g, item_make_buckler(), EQUIPMENT_WEAK_HAND);
    while (g->inventory_count < MAX_INVENTORY - 1) {
        g->inventory[g->inventory_count++] = item_make_health_potion();
    }
    ASSERT("unequipping a weapon and shield requires two free bag spaces", !game_unequip_item(g, EQUIPMENT_STRONG_HAND) && game_equipped_item(g, EQUIPMENT_WEAK_HAND));

    reset_gear(g);
    put_on(g, item_make_long_sword(), EQUIPMENT_STRONG_HAND);
    put_on(g, item_make_buckler(), EQUIPMENT_WEAK_HAND);
    put_on(g, item_make_leather_cap(), EQUIPMENT_HELMET);
    put_on(g, item_make_chain_mail(), EQUIPMENT_ARMOR);
    put_on(g, item_make_ring_of_vigor(), EQUIPMENT_ACCESSORY_1);
    put_on(g, item_make_copper_amulet(), EQUIPMENT_ACCESSORY_2);
    for (int i = 0; i < MAX_INVENTORY; i++) {
        g->inventory[g->inventory_count++] = item_make_short_sword();
    }
    game_enter_town_gate(g, LOCATION_CASTLE, INFORMANT_X, INFORMANT_Y + 1);
    jail_talk_nearby(g);
    ASSERT("arrest safely stores all sixteen carried gear items and removes every bonus", g->jail_gear_count == MAX_CARRIED_ITEMS && g->inventory_count == 0 && !game_equipped_item(g, EQUIPMENT_HELMET) && g->player.attack == 14 && g->player.defense == 6 && g->player.max_hp == 150 && g->player.max_mp == 20);
    ok = save_game(g, STORAGE_SLOT) && load_game(loaded, STORAGE_SLOT);
    ASSERT("six confiscated slot assignments and sixteen items survive reload", ok && loaded->jail_gear_count == MAX_CARRIED_ITEMS && !memcmp(g->jail_gear, loaded->jail_gear, sizeof(g->jail_gear)) && !memcmp(g->jail_extra_slots, loaded->jail_extra_slots, sizeof(g->jail_extra_slots)));
    if (ok) {
        *g = *loaded;
        g->location = LOCATION_JAIL_ARMORY;
        g->player.x = JAIL_GEAR_X;
        g->player.y = JAIL_GEAR_Y + 1;
        g->enemy_count = 0;
        for (int i = 0; i < MAX_INVENTORY; i++) {
            g->inventory[g->inventory_count++] = item_make_health_potion();
        }
        jail_recover_gear(g);
        ASSERT("original worn slots recover directly despite a full bag", g->jail_gear_claimed == 63 && g->jail_gear_state == 1 && game_equipment_valid(g) && game_equipped_item(g, EQUIPMENT_ACCESSORY_2) && g->inventory_count == MAX_INVENTORY);
        for (int i = 0; i < MAX_INVENTORY; i++) {
            game_remove_inventory_item(g, 0);
        }
        jail_recover_gear(g);
        ASSERT("remaining bag gear recovers once without overflowing storage", g->jail_gear_state == 3 && g->jail_gear_claimed == 65535 && g->inventory_count == MAX_INVENTORY);
    }
    InventoryScreen inv;
    inventory_init(&inv);
    inventory_handle_key(&inv, SDL_SCANCODE_TAB, 0, SDL_SCANCODE_I);
    inventory_handle_key(&inv, SDL_SCANCODE_DOWN, 0, SDL_SCANCODE_I);
    ASSERT("equipment remains selectable when the bag is empty", inv.equipment_tab && inventory_selected_index(&inv) == EQUIPMENT_WEAK_HAND && inventory_handle_key(&inv, SDL_SCANCODE_X, 0, SDL_SCANCODE_I) == INVENTORY_UNEQUIP);
    inventory_handle_key(&inv, SDL_SCANCODE_TAB, 2, SDL_SCANCODE_I);
    ASSERT("number keys select the destination accessory slot", inventory_handle_key(&inv, SDL_SCANCODE_2, 2, SDL_SCANCODE_I) == INVENTORY_EQUIP && inv.accessory_slot == 2);
    ASSERT("inventory scrolling uses the same visible range as mouse selection", inventory_visible_rows(27) == 8 && inventory_list_start(9, 10, 3) == 7);
    remove("saves/savegame_99226.json");
    free(g);
    free(loaded);
}
