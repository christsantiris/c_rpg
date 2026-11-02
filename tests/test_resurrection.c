#include "test_utils.h"
#include "town_road_checks.h"
#include "game/aldric.h"
#include "game/elowen.h"
#include "game/resurrection.h"
#include "screens/shop.h"
#include "systems/save_load.h"
#include "cJSON.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>

#define RESURRECTION_SLOT 99246
static GameState g;
static GameState loaded;

static void start(PlayerClass cls) {
    memset(&g, 0, sizeof(g));
    g.player.player_class = cls;
    game_init(&g);
    game_enter_town_gate(&g, LOCATION_TOWN, 20, 0);
    action_resolve_player(&g, (Action){ACTION_MOVE, 20, 0});
}

static void die(void) {
    g.aldric_party_state = ALDRIC_FOLLOWING;
    g.aldric_hp = 1;
    g.aldric_x = g.player.x - 1;
    g.aldric_y = g.player.y;
    Enemy enemy = {.attack = 100};
    strcpy(enemy.name, "Test enemy");
    aldric_take_hit(&g, &enemy);
}

static void warlock(void) {
    g.dialogue_active = 0;
    g.resurrection_prompt = RESURRECTION_NONE;
    walk_town_road_to(&g, SOULSTONE_WARLOCK_X, SOULSTONE_WARLOCK_Y - 1);
    ASSERT("Talk reaches the actual warlock from the shrine path", resurrection_talk(&g));
}

static void mage_circle(void) {
    start(CLASS_MAGE);
    g.player.level = RESURRECTION_LEVEL;
    g.player.mp = 200;
    g.player.max_mp = 200;
    g.player.known_spells[0] = spell_make_resurrection();
    g.player.known_spell_count = 1;
    g.player.equipped_spell = 0;
    die();
    walk_town_road_to(&g, SOULSTONE_RITUAL_X, SOULSTONE_RITUAL_Y);
    g.player.last_dx = 0;
    g.player.last_dy = 0;
}

static void paid_rituals(void) {
    for (int cls = CLASS_WARRIOR; cls <= CLASS_ROGUE; cls++) {
        start(cls);
        ASSERT("the shrine and warlock exist before the Lich or recruitment", !g.defeated_bosses && g.map.tiles[SOULSTONE_Y][SOULSTONE_X] == TILE_SOULSTONE_SHRINE &&
            g.map.tiles[SOULSTONE_WARLOCK_Y][SOULSTONE_WARLOCK_X] == TILE_NPC_WARLOCK);
        g.gold = 1000;
        warlock();
        resurrection_handle_key(&g, SDL_SCANCODE_1, 0);
        ASSERT("gold alone cannot buy resurrection before recruiting a companion", g.aldric_party_state == ALDRIC_WAITING && g.gold == 1000 && g.resurrection_prompt == RESURRECTION_SERVICE);
        resurrection_handle_key(&g, SDL_SCANCODE_2, 0);
        ASSERT("a player without a party cannot buy the scroll", g.gold == 1000 && g.resurrection_prompt == RESURRECTION_SERVICE);
        resurrection_handle_key(&g, SDL_SCANCODE_ESCAPE, 0);
        die();
        ASSERT("a lethal hit preserves a dead party member with zero HP", g.aldric_party_state == ALDRIC_DEAD && g.aldric_hp == 0 && !aldric_at(&g, g.player.x - 1, g.player.y));
        warlock();
        g.gold = RESURRECTION_GOLD - 1;
        resurrection_handle_key(&g, SDL_SCANCODE_1, 0);
        resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
        ASSERT("insufficient payment leaves gold and death unchanged", g.gold == RESURRECTION_GOLD - 1 && g.aldric_party_state == ALDRIC_DEAD);
        g.gold = RESURRECTION_GOLD;
        resurrection_handle_key(&g, SDL_SCANCODE_N, 0);
        ASSERT("canceling a revival spends no gold", !resurrection_prompt_active(&g) && g.gold == RESURRECTION_GOLD && g.aldric_party_state == ALDRIC_DEAD);
        warlock();
        resurrection_handle_key(&g, SDL_SCANCODE_1, 0);
        ASSERT("a paid confirmation survives saving", save_game(&g, RESURRECTION_SLOT) && load_game(&loaded, RESURRECTION_SLOT) &&
            loaded.resurrection_prompt == RESURRECTION_PAID_CONFIRM && resurrection_prompt_active(&loaded) && loaded.aldric_hp == 0);
        g = loaded;
        resurrection_handle_key(&g, SDL_SCANCODE_Y, 1);
        ASSERT("a repeated key cannot confirm the ritual", g.gold == RESURRECTION_GOLD && g.aldric_party_state == ALDRIC_DEAD);
        int result = resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
        ASSERT("every class can pay 200 gold to revive Aldric at full HP", result == 1 && g.gold == 0 && g.aldric_party_state == ALDRIC_FOLLOWING && g.aldric_hp == ALDRIC_MAX_HP &&
            map_is_walkable(&g.map, g.aldric_x, g.aldric_y) && abs(g.aldric_x - g.player.x) + abs(g.aldric_y - g.player.y) == 1);
        resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
        ASSERT("a confirmation cannot revive or charge twice", g.gold == 0 && !resurrection_prompt_active(&g));
        walk_town_road_to(&g, OAKHAVEN_FERRY_X, OAKHAVEN_FERRY_Y);
        action_resolve_player(&g, (Action){ACTION_MOVE, OAKHAVEN_FERRY_X, OAKHAVEN_RIVER_EDGE + 1});
        action_resolve_player(&g, (Action){ACTION_MOVE, OAKHAVEN_FERRY_X, OAKHAVEN_RIVER_EDGE});
        ASSERT("the shrine never opens the river crossing", g.player.y == OAKHAVEN_RIVER_EDGE + 1 && !map_is_walkable(&g.map, OAKHAVEN_FERRY_X, OAKHAVEN_RIVER_EDGE));
        walk_town_road_to(&g, OAKHAVEN_FERRY_X, TOWN_SPRITE_Y);
        ASSERT("a revived party walks back through Oakhaven's real north entrance", g.location == LOCATION_TOWN && g.player.y == 1 && g.aldric_party_state == ALDRIC_FOLLOWING);
    }
}

