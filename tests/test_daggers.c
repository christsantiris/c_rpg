#include "test_utils.h"
#include "game/game.h"
#include "screens/inventory.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define DAGGER_SLOT 99340
#define DAGGER_SAVE "saves/savegame_99340.json"

static GameState game;
static GameState loaded;

static int sample_gold(Item weapon, int source, int off_hand, int *drops) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_ROGUE;
    game_init(&game);
    game.location = source == 3 ? LOCATION_CASTLE_INTERIOR : LOCATION_DUNGEON;
    game.player.x = 20;
    game.player.y = 12;
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    game.player.known_spells[0] = spell_make_magic_arrow();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    game.inventory_count = 2;
    weapon.critical_chance_bonus = 0;
    game.inventory[0] = source == 2 ? item_make_bow() : off_hand ? item_make_short_sword() : weapon;
    game.inventory[1] = weapon;
    game_equip_main_hand(&game, 0);
    if (off_hand) {
        game_equip_off_hand(&game, 0);
    }
    for (int x = 20; x <= 24; x++) {
        game.map.tiles[12][x] = TILE_FLOOR;
    }
    int coins = 0;
    *drops = 0;
    srand(424242);
    for (int i = 0; i < 20000; i++) {
        int x = source == 1 || source == 2 ? 22 : 21;
        game.enemy_count = 1;
        game.floor_item_count = 0;
        game.map.tiles[12][x] = TILE_FLOOR;
        game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_STONE_GIANT, .hp = 1, .max_hp = 1, .x = x, .y = 12, .experience = 1};
        game.player.experience = 0;
        int before = game.gold;
        if (source == 1) {
            game.player.mp = game.player.max_mp;
            action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
        } else if (source == 2) {
            game.player.arrows = 1;
            action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
        } else {
            action_resolve_player(&game, (Action){ACTION_MOVE, x, 12});
        }
        if (game.enemies[0].active) {
            return -1;
        }
        int purse = game.gold - before;
        for (int j = 0; j < game.floor_item_count; j++) {
            if (game.floor_items[j].active && game.floor_items[j].item.type == ITEM_GOLD) {
                purse += game.floor_items[j].item.value;
            }
        }
        coins += purse;
        *drops += purse > 0;
    }
    return coins;
}

static void test_dagger_gold(void) {
    Item rusty = item_make_rusty_dagger();
    Item dagger = item_make_dagger();
    Item magic = item_make_magic_dagger();
    ASSERT("rusty dagger is the weakest Rogue blade with its own rust-colored visual", rusty.attack_bonus == 1 &&
        rusty.critical_chance_bonus == 10 && rusty.value == 10 && rusty.class_mask == ITEM_CLASS_ROGUE &&
        rusty.visual_id == ITEM_VISUAL_RUSTY_DAGGER && item_can_sharpen(&rusty));
    ASSERT("dagger upgrades improve attack, critical chance, and gold rating", dagger.attack_bonus == 3 && magic.attack_bonus == 7 &&
        rusty.gold_drop_bonus_percent == 10 && dagger.gold_drop_bonus_percent == 25 && magic.gold_drop_bonus_percent == 50);
    int normal_drops;
    int rusty_drops;
    int dagger_drops;
    int magic_drops;
    Item sword = item_make_short_sword();
    int normal_gold = sample_gold(sword, 0, 0, &normal_drops);
    int rusty_gold = sample_gold(rusty, 0, 0, &rusty_drops);
    int dagger_gold = sample_gold(dagger, 0, 0, &dagger_drops);
    int magic_gold = sample_gold(magic, 0, 0, &magic_drops);
    ASSERT("dagger ratings increase coin-drop frequency on melee kills", normal_drops < rusty_drops &&
        rusty_drops < dagger_drops && dagger_drops < magic_drops);
    ASSERT("better daggers also yield larger average purses", normal_gold > 0 && normal_gold < rusty_gold &&
        rusty_gold < dagger_gold && dagger_gold < magic_gold);
    int drops;
    int off_gold = sample_gold(magic, 0, 1, &drops);
    ASSERT("off-hand dagger grants half its gold rating", off_gold == dagger_gold && drops == dagger_drops);
    int spell_gold = sample_gold(magic, 1, 0, &drops);
    ASSERT("spell kills gain no gold bonus from an equipped dagger", spell_gold >= 0 && spell_gold == sample_gold(sword, 1, 0, &drops));
    int arrow_gold = sample_gold(magic, 2, 0, &drops);
    ASSERT("carrying a dagger cannot boost bow kills", arrow_gold >= 0 && arrow_gold == sample_gold(sword, 2, 0, &drops));
    ASSERT("dagger bonus also applies to guaranteed castle purses", sample_gold(magic, 3, 0, &drops) == 20000 * 8);
}

