#include "test_utils.h"
#include "equipment_test_utils.h"
#include "game/game.h"
#include "game/elderwood.h"
#include "game/combat_feedback.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define POISON_SLOT 99216
#define POISON_SAVE "saves/savegame_99216.json"

static GameState game;
static GameState loaded;

static void setup_combat(void) {
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    game.location = LOCATION_DUNGEON;
    game.level = 1;
    game.player.x = 10;
    game.player.y = 10;
    for (int y = 5; y <= 16; y++) {
        for (int x = 5; x <= 16; x++) {
            game.map.tiles[y][x] = TILE_FLOOR;
        }
    }
    game.inventory[game.inventory_count++] = item_make_poison_sword();
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, game.inventory_count - 1, 0});
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.active = 1, .type = ENEMY_BANDIT, .x = 11, .y = 10,
        .hp = 200, .max_hp = 200, .frozen_turns = 10, .experience = 12};
    strcpy(game.enemies[0].name, "Bandit");
    combat_feedback_clear();
}

static void test_poison_combat(void) {
    setup_combat();
    action_resolve_player(&game, (Action){ACTION_MOVE, 11, 10});
    Enemy *enemy = &game.enemies[0];
    int hp = enemy->hp;
    ASSERT("a main-hand Poison Sword hit applies poison without immediate extra damage", hp == 200 - game.player.attack && enemy->poison_turns == 3);
    action_resolve_player(&game, (Action){ACTION_MOVE, 11, 10});
    ASSERT("repeated sword hits refresh poison instead of stacking it", enemy->poison_turns == 3);
    hp = enemy->hp;
    for (int turn = 0; turn < 4; turn++) {
        action_resolve_enemies(&game);
    }
    ASSERT("poison deals exactly one HP for three enemy phases and then expires, including while frozen", enemy->hp == hp - 3 && !enemy->poison_turns);
    const CombatFeedbackEvent *event = combat_feedback_get(2);
    ASSERT("poison ticks produce separate combat feedback", event && event->kind == FEEDBACK_ENEMY_POISON && event->amount == 1);
    setup_combat();
    game.inventory[game.inventory_count++] = item_make_buckler();
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, game.inventory_count - 1, 0});
    ASSERT("the Poison Sword can be paired with a shield", game.equipped_off_hand == EQUIPMENT_WEAK_HAND);
    game.inventory[game.inventory_count++] = item_make_short_sword();
    action_resolve_player(&game, (Action){ACTION_EQUIP_ITEM, game.inventory_count - 1, 0});
    action_resolve_player(&game, (Action){ACTION_EQUIP_OFF_HAND, game.inventory_count - 1, 0});
    action_resolve_player(&game, (Action){ACTION_MOVE, 11, 10});
    ASSERT("off-hand poison swords contribute attack but never apply poison", game.equipped_off_hand == EQUIPMENT_WEAK_HAND && !game.enemies[0].poison_turns);
    setup_combat();
    enemy = &game.enemies[0];
    enemy->hp = 1;
    enemy->poison_turns = 3;
    int xp = game.player.experience;
    int score = game.score;
    action_resolve_enemies(&game);
    int rewarded = game.score;
    action_resolve_enemies(&game);
    ASSERT("poison kills award XP and score once, clear poison and prevent retaliation", !enemy->active && !enemy->poison_turns &&
        game.player.experience == xp + 12 && rewarded > score && game.score == rewarded && game.player.hp == game.player.max_hp);
    setup_combat();
    action_resolve_player(&game, (Action){ACTION_RANGED_ATTACK, 0, 0});
    ASSERT("the Poison Sword never inherits the Demonic Sword's ranged attack", game.enemies[0].hp == 200 &&
        !game.enemies[0].poison_turns && !game.trail_count);
}