static void scroll_learning(void) {
    start(CLASS_MAGE);
    die();
    warlock();
    g.gold = RESURRECTION_SCROLL_GOLD;
    g.player.level = RESURRECTION_LEVEL - 1;
    resurrection_handle_key(&g, SDL_SCANCODE_2, 0);
    ASSERT("the warlock keeps Resurrection for experienced Mages", g.gold == RESURRECTION_SCROLL_GOLD && g.resurrection_prompt == RESURRECTION_SERVICE);
    g.player.level++;
    g.aldric_party_state = ALDRIC_WAITING;
    g.aldric_hp = ALDRIC_MAX_HP;
    ASSERT("past recruitment is recorded after a companion has fallen", g.aldric_recruited);
    ASSERT("past recruitment remains unlocked through saving", save_game(&g, RESURRECTION_SLOT) && load_game(&loaded, RESURRECTION_SLOT) && loaded.aldric_recruited);
    g = loaded;
    g.gold--;
    resurrection_handle_key(&g, SDL_SCANCODE_2, 0);
    ASSERT("an unaffordable scroll does not open a purchase confirmation", g.resurrection_prompt == RESURRECTION_SERVICE);
    g.gold++;
    int bag = g.inventory_count;
    g.inventory_count = MAX_INVENTORY;
    resurrection_handle_key(&g, SDL_SCANCODE_2, 0);
    ASSERT("a full bag prevents scroll purchase without spending gold", g.gold == RESURRECTION_SCROLL_GOLD && g.resurrection_prompt == RESURRECTION_SERVICE);
    g.inventory_count = bag;
    resurrection_handle_key(&g, SDL_SCANCODE_2, 0);
    resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("the scroll costs 400 gold and uses a normal bag slot", g.gold == 0 && g.inventory_count == bag + 1 && g.inventory[bag].spell_id == SPELL_RESURRECTION && shop_buy_price(&g.inventory[bag]) == RESURRECTION_SCROLL_GOLD);
    g.player.player_class = CLASS_WARRIOR;
    action_resolve_player(&g, (Action){ACTION_USE_ITEM, bag, 0});
    ASSERT("a Warrior cannot learn the resurrection scroll", g.inventory_count == bag + 1 && g.player.known_spell_count == 0);
    g.player.player_class = CLASS_MAGE;
    g.player.level = RESURRECTION_LEVEL - 1;
    action_resolve_player(&g, (Action){ACTION_USE_ITEM, bag, 0});
    ASSERT("an inexperienced Mage retains an unread scroll", g.inventory_count == bag + 1 && g.player.known_spell_count == 0);
    g.player.level++;
    action_resolve_player(&g, (Action){ACTION_USE_ITEM, bag, 0});
    ASSERT("a level 10 Mage permanently learns Resurrection from the scroll", g.inventory_count == bag && g.player.known_spell_count == 1 &&
        g.player.known_spells[0].id == SPELL_RESURRECTION && g.player.known_spells[0].mp_cost == RESURRECTION_MP && !spell_upgrade(&g.player.known_spells[0]));
    g.gold = RESURRECTION_SCROLL_GOLD;
    warlock();
    resurrection_handle_key(&g, SDL_SCANCODE_2, 0);
    ASSERT("the warlock does not sell a redundant known spell", g.gold == RESURRECTION_SCROLL_GOLD && g.resurrection_prompt == RESURRECTION_SERVICE);
}

