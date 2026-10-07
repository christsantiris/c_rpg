#include "test_utils.h"
#include "game/game.h"
#include "game/castle.h"
#include "screens/workshop.h"
#include "systems/save_load.h"
#include "../external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define WORKSHOP_TEST_SLOT 99137
static GameState game;
static GameState loaded;

static void start(void) {
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_ROGUE;
    game_init(&game);
    game_unequip_off_hand(&game);
    game_unequip_main_hand(&game);
    game.inventory_count = 7;
    game.inventory[0] = item_make_short_sword();
    game.inventory[1] = item_make_dagger();
    game.inventory[2] = item_make_battle_axe();
    game.inventory[3] = item_make_short_sword();
    game.inventory[4] = item_make_bow();
    game.inventory[5] = item_make_staff();
    game.inventory[6] = item_make_health_potion();
    game.gold = 300;
    game_enter_workshop(&game);
    game.player.x = WORKSHOP_SMITH_X;
    game.player.y = WORKSHOP_SMITH_Y + 1;
}

static void test_workshop_travel(void) {
    start();
    game_leave_workshop(&game);
    ASSERT("Ridgeshire workshop doorway is accessible", game.location == LOCATION_TOWN4 &&
        game.map.tiles[TOWN4_WORKSHOP_DOOR_Y][TOWN4_WORKSHOP_DOOR_X] == TILE_WORKSHOP_DOOR &&
        map_is_walkable(&game.map, TOWN4_WORKSHOP_DOOR_X, TOWN4_WORKSHOP_DOOR_Y));
    action_resolve_player(&game, (Action){ACTION_MOVE, TOWN4_WORKSHOP_DOOR_X, TOWN4_WORKSHOP_DOOR_Y});
    ASSERT("workshop doorway enters a safe interior", game.location == LOCATION_WORKSHOP && !game.enemy_count &&
        game.map.tiles[WORKSHOP_SMITH_Y][WORKSHOP_SMITH_X] == TILE_NPC_SHARPENER &&
        !map_is_walkable(&game.map, WORKSHOP_SMITH_X, WORKSHOP_SMITH_Y));
    ASSERT("smith does not sharpen remotely", !game_sharpen_weapon(&game, 0) && game.gold == 300);
    int base_attack = game.player.attack;
    game.player.poison_turns = 2;
    game.player.frozen_turns = 2;
    game.player.known_spells[0] = spell_make_return_to_town();
    game.player.known_spell_count = 1;
    game.player.equipped_spell = 0;
    game.player.mp = 100;
    action_resolve_player(&game, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("workshop is a town interior for spells and status recovery", game.location == LOCATION_WORKSHOP &&
        !game.player.poison_turns && !game.player.frozen_turns && game.player.attack == base_attack);
    game.defeated_bosses |= 1 << LOCATION_MOUNTAINS;
    game.player.x = game.map.stairs_down_x;
    game.player.y = game.map.stairs_down_y - 1;
    action_resolve_player(&game, (Action){ACTION_MOVE, game.map.stairs_down_x, game.map.stairs_down_y});
    ASSERT("workshop exit returns below the Ridgeshire doorway", game.location == LOCATION_TOWN4 &&
        game.player.x == TOWN4_WORKSHOP_DOOR_X && game.player.y == TOWN4_WORKSHOP_DOOR_Y + 1);
    ASSERT("workshop exit preserves the unlocked mountain shortcut", game.map.tiles[TOWN_H - 1][RIDGESHIRE_MOUNTAIN_ROAD_X] == TILE_TOWN_EXIT);
}

static void test_workshop_purchase(void) {
    start();
    game_equip_main_hand(&game, 0);
    int attack = game.player.attack;
    int value = game.inventory[0].value;
    ASSERT("50 gold sharpens equipped sword with immediate attack increase", game_sharpen_weapon(&game, 0) &&
        game.gold == 250 && game.inventory[0].attack_bonus == 4 && game.inventory[0].sharpened && game.player.attack == attack + 1);
    ASSERT("sharpening preserves weapon identity and value", strcmp(game.inventory[0].name, "Short Sword") == 0 && game.inventory[0].value == value);
    ASSERT("same weapon cannot be charged or upgraded twice", !game_sharpen_weapon(&game, 0) &&
        game.gold == 250 && game.inventory[0].attack_bonus == 4 && game.player.attack == attack + 1);
    ASSERT("another copy of the same weapon has its own allowance", game_sharpen_weapon(&game, 3) &&
        game.inventory[3].sharpened && game.player.attack == attack + 1 && game.gold == 200);
    ASSERT("axes are eligible without changing equipped sword attack", game_sharpen_weapon(&game, 2) &&
        game.inventory[2].attack_bonus == 11 && game.gold == 150 && game.player.attack == attack + 1);
    ASSERT("bows, staves, potions and invalid slots are rejected without payment", !game_sharpen_weapon(&game, 4) &&
        !game_sharpen_weapon(&game, 5) && !game_sharpen_weapon(&game, 6) && !game_sharpen_weapon(&game, -1) &&
        !game_sharpen_weapon(&game, 7) && game.gold == 150);
    game_equip_off_hand(&game, 1);
    int off_attack = game.player.attack;
    ASSERT("off-hand dagger sharpening respects rounded half attack", game_sharpen_weapon(&game, 1) &&
        game.inventory[1].attack_bonus == 3 && game.player.attack == off_attack + 1 && game.gold == 100);
    game_unequip_off_hand(&game);
    ASSERT("unequipping sharpened off-hand removes its updated contribution", game.player.attack == attack + 1);
    game_unequip_main_hand(&game);
    game_equip_main_hand(&game, 0);
    ASSERT("re-equipping sharpened sword does not duplicate upgrade", game.player.attack == attack + 1);

    start();
    game.gold = 49;
    ASSERT("insufficient funds leave weapon untouched", !game_sharpen_weapon(&game, 0) &&
        game.gold == 49 && !game.inventory[0].sharpened && game.inventory[0].attack_bonus == 3);
    game.gold = 50;
    ASSERT("exact price is accepted", game_sharpen_weapon(&game, 0) && !game.gold);

    start();
    game_equip_main_hand(&game, 0);
    game.inventory[1].attack_bonus = 3;
    game_equip_off_hand(&game, 1);
    attack = game.player.attack;
    ASSERT("odd off-hand bonus retains existing rounding after sharpening", game_sharpen_weapon(&game, 1) &&
        game.inventory[1].attack_bonus == 4 && game.player.attack == attack);
}

static void test_workshop_saves(void) {
    start();
    game_equip_main_hand(&game, 0);
    game_sharpen_weapon(&game, 0);
    game_sharpen_weapon(&game, 1);
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 1, 0});
    int attack = game.player.attack;
    ASSERT("workshop and dropped sharpened weapon save and load", save_game(&game, WORKSHOP_TEST_SLOT) && load_game(&loaded, WORKSHOP_TEST_SLOT) &&
        loaded.location == LOCATION_WORKSHOP && loaded.inventory[0].sharpened && loaded.inventory[0].attack_bonus == 4 &&
        loaded.floor_items[0].item.sharpened && loaded.floor_items[0].item.attack_bonus == 3 && loaded.player.attack == attack);
    ASSERT("save/load cannot restore sharpening allowance", !game_sharpen_weapon(&loaded, 0) && loaded.gold == game.gold);
    action_resolve_player(&loaded, (Action){ACTION_PICK_UP, 0, 0});
    int dagger = loaded.inventory_count - 1;
    ASSERT("dropped and picked-up weapon retains its allowance", loaded.inventory[dagger].sharpened &&
        !game_sharpen_weapon(&loaded, dagger) && loaded.gold == game.gold);
    game = loaded;
    castle_enter(&game, 1);
    action_resolve_player(&game, (Action){ACTION_DROP_ITEM, 0, 0});
    castle_enter(&game, 2);
    ASSERT("castle loot cache saves sharpened status", save_game(&game, WORKSHOP_TEST_SLOT) && load_game(&loaded, WORKSHOP_TEST_SLOT) &&
        loaded.castle_loot[0][0].item.sharpened && loaded.castle_loot[0][0].item.attack_bonus == 4);
    castle_enter(&loaded, 1);
    ASSERT("revisited castle floor restores sharpening metadata", loaded.floor_items[0].item.sharpened);
    remove("saves/savegame_99137.json");
}

