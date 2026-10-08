#include "test_utils.h"
#include "game/game.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

#define ARROW_SLOT 99150
#define ARROW_SAVE "saves/savegame_99150.json"
static GameState game;
static GameState loaded;

static int rewrite_arrow_save(int version, int count) {
    FILE *file = fopen(ARROW_SAVE, "rb");
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
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), version);
    cJSON *player = cJSON_GetObjectItem(root, "player");
    cJSON_DeleteItemFromObject(player, "arrows");
    if (count != -2) {
        cJSON_AddNumberToObject(player, "arrows", count);
    }
    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!json) {
        return 0;
    }
    file = fopen(ARROW_SAVE, "wb");
    int written = file && fputs(json, file) >= 0;
    if (file) {
        fclose(file);
    }
    free(json);
    return written;
}

void test_arrows(void) {
    printf("Rogue arrow supply tests:\n");
    ASSERT("arrow test save slot is unused", !save_exists(ARROW_SLOT));
    memset(&game, 0, sizeof(game));
    game.player.player_class = CLASS_ROGUE;
    game_init(&game);
    ASSERT("a new Rogue receives 100 arrows", game.player.arrows == MAX_ARROWS);
    game.equipped_main_hand = 0;
    game.player.x = 20;
    game.player.y = 12;
    game.enemy_count = 0;
    for (int x = 20; x <= 32; x++) {
        game.map.tiles[12][x] = TILE_TOWN_FLOOR;
    }
    Action fire = {ACTION_RANGED_ATTACK, 0, 0};
    action_resolve_player(&game, fire);
    ASSERT("an unaimed shot spends no arrows", game.player.arrows == MAX_ARROWS);
    game.player.last_dx = 1;
    game.player.last_dy = 0;
    for (int i = 0; i < MAX_ARROWS; i++) {
        action_resolve_player(&game, fire);
    }
    ASSERT("100 fired misses exhaust the quiver", game.player.arrows == 0 && strstr(game.messages[game.message_count - 1], "Last arrow"));
    game.enemy_count = 1;
    game.enemies[0] = (Enemy){.active = 1, .x = 22, .y = 12, .hp = 1000, .max_hp = 1000, .type = ENEMY_SKELETON};
    game.trail_count = 0;
    action_resolve_player(&game, fire);
    ASSERT("shot 101 deals no damage and emits no projectile", game.player.arrows == 0 && game.enemies[0].hp == 1000 && game.trail_count == 0 &&
        strstr(game.messages[game.message_count - 1], "No arrows"));
    game.enemies[0].x = 21;
    action_resolve_player(&game, (Action){ACTION_MOVE, 21, 12});
    ASSERT("without a dagger, an empty bow still allows melee hits", game.inventory_count == 1 && game.inventory[0].weapon_family == WEAPON_FAMILY_BOW &&
        game.enemies[0].hp == 1000 - (game.player.attack - game.inventory[0].attack_bonus) && game.player.arrows == 0);
    game.player.arrows = 1;
    action_resolve_player(&game, fire);
    ASSERT("an adjacent target prevents firing without spending the last arrow", game.player.arrows == 1);
    game.enemies[0].x = 22;
    game.map.tiles[12][21] = TILE_WALL;
    int blocked_hp = game.enemies[0].hp;
    action_resolve_player(&game, fire);
    ASSERT("an arrow fired into a wall is spent without damaging the target", game.player.arrows == 0 && game.enemies[0].hp == blocked_hp);
    game.map.tiles[12][21] = TILE_TOWN_FLOOR;
    game.player.arrows = 1;
    game.inventory[0] = item_make_magic_longbow();
    game.enemy_count = 2;
    game.enemies[1] = game.enemies[0];
    game.enemies[1].x = 24;
    action_resolve_player(&game, fire);
    ASSERT("one piercing shot uses one arrow while hitting both targets", game.player.arrows == 0 && game.enemies[0].hp < 990 && game.enemies[1].hp < 990);
    game.inventory[0] = item_make_demonic_sword();
    int hp = game.enemies[0].hp;
    action_resolve_player(&game, fire);
    ASSERT("the Demonic Sword fires with no arrows", game.enemies[0].hp < hp && game.player.arrows == 0);
    game.inventory[0] = item_make_dagger();
    hp = game.enemies[0].hp;
    action_resolve_player(&game, fire);
    ASSERT("a dagger cannot fire or consume arrows", game.enemies[0].hp == hp && game.player.arrows == 0);
    game.enemies[0].x = 21;
    action_resolve_player(&game, (Action){ACTION_MOVE, 21, 12});
    ASSERT("switching to a dagger permits melee without arrows", game.enemies[0].hp < hp && game.player.arrows == 0);

    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_BLACKSMITH, 0);
    int stocks_arrows = 0;
    for (int i = 0; i < shop.item_count; i++) {
        stocks_arrows |= shop.items[i].type == ITEM_ARROWS;
    }
    ASSERT("the Blacksmith stocks arrows before any boss victory", stocks_arrows);
    Item arrows = item_make_arrows();
    game.gold = 9;
    ASSERT("insufficient gold cannot change the quiver", !shop_purchase(&game, &arrows) && game.gold == 9 && game.player.arrows == 0);
    game.gold = 100;
    game.inventory_count = MAX_INVENTORY;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        game.inventory[i] = item_make_health_potion();
    }
    ASSERT("20 arrows cost 10 gold even with a full inventory", shop_purchase(&game, &arrows) && game.gold == 90 && game.player.arrows == 20 && game.inventory_count == MAX_INVENTORY);
    game.player.arrows = 95;
    ASSERT("a partial refill buys only five arrows for three gold", shop_purchase_price(&game, &arrows) == 3 && shop_purchase(&game, &arrows) &&
        game.player.arrows == MAX_ARROWS && game.gold == 87);
    ASSERT("a full quiver cannot be charged or overfilled", !shop_purchase(&game, &arrows) && game.gold == 87 && game.player.arrows == MAX_ARROWS);
    ASSERT("ordinary purchases still reject a full inventory", !shop_purchase(&game, &game.inventory[0]) && game.gold == 87 && game.inventory_count == MAX_INVENTORY);
    game.inventory_count = 1;
    ASSERT("ordinary item purchases retain their normal price and inventory behavior", shop_purchase(&game, &game.inventory[0]) && game.gold == 67 && game.inventory_count == 2);
    game.player.arrows = 17;
    game_enter_tavern(&game);
    game_leave_tavern(&game);
    ASSERT("travel does not refill arrows", game.player.arrows == 17);
    ASSERT("remaining arrows survive save/load", save_game(&game, ARROW_SLOT) && load_game(&loaded, ARROW_SLOT) && loaded.player.arrows == 17);
    game.player.arrows = 0;
    ASSERT("saving an empty quiver never gives free arrows", save_game(&game, ARROW_SLOT) && load_game(&loaded, ARROW_SLOT) && loaded.player.arrows == 0);
    ASSERT("legacy Rogue saves gain 100 arrows while preserving gold and inventory", rewrite_arrow_save(102, -2) && load_game(&loaded, ARROW_SLOT) &&
        loaded.player.arrows == MAX_ARROWS && loaded.gold == game.gold && loaded.inventory_count == game.inventory_count);
    loaded.player.arrows = 8;
    ASSERT("migration runs once, not on subsequent loads", save_game(&loaded, ARROW_SLOT) && load_game(&loaded, ARROW_SLOT) && loaded.player.arrows == 8);
    ASSERT("current saves reject an arrow count above the cap", rewrite_arrow_save(103, 101) && !load_game(&loaded, ARROW_SLOT));
    ASSERT("current saves reject a negative count", rewrite_arrow_save(103, -1) && !load_game(&loaded, ARROW_SLOT));
    ASSERT("current saves cannot reset ammunition by omitting the count", rewrite_arrow_save(103, -2) && !load_game(&loaded, ARROW_SLOT));
    game.player.player_class = CLASS_WARRIOR;
    game_init(&game);
    ASSERT("Warriors do not start with ammunition", game.player.arrows == 0);
    ASSERT("legacy Warrior saves do not receive Rogue arrows", save_game(&game, ARROW_SLOT) && rewrite_arrow_save(102, -2) && load_game(&loaded, ARROW_SLOT) && loaded.player.arrows == 0);
    game.player.player_class = CLASS_MAGE;
    game_init(&game);
    ASSERT("Mages do not start with ammunition", game.player.arrows == 0);
    remove(ARROW_SAVE);
}