static void spell_rituals(void) {
    mage_circle();
    ASSERT("the Cast key can be remapped for the shrine", controls_assign(g.key_bindings, CONTROL_CAST_SPELL, SDL_SCANCODE_K));
    action_resolve_player(&g, (Action){ACTION_MOVE, SOULSTONE_RITUAL_X + 1, SOULSTONE_RITUAL_Y});
    action_resolve_player(&g, (Action){ACTION_MOVE, SOULSTONE_RITUAL_X, SOULSTONE_RITUAL_Y});
    ASSERT("the ritual position names the character's remapped Cast key", strstr(g.messages[g.message_count - 1], "press K") != NULL);
    int bag = g.inventory_count;
    g.inventory[g.inventory_count++] = item_make_health_potion();
    action_resolve_player(&g, (Action){ACTION_DROP_ITEM, bag, 0});
    ASSERT("dropped loot leaves the actual ritual marker visible", g.map.tiles[SOULSTONE_RITUAL_Y][SOULSTONE_RITUAL_X] == TILE_SOULSTONE_RITUAL);
    action_resolve_player(&g, (Action){ACTION_PICK_UP, 0, 0});
    ASSERT("loot on the ritual marker stays collectible", g.inventory_count == bag + 1 && g.map.tiles[SOULSTONE_RITUAL_Y][SOULSTONE_RITUAL_X] == TILE_SOULSTONE_RITUAL);
    g.player.last_dx = 0;
    g.player.last_dy = 0;
    g.player.mp = RESURRECTION_MP - 1;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("insufficient mana cannot start or pay for a ritual", g.player.mp == RESURRECTION_MP - 1 && !resurrection_prompt_active(&g) && g.aldric_party_state == ALDRIC_DEAD);
    g.player.mp = 200;
    g.player.x++;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("a nearby Mage must stand on the marked ritual position", !resurrection_prompt_active(&g) && g.player.mp == 200);
    g.player.x--;
    g.player.level--;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("a known spell cannot bypass the Mage level requirement", !resurrection_prompt_active(&g));
    g.player.level++;
    g.enemies[0] = (Enemy){.active = 1, .x = 40, .y = 40};
    g.enemy_count = 1;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("even the correct shrine position rejects combat resurrection", !resurrection_prompt_active(&g) && g.player.mp == 200);
    g.enemy_count = 0;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("a Mage starts the ritual directly without talking to the warlock or aiming", g.resurrection_prompt == RESURRECTION_SPELL_CONFIRM && g.player.mp == 200);
    resurrection_handle_key(&g, SDL_SCANCODE_ESCAPE, 0);
    ASSERT("canceling a Mage ritual preserves MP and death", g.player.mp == 200 && g.aldric_party_state == ALDRIC_DEAD);
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("a Mage's ritual confirmation and learned spell survive loading", save_game(&g, RESURRECTION_SLOT) && load_game(&loaded, RESURRECTION_SLOT) &&
        loaded.resurrection_prompt == RESURRECTION_SPELL_CONFIRM && loaded.player.known_spells[0].id == SPELL_RESURRECTION);
    g = loaded;
    int result = resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("the Mage pays 80 MP and revives Aldric at half HP for one turn", result == 2 && g.player.mp == 120 && g.aldric_hp == ALDRIC_MAX_HP / 2 && g.aldric_party_state == ALDRIC_FOLLOWING);
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("a living companion cannot be healed by recasting Resurrection", !resurrection_prompt_active(&g) && g.player.mp == 120 && g.aldric_hp == ALDRIC_MAX_HP / 2);

    mage_circle();
    Item robes = item_make_archmage_robes();
    robes.spell_cost_reduction_percent = 25;
    g.inventory[g.inventory_count++] = robes;
    ASSERT("the Mage can equip spell-cost gear", game_equip_item(&g, g.inventory_count - 1, EQUIPMENT_ARMOR));
    int mp = g.player.mp;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("Resurrection respects the existing spell-cost reduction", mp - g.player.mp == 60 && g.aldric_hp == ALDRIC_MAX_HP / 2);

    mage_circle();
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    const int dx[] = {0, -1, 1, 0};
    const int dy[] = {1, 0, 0, -1};
    for (int i = 0; i < 4; i++) {
        g.map.tiles[g.player.y + dy[i]][g.player.x + dx[i]] = TILE_WALL;
    }
    resurrection_handle_key(&g, SDL_SCANCODE_Y, 0);
    ASSERT("blocked revival placement spends no MP and keeps the companion dead", g.player.mp == 200 && g.aldric_party_state == ALDRIC_DEAD);
    g.location = LOCATION_FOREST;
    g.map.tiles[g.player.y][g.player.x] = TILE_FOREST_FLOOR;
    g.dialogue_active = 0;
    g.resurrection_prompt = 0;
    action_resolve_player(&g, (Action){ACTION_CAST_SPELL, 0, 0});
    ASSERT("matching coordinates in another area never enable field resurrection", g.player.mp == 200 && g.aldric_party_state == ALDRIC_DEAD && !resurrection_prompt_active(&g));
}