static void remove_gold_metadata(cJSON *node) {
    if (cJSON_IsObject(node)) {
        cJSON_DeleteItemFromObject(node, "gold_drop_bonus_percent");
        cJSON_DeleteItemFromObject(node, "weapon_family");
    }
    for (cJSON *child = node->child; child; child = child->next) {
        remove_gold_metadata(child);
    }
}

static int legacy_save(void) {
    if (!save_game(&game, DAGGER_SLOT)) {
        return 0;
    }
    FILE *file = fopen(DAGGER_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 152);
    remove_gold_metadata(root);
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(DAGGER_SAVE, "wb");
    int ok = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return ok;
}

static void test_dagger_saves(void) {
    for (int sharpened = 0; sharpened <= 1; sharpened++) {
        memset(&game, 0, sizeof(game));
        game.player.player_class = CLASS_ROGUE;
        game_init(&game);
        game.inventory_count = 2;
        game.inventory[0] = item_make_dagger();
        game.inventory[1] = item_make_magic_dagger();
        for (int i = 0; i < 2; i++) {
            game.inventory[i].attack_bonus--;
            game.inventory[i].attack_bonus += sharpened;
            game.inventory[i].sharpened = sharpened;
        }
        // Build a real pre-storage fixture: both worn items still occupy its bag.
        game.equipped_main_hand = 0;
        game.equipped_off_hand = 1;
        game.player.attack += game.inventory[0].attack_bonus + game_off_hand_attack_bonus(&game.inventory[1]);
        game.floor_item_count = 1;
        game.floor_items[0] = (FloorItem){.active = 1, .x = 20, .y = 12, .underlying_tile = TILE_TOWN_PATH, .item = game.inventory[0]};
        game.map.tiles[12][20] = TILE_ITEM;
        game.castle_cache[0].valid = 1;
        game.castle_cache[0].map = game.map;
        game.castle_loot_count[0] = 1;
        game.castle_loot[0][0] = game.floor_items[0];
        game.castle_loot[0][0].item = game.inventory[1];
        game.gold = 317;
        game.rook_quest_state = 2;
        int ok = legacy_save() && load_game(&loaded, DAGGER_SLOT);
        ASSERT("old equipped daggers rebalance both hands without losing sharpening or quest progress", ok &&
            loaded.inventory[EQUIPMENT_STRONG_HAND].attack_bonus == 3 + sharpened && loaded.inventory[EQUIPMENT_WEAK_HAND].attack_bonus == 7 + sharpened &&
            loaded.player.attack == 10 + 3 + sharpened + (8 + sharpened) / 2 && loaded.inventory[EQUIPMENT_STRONG_HAND].sharpened == sharpened &&
            loaded.equipped_main_hand == EQUIPMENT_STRONG_HAND && loaded.equipped_off_hand == EQUIPMENT_WEAK_HAND && loaded.gold == 317 && loaded.rook_quest_state == 2);
        ASSERT("legacy dropped and cached daggers gain their gold ratings", ok && loaded.floor_items[0].item.gold_drop_bonus_percent == 25 &&
            loaded.castle_loot[0][0].item.gold_drop_bonus_percent == 50 && loaded.castle_loot[0][0].item.attack_bonus == 7 + sharpened);
        loaded.inventory[EQUIPMENT_STRONG_HAND].gold_drop_bonus_percent = 37;
        loaded.floor_items[0].item.gold_drop_bonus_percent = 13;
        loaded.castle_loot[0][0].item.gold_drop_bonus_percent = 29;
        int attack = loaded.player.attack;
        ASSERT("new gold ratings serialize in inventory, floor loot, and cached castle loot without repeating migrations",
            save_game(&loaded, DAGGER_SLOT) && load_game(&game, DAGGER_SLOT) && game.player.attack == attack &&
            game.inventory[EQUIPMENT_STRONG_HAND].gold_drop_bonus_percent == 37 && game.floor_items[0].item.gold_drop_bonus_percent == 13 &&
            game.castle_loot[0][0].item.gold_drop_bonus_percent == 29);
    }
    remove(DAGGER_SAVE);
}

