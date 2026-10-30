#include "test_utils.h"
#include "game/game.h"
#include "screens/inventory.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

static void potion_fixture(GameState *g) {
    game_init(g);
    g->location = LOCATION_DUNGEON;
    g->level = 1;
    g->player.x = 10;
    g->player.y = 10;
    g->player.hp = 50;
    g->player.max_hp = 100;
    g->player.mp = 5;
    g->player.max_mp = 20;
    g->inventory_count = 2;
    g->inventory[0] = item_make_health_potion();
    g->inventory[1] = item_make_mana_potion();
    g->enemy_count = 1;
    g->enemies[0] = (Enemy){.active = 1, .type = ENEMY_SKELETON, .x = 11, .y = 10, .hp = 20, .max_hp = 20, .attack = 5};
    strcpy(g->enemies[0].name, "Skeleton");
    for (int y = 8; y < 13; y++) {
        for (int x = 8; x < 13; x++) {
            g->map.tiles[y][x] = TILE_FLOOR;
        }
    }
}

void test_quick_potion(void) {
    printf("Quick potion controls and persistence:\n");
    GameState *g = calloc(1, sizeof(*g));
    GameState *normal = calloc(1, sizeof(*normal));
    ASSERT("quick potion fixtures allocate", g && normal);
    if (!g || !normal) {
        free(g);
        free(normal);
        return;
    }
    potion_fixture(g);
    ASSERT("new players default to Health on remappable V", g->quick_potion_type == ITEM_POTION_HEALTH && g->key_bindings[CONTROL_DRINK_POTION] == SDL_SCANCODE_V);
    InventoryScreen inv;
    inventory_init(&inv);
    ASSERT("inventory Q designates a quick potion without consuming it", inventory_handle_key(&inv, SDL_SCANCODE_Q, 2, SDL_SCANCODE_I) == INVENTORY_SET_QUICK_POTION && game_set_quick_potion(g, 1) && g->inventory_count == 2 && g->player.mp == 5 && game_quick_potion_count(g) == 1);
    g->inventory[0] = item_make_strength_potion();
    ASSERT("only Health and Mana may be designated", !game_set_quick_potion(g, 0) && g->quick_potion_type == ITEM_POTION_MANA && !game_set_quick_potion(g, -1) && !game_set_quick_potion(g, 2));
    inv.equipment_tab = 1;
    ASSERT("equipment cannot be designated as a potion", inventory_handle_key(&inv, SDL_SCANCODE_Q, 2, SDL_SCANCODE_I) == INVENTORY_NONE);
    for (int type = ITEM_POTION_HEALTH; type <= ITEM_POTION_MANA; type++) {
        for (int status = 0; status < 3; status++) {
            potion_fixture(g);
            g->quick_potion_type = type;
            g->player.poison_turns = status == 1 ? 2 : 0;
            g->player.frozen_turns = status == 2 ? 1 : 0;
            memcpy(normal, g, sizeof(*g));
            EnemyProjectiles quick_shots = {0};
            EnemyProjectiles normal_shots = {0};
            srand(124);
            int quick = action_drink_quick_potion(g, &quick_shots);
            srand(124);
            int used = action_use_inventory_item(normal, type == ITEM_POTION_HEALTH ? 0 : 1, &normal_shots);
            ASSERT("quick drinking exactly matches inventory effects, status ticks and enemy retaliation", quick && used && g->inventory_count == 1 && g->player.hp == normal->player.hp && g->player.mp == normal->player.mp && g->player.frozen_turns == normal->player.frozen_turns && g->player.poison_turns == normal->player.poison_turns && g->enemies[0].move_timer == normal->enemies[0].move_timer && quick_shots.count == normal_shots.count);
        }
    }
    potion_fixture(g);
    g->player.hp = 100;
    g->player.poison_turns = 2;
    ASSERT("full HP refuses consumption and a combat turn", !action_drink_quick_potion(g, NULL) && g->inventory_count == 2 && g->player.poison_turns == 2 && g->player.hp == 100);
    game_set_quick_potion(g, 1);
    g->player.mp = 20;
    ASSERT("full MP refuses consumption and a combat turn", !action_drink_quick_potion(g, NULL) && g->inventory_count == 2 && g->player.poison_turns == 2);
    g->player.mp = 5;
    ASSERT("designated Mana is consumed", action_drink_quick_potion(g, NULL) && game_quick_potion_count(g) == 0);
    int hp = g->player.hp;
    int poison = g->player.poison_turns;
    ASSERT("depletion keeps the preference and never substitutes Health", !action_drink_quick_potion(g, NULL) && g->quick_potion_type == ITEM_POTION_MANA && g->inventory_count == 1 && g->inventory[0].type == ITEM_POTION_HEALTH && g->player.hp == hp && g->player.poison_turns == poison);
    ASSERT("U still permits the other potion type", action_use_inventory_item(g, 0, NULL) && g->inventory_count == 0 && g->quick_potion_type == ITEM_POTION_MANA);
    potion_fixture(g);
    game_set_quick_potion(g, 1);
    controls_assign(g->key_bindings, CONTROL_DRINK_POTION, SDL_SCANCODE_G);
    ASSERT("potion preference and remapped action round-trip through saves", save_game(g, 99228) && load_game(normal, 99228) && normal->quick_potion_type == ITEM_POTION_MANA && normal->key_bindings[CONTROL_DRINK_POTION] == SDL_SCANCODE_G);
    FILE *file = fopen("saves/savegame_99228.json", "rb");
    ASSERT("saved potion fixture is readable", file != NULL);
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);
        char *text = malloc((size_t)size + 1);
        size_t count = fread(text, 1, (size_t)size, file);
        fclose(file);
        text[count] = '\0';
        cJSON *root = cJSON_Parse(text);
        free(text);
        cJSON_DeleteItemFromObject(root, "quick_potion_type");
        cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 226);
        cJSON *keys = cJSON_GetObjectItem(root, "key_bindings");
        cJSON_DeleteItemFromArray(keys, CONTROL_DRINK_POTION);
        cJSON_SetNumberValue(cJSON_GetArrayItem(keys, CONTROL_INVENTORY), SDL_SCANCODE_V);
        text = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        file = fopen("saves/savegame_99228.json", "wb");
        fputs(text, file);
        fclose(file);
        free(text);
        ASSERT("older saves preserve existing V remaps and choose a free Drink key", load_game(normal, 99228) && normal->quick_potion_type == ITEM_POTION_HEALTH && normal->key_bindings[CONTROL_INVENTORY] == SDL_SCANCODE_V && normal->key_bindings[CONTROL_DRINK_POTION] != SDL_SCANCODE_V && controls_valid(normal->key_bindings));
    }
    remove("saves/savegame_99228.json");
    free(g);
    free(normal);
}
