#include "test_utils.h"
#include "game/game.h"
#include "screens/inventory.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ACCESSORY_SLOT_SAVE 99272

static int old_six_slot_save(void) {
    FILE *file = fopen("saves/savegame_99272.json", "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *data = calloc((size_t)length + 1, 1);
    if (!data) {
        fclose(file);
        return 0;
    }
    size_t read = fread(data, 1, (size_t)length, file);
    data[read] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        return 0;
    }
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 240);
    cJSON *slots = cJSON_GetObjectItem(root, "equipment");
    cJSON_DeleteItemFromArray(slots, 7);
    cJSON_DeleteItemFromArray(slots, 6);
    slots = cJSON_GetObjectItem(root, "jail_extra_slots");
    cJSON_DeleteItemFromArray(slots, 4);
    cJSON_DeleteItemFromArray(slots, 3);
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99272.json", "wb");
    int ok = file && data && fputs(data, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(data);
    return ok;
}

void test_accessory_slots(void) {
    printf("Four accessory slots:\n");
    static GameState g;
    static GameState loaded;
    ASSERT("four-accessory test slot is unused", !save_exists(ACCESSORY_SLOT_SAVE));
    if (save_exists(ACCESSORY_SLOT_SAVE)) {
        return;
    }
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        g.player.player_class = cls;
        game_init(&g);
        g.inventory_count = 0;
        g.player.hp = 37;
        g.player.mp = 3;
        int attack = g.player.attack;
        int defense = g.player.defense;
        int hp = g.player.max_hp;
        int mp = g.player.max_mp;
        Item items[] = {item_make_leather_gauntlets(), item_make_leather_bracers(), item_make_ring_of_vigor(), item_make_copper_amulet()};
        for (int i = 0; i < ACCESSORY_SLOT_COUNT; i++) {
            g.inventory[g.inventory_count++] = items[i];
            action_resolve_player(&g, (Action){ACTION_EQUIP_ITEM, 0, 0});
        }
        ASSERT("all classes can auto-equip gauntlets, bracers, a ring and an amulet together", !g.inventory_count && game_equipment_valid(&g) && g.player.attack == attack + 1 && g.player.defense == defense + 1 && g.player.max_hp == hp + 10 && g.player.max_mp == mp + 10 && g.player.hp == 37 && g.player.mp == 3 && game_equipped_item(&g, EQUIPMENT_ACCESSORY_4));
        ASSERT("four worn accessories retain their categories and wounds on reload", save_game(&g, ACCESSORY_SLOT_SAVE) && load_game(&loaded, ACCESSORY_SLOT_SAVE) && !memcmp(g.inventory, loaded.inventory, sizeof(g.inventory)) && loaded.player.max_hp == g.player.max_hp && loaded.player.max_mp == g.player.max_mp && loaded.player.hp == 37 && loaded.player.mp == 3);
        g.inventory[0] = item_make_accessory(ACCESSORY_FAMILY_AMULET, 2);
        g.inventory_count = 1;
        action_resolve_player(&g, (Action){ACTION_EQUIP_ITEM, 0, 0});
        ASSERT("default equip upgrades a non-ring category in its original slot", game_equipped_item(&g, EQUIPMENT_ACCESSORY_4)->max_mp_bonus == 20 && g.inventory[0].accessory_family == ACCESSORY_FAMILY_AMULET && game_equipped_item(&g, EQUIPMENT_ACCESSORY_1)->accessory_family == ACCESSORY_FAMILY_GAUNTLETS);
        game_init(&g);
        g.inventory_count = 0;
        hp = g.player.max_hp;
        Item mixed[] = {item_make_ring_of_vigor(), item_make_ring_of_vigor(), item_make_leather_gauntlets(), item_make_leather_bracers()};
        for (int i = 0; i < ACCESSORY_SLOT_COUNT; i++) {
            g.inventory[g.inventory_count++] = mixed[i];
            action_resolve_player(&g, (Action){ACTION_EQUIP_ITEM, 0, i + 1});
        }
        ASSERT("two rings stack alongside gauntlets and bracers without using bag capacity", !g.inventory_count && game_equipment_valid(&g) && g.player.max_hp == hp + 20 && game_equipped_item(&g, EQUIPMENT_ACCESSORY_4)->accessory_family == ACCESSORY_FAMILY_BRACERS);
        g.inventory[g.inventory_count++] = item_make_crypt_ring();
        ASSERT("a third ring is refused without replacing gauntlets or changing bonuses", !game_equip_item(&g, 0, EQUIPMENT_ACCESSORY_3) && g.inventory_count == 1 && g.player.max_hp == hp + 20 && game_equipped_item(&g, EQUIPMENT_ACCESSORY_3)->accessory_family == ACCESSORY_FAMILY_GAUNTLETS);

    }
    for (int family = ACCESSORY_FAMILY_GAUNTLETS; family <= ACCESSORY_FAMILY_BOOTS; family++) {
        if (family == ACCESSORY_FAMILY_RING) {
            continue;
        }
        game_init(&g);
        g.inventory_count = 1;
        g.inventory[0] = item_make_accessory(family, 1);
        ASSERT("every non-ring category can use accessory slot four", game_equip_item(&g, 0, EQUIPMENT_ACCESSORY_4));
        g.inventory_count = 1;
        g.inventory[0] = item_make_accessory(family, 2);
        for (int slot = EQUIPMENT_ACCESSORY_1; slot < EQUIPMENT_ACCESSORY_4; slot++) {
            Player before = g.player;
            ASSERT("a second gauntlet, bracer, amulet, belt or boot category is refused in any other slot", !game_equip_item(&g, 0, slot) && g.inventory_count == 1 && !memcmp(&before, &g.player, sizeof(before)) && !game_equipped_item(&g, slot));
        }
    }
    game_init(&g);
    g.inventory_count = 0;
    Item ring = item_make_ring_of_vigor();
    Item amulet = item_make_copper_amulet();
    game_restore_equipment(&g, &ring, EQUIPMENT_ACCESSORY_1);
    game_restore_equipment(&g, &amulet, EQUIPMENT_ACCESSORY_2);
    g.player.hp = 27;
    g.player.mp = 3;
    g.gold = 429;
    loaded.inventory[EQUIPMENT_ACCESSORY_3] = ring;
    loaded.inventory[EQUIPMENT_ACCESSORY_4] = amulet;
    ASSERT("six-slot saves gain two empty slots without duplicating bonuses or retaining a previous character's gear", save_game(&g, ACCESSORY_SLOT_SAVE) && old_six_slot_save() && load_game(&loaded, ACCESSORY_SLOT_SAVE) && !game_equipped_item(&loaded, EQUIPMENT_ACCESSORY_3) && !game_equipped_item(&loaded, EQUIPMENT_ACCESSORY_4) && loaded.player.max_hp == g.player.max_hp && loaded.player.max_mp == g.player.max_mp && loaded.player.hp == 27 && loaded.player.mp == 3 && loaded.gold == 429 && loaded.jail_extra_slots[3] == -1 && loaded.jail_extra_slots[4] == -1);
    InventoryScreen inv;
    inventory_init(&inv);
    for (int i = 0; i < ACCESSORY_SLOT_COUNT; i++) {
        ASSERT("number keys one through four choose the corresponding accessory destination", inventory_handle_key(&inv, SDL_SCANCODE_1 + i, 1, SDL_SCANCODE_I) == INVENTORY_EQUIP && inv.accessory_slot == i + 1);
    }
    inventory_handle_key(&inv, SDL_SCANCODE_TAB, 0, SDL_SCANCODE_I);
    for (int i = 1; i < EQUIPMENT_SLOT_COUNT; i++) {
        inventory_handle_key(&inv, SDL_SCANCODE_DOWN, 0, SDL_SCANCODE_I);
    }
    ASSERT("the final accessory slot is selectable and removable with an empty bag", inventory_selected_index(&inv) == EQUIPMENT_ACCESSORY_4 && inventory_handle_key(&inv, SDL_SCANCODE_X, 0, SDL_SCANCODE_I) == INVENTORY_UNEQUIP);
    remove("saves/savegame_99272.json");
}