static void test_poison_save(void) {
    if (save_exists(POISON_SLOT)) {
        ASSERT("poison tests use an unused save slot", 0);
        return;
    }
    setup_combat();
    map_generate(&game.map, 1);
    game.player.x = game.map.stairs_up_x;
    game.player.y = game.map.stairs_up_y;
    game.enemies[0].hp = 87;
    game.enemies[0].poison_turns = 2;
    game.level_cache[0].valid = 1;
    game.level_cache[0].map = game.map;
    game.level_cache[0].enemy_count = 1;
    game.level_cache[0].enemies[0] = game.enemies[0];
    game.inventory[EQUIPMENT_STRONG_HAND].attack_bonus++;
    game.player.attack++;
    game.inventory[EQUIPMENT_STRONG_HAND].sharpened = 1;
    ASSERT("save/load retains active and cached poison and sharpened Poison Sword identity", save_game(&game, POISON_SLOT) && load_game(&loaded, POISON_SLOT) &&
        loaded.enemies[0].hp == 87 && loaded.enemies[0].poison_turns == 2 && loaded.level_cache[0].enemies[0].poison_turns == 2 &&
        loaded.inventory[EQUIPMENT_STRONG_HAND].visual_id == ITEM_VISUAL_POISON_SWORD && loaded.inventory[EQUIPMENT_STRONG_HAND].attack_bonus == 17 && loaded.inventory[EQUIPMENT_STRONG_HAND].sharpened);
    game = loaded;
    elderwood_enter(&game);
    game.enemies[0].hp = 11;
    game.enemies[0].poison_turns = 3;
    elderwood_store(&game);
    ASSERT("poison also survives compact regional cache serialization", save_game(&game, POISON_SLOT) && load_game(&loaded, POISON_SLOT) &&
        loaded.enemies[0].hp == 11 && loaded.enemies[0].poison_turns == 3 && loaded.elderwood_cache[0].enemies[0].poison_turns == 3);
    FILE *file = fopen(POISON_SAVE, "rb");
    if (!file) {
        ASSERT("legacy poison fixture opens", 0);
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *text = malloc(length + 1);
    fread(text, 1, length, file);
    text[length] = '\0';
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    test_legacy_equipment_json(root);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 215);
    cJSON *enemies = cJSON_GetObjectItem(root, "enemies");
    for (int i = 0; i < cJSON_GetArraySize(enemies); i++) {
        cJSON_DeleteItemFromObject(cJSON_GetArrayItem(enemies, i), "poison_turns");
    }
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen(POISON_SAVE, "wb");
    fputs(text, file);
    fclose(file);
    free(text);
    ASSERT("older saves default missing poison to zero without healing enemies or changing gear", load_game(&loaded, POISON_SLOT) &&
        loaded.enemies[0].hp == 11 && !loaded.enemies[0].poison_turns && loaded.inventory[EQUIPMENT_STRONG_HAND].visual_id == ITEM_VISUAL_POISON_SWORD);
    remove(POISON_SAVE);
}

void test_poison_sword(void) {
    printf("Poison Sword tests:\n");
    ShopScreen shop;
    Item sword = item_make_poison_sword();
    ASSERT("the green Poison Sword is a rare one-handed Warrior/Rogue sword just under the Magic Greatsword", sword.weapon_family == WEAPON_FAMILY_SWORD &&
        sword.weapon_hands == WEAPON_HANDS_ONE && sword.class_mask == (ITEM_CLASS_WARRIOR | ITEM_CLASS_ROGUE) && sword.rarity == ITEM_RARITY_RARE &&
        sword.attack_bonus == 16 && sword.attack_bonus + 1 < item_make_magic_greatsword().attack_bonus && !sword.is_ranged);
    shop_init(&shop, SHOP_TYPE_BLACKSMITH, 1 << LOCATION_DUNGEON, CLASS_ROGUE);
    int found = 0;
    for (int i = 0; i < shop.item_count; i++) {
        found |= shop.items[i].visual_id == ITEM_VISUAL_POISON_SWORD;
    }
    ASSERT("Poison Sword is absent below the magical stock tier", !found);
    shop_init(&shop, SHOP_TYPE_BLACKSMITH, (1 << LOCATION_DUNGEON) | (1 << LOCATION_FOREST), CLASS_ROGUE);
    found = 0;
    for (int i = 0; i < shop.item_count; i++) {
        found |= shop.items[i].visual_id == ITEM_VISUAL_POISON_SWORD;
    }
    setup_combat();
    game.gold = 2000;
    int count = game.inventory_count;
    ASSERT("the magical blacksmith tier sells the Poison Sword for 2000 gold", found && shop_purchase(&game, &sword) &&
        !game.gold && game.inventory_count == count + 1 && game.inventory[count].visual_id == ITEM_VISUAL_POISON_SWORD);
    test_poison_combat();
    test_poison_save();
}