static void test_workshop_legacy_save(void) {
    start();
    game_leave_workshop(&game);
    game.map.tiles[TOWN4_WORKSHOP_DOOR_Y][TOWN4_WORKSHOP_DOOR_X] = TILE_TOWN_PATH;
    ASSERT("legacy workshop fixture saves", save_game(&game, WORKSHOP_TEST_SLOT));
    FILE *f = fopen("saves/savegame_99137.json", "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buffer = malloc((size_t)size + 1);
    fread(buffer, 1, (size_t)size, f);
    buffer[size] = '\0';
    fclose(f);
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 86);
    cJSON *inventory = cJSON_GetObjectItem(root, "inventory");
    for (int i = 0; i < cJSON_GetArraySize(inventory); i++) {
        cJSON_DeleteItemFromObject(cJSON_GetArrayItem(inventory, i), "sharpened");
    }
    char *json = cJSON_Print(root);
    cJSON_Delete(root);
    f = fopen("saves/savegame_99137.json", "wb");
    fputs(json, f);
    fclose(f);
    free(json);
    ASSERT("version 86 preserves gold and existing weapons", load_game(&loaded, WORKSHOP_TEST_SLOT) &&
        loaded.gold == game.gold && loaded.inventory[0].attack_bonus == game.inventory[0].attack_bonus &&
        loaded.inventory_count == game.inventory_count && !loaded.inventory[0].sharpened);
    ASSERT("legacy Ridgeshire save receives usable workshop doorway", loaded.map.tiles[TOWN4_WORKSHOP_DOOR_Y][TOWN4_WORKSHOP_DOOR_X] == TILE_WORKSHOP_DOOR);
    ASSERT("migrated workshop save rewrites and reloads", save_game(&loaded, WORKSHOP_TEST_SLOT) && load_game(&game, WORKSHOP_TEST_SLOT));
    remove("saves/savegame_99137.json");
}