static void test_inventory_unequip(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_ROGUE;
    game_init(&game);
    game.inventory[0] = item_make_dagger();
    game.inventory[1] = item_make_rusty_dagger();
    game.inventory_count = 2;
    game_equip_main_hand(&game, 0);
    game_equip_off_hand(&game, 0);
    InventoryScreen screen;
    inventory_init(&screen);
    int bag_ignores = inventory_handle_key(&screen, SDL_SCANCODE_X, 0, SDL_SCANCODE_I) == INVENTORY_NONE;
    inventory_handle_key(&screen, SDL_SCANCODE_TAB, 0, SDL_SCANCODE_I);
    ASSERT("equipment tab offers X even when the bag is empty", bag_ignores && inventory_handle_key(&screen, SDL_SCANCODE_X, 0, SDL_SCANCODE_I) == INVENTORY_UNEQUIP);
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, EQUIPMENT_WEAK_HAND, 0});
    ASSERT("unequipping the weak hand returns its item and removes its attack contribution", game.equipped_main_hand == EQUIPMENT_STRONG_HAND && game.equipped_off_hand == -1 && game.inventory_count == 1 && game.player.attack == 13);
    game_equip_off_hand(&game, 0);
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, EQUIPMENT_STRONG_HAND, 0});
    ASSERT("unequipping strong hand promotes the weak weapon at full attack", game.equipped_main_hand == EQUIPMENT_STRONG_HAND && game.equipped_off_hand == -1 && game.player.attack == 11);
    action_resolve_player(&game, (Action){ACTION_EQUIP_OFF_HAND, 0, 0});
    ASSERT("the former strong weapon can then move to the weak hand", game.equipped_main_hand == EQUIPMENT_STRONG_HAND && game.equipped_off_hand == EQUIPMENT_WEAK_HAND && game.player.attack == 13);
    ASSERT("swapped hands persist across save and load", save_game(&game, DAGGER_SLOT) && load_game(&loaded, DAGGER_SLOT) && loaded.equipped_main_hand == EQUIPMENT_STRONG_HAND && loaded.equipped_off_hand == EQUIPMENT_WEAK_HAND && loaded.player.attack == 13 && loaded.inventory[EQUIPMENT_STRONG_HAND].visual_id == ITEM_VISUAL_RUSTY_DAGGER && loaded.inventory[EQUIPMENT_STRONG_HAND].gold_drop_bonus_percent == 10);
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, EQUIPMENT_WEAK_HAND, 0});
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 0, 0});
    ASSERT("moving the weak weapon to strong hand does not double-count bonuses", game.equipped_main_hand == EQUIPMENT_STRONG_HAND && game.equipped_off_hand == -1 && game.player.attack == 13);
    game.inventory[1] = item_make_leather_armor();
    game.inventory_count = 2;
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, 1, 0});
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, EQUIPMENT_ARMOR, 0});
    ASSERT("unequip also removes armor bonuses without dropping it", game.equipped_armor == -1 && game.player.defense == 4 && game.inventory_count == 2);
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, EQUIPMENT_STRONG_HAND, 0});
    int attack = game.player.attack;
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, EQUIPMENT_STRONG_HAND, 0});
    action_resolve_player(&game, (Action){ACTION_UNEQUIP_ITEM, -1, 0});
    ASSERT("repeated and invalid unequip actions never subtract stats twice", game.equipped_main_hand == -1 && game.player.attack == attack && attack == 10);
    remove(DAGGER_SAVE);
}

void test_daggers(void) {
    printf("Rogue daggers, gold bonuses, and inventory unequip tests:\n");
    ASSERT("dagger test save slot is unused", !save_exists(DAGGER_SLOT));
    if (save_exists(DAGGER_SLOT)) {
        return;
    }
    test_dagger_gold();
    test_dagger_saves();
    test_inventory_unequip();
}