static void legacy_save(void) {
    start(CLASS_WARRIOR);
    die();
    g.gold = 523;
    g.player.hp -= 7;
    int hp = g.player.hp;
    g.player.x = SOULSTONE_X;
    g.player.y = SOULSTONE_Y;
    g.map.tiles[SOULSTONE_Y][SOULSTONE_X] = TILE_ITEM;
    g.floor_item_count = 1;
    g.floor_items[0] = (FloorItem){.active = 1, .x = SOULSTONE_X, .y = SOULSTONE_Y, .underlying_tile = TILE_TOWN_FLOOR, .item = item_make_health_potion()};
    map_mark_explored(&g.map, SOULSTONE_X, SOULSTONE_Y);
    ASSERT("a legacy fixture saves before adding the shrine", save_game(&g, RESURRECTION_SLOT));
    FILE *file = fopen("saves/savegame_99246.json", "rb");
    if (!file) {
        ASSERT("legacy resurrection save is readable", 0);
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    char *text = calloc((size_t)length + 1, 1);
    fread(text, 1, (size_t)length, file);
    fclose(file);
    cJSON *root = cJSON_Parse(text);
    free(text);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "save_version"), 233);
    cJSON_SetNumberValue(cJSON_GetObjectItem(root, "aldric_hp"), ALDRIC_MAX_HP);
    cJSON_DeleteItemFromObject(root, "resurrection_prompt");
    cJSON_DeleteItemFromObject(root, "aldric_recruited");
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    file = fopen("saves/savegame_99246.json", "wb");
    fputs(text, file);
    fclose(file);
    free(text);
    int ok = load_game(&loaded, RESURRECTION_SLOT);
    ASSERT("legacy recovery converts to death while retaining gold, wounds and discovery", ok && loaded.aldric_party_state == ALDRIC_DEAD && loaded.aldric_hp == 0 && loaded.gold == 523 && loaded.player.hp == hp &&
        map_is_explored(&loaded.map, SOULSTONE_X, SOULSTONE_Y) && loaded.resurrection_prompt == RESURRECTION_NONE);
    ASSERT("adding shrine collision preserves a playable position and collectible loot", ok && map_is_walkable(&loaded.map, loaded.player.x, loaded.player.y) && loaded.floor_items[0].active &&
        map_is_walkable(&loaded.map, loaded.floor_items[0].x, loaded.floor_items[0].y) && loaded.floor_items[0].item.type == ITEM_POTION_HEALTH);
}

void test_resurrection(void) {
    printf("Soulstone Shrine and party resurrection:\n");
    ASSERT("resurrection tests use an empty isolated save slot", !save_exists(RESURRECTION_SLOT));
    paid_rituals();
    scroll_learning();
    spell_rituals();
    legacy_save();
    remove("saves/savegame_99246.json");
}