static void test_workshop_screen(void) {
    WorkshopScreen screen = {0};
    ASSERT("empty service cannot charge gold", workshop_handle_key(&screen, SDL_SCANCODE_RETURN, 0) == WORKSHOP_NONE);
    workshop_handle_key(&screen, SDL_SCANCODE_UP, 7);
    ASSERT("service selection cannot move above inventory", screen.selected == 0);
    for (int i = 0; i < 12; i++) {
        workshop_handle_key(&screen, SDL_SCANCODE_DOWN, 7);
    }
    ASSERT("service selection cannot move below inventory", screen.selected == 6);
    ASSERT("enter requests sharpening and escape returns to workshop", workshop_handle_key(&screen, SDL_SCANCODE_RETURN, 7) == WORKSHOP_SHARPEN &&
        workshop_handle_key(&screen, SDL_SCANCODE_ESCAPE, 7) == WORKSHOP_CLOSED);
    screen.selected = 0;
    workshop_handle_click(&screen, 400, 150 + 32, 800, 600, 7);
    ASSERT("mouse selects weapon before payment", screen.selected == 1);
    SDL_Rect button = workshop_button_rect(800, 600);
    ASSERT("explicit mouse button requests sharpening", workshop_handle_click(&screen, button.x + 10, button.y + 10, 800, 600, 7) == WORKSHOP_SHARPEN);
}

void test_workshop(void) {
    printf("Ridgeshire workshop\n");
    test_workshop_travel();
    test_workshop_purchase();
    test_workshop_saves();
    test_workshop_legacy_save();
    test_workshop_screen();
}
