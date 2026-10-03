#include "test_utils.h"
#include "../src/game/map.h"
#include "../src/game/game.h"
#include "../src/screens/shop.h"
#include "../src/screens/harbor.h"
#include "../src/systems/save_load.h"
#include <string.h>

void test_king_roads_and_castle(void) {
    printf("King Roads and castle grounds tests:\n");
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_swamp(&g);
    game_enter_town3(&g);
    ASSERT("Rosemoor has an east road, apothecary, and no castle moat",
        g.location == LOCATION_TOWN3 &&
        g.map.tiles[TOWN3_KING_GATE_Y][TOWN_W - 1] == TILE_TOWN_EXIT &&
        g.map.tiles[TOWN_APOTHECARY_Y][TOWN_APOTHECARY_X] == TILE_SHOP_ALCHEMIST &&
        g.map.tiles[TOWN_APOTHECARY_DOOR_Y][TOWN_APOTHECARY_DOOR_X] == TILE_ALCHEMIST_DOOR &&
        map_is_walkable(&g.map, TOWN_MOAT_X, 5) &&
        g.map.tiles[10][20] == TILE_TOWN_PATH);
    g.player.x = TOWN_W - 2;
    g.player.y = TOWN3_KING_GATE_Y;
    action_resolve_player(&g, (Action){ACTION_MOVE, TOWN_W - 1, TOWN3_KING_GATE_Y});
    ASSERT("Rosemoor east gate enters King Road East at its west end",
        g.location == LOCATION_CROWNROAD && g.enemy_count == MAX_ENEMIES &&
        g.player.x == 1 && g.player.y == CROWNROAD_Y);
    int archers = 0;
    int horsemen = 0;
    int open = 1;
    int distinct = 1;
    for (int i = 0; i < g.enemy_count; i++) {
        archers += g.enemies[i].type == ENEMY_ROAD_ARCHER;
        horsemen += g.enemies[i].type == ENEMY_HORSEMAN;
        open &= map_is_walkable(&g.map, g.enemies[i].x, g.enemies[i].y);
        for (int j = 0; j < i; j++) {
            distinct &= g.enemies[i].x != g.enemies[j].x || g.enemies[i].y != g.enemies[j].y;
        }
    }
    ASSERT("rotated road preserves its 30-enemy ambush on distinct walkable tiles",
        archers == 8 && horsemen == 6 && open && distinct);
    g.enemies[0].active = 0;
    g.player.x = CROWNROAD_W - 2;
    g.player.y = CROWNROAD_Y;
    action_resolve_player(&g, (Action){ACTION_MOVE, CROWNROAD_W - 1, CROWNROAD_Y});
    ASSERT("King Road East reaches separate castle grounds without clearing enemies",
        g.location == LOCATION_CASTLE && g.player.x == 1 &&
        g.crownroad_cache.valid && !g.crownroad_cache.enemies[0].active);
    ASSERT("castle moat and drawbridge are now on the castle grounds",
        !map_is_walkable(&g.map, TOWN_MOAT_X, 5) &&
        map_is_walkable(&g.map, 20, 11) && g.map.tiles[10][20] == TILE_TOWN_EXIT);
    g.player.x = 20;
    g.player.y = 11;
    action_resolve_player(&g, (Action){ACTION_MOVE, 20, 10});
    ASSERT("castle interior remains sealed", g.location == LOCATION_CASTLE && g.player.y == 11);
    g.player.x = TOWN_W - 2;
    g.player.y = CASTLE_ROAD_Y;
    action_resolve_player(&g, (Action){ACTION_MOVE, TOWN_W - 1, CASTLE_ROAD_Y});
    ASSERT("castle east gate enters King Road West with independent enemies",
        g.location == LOCATION_KING_ROAD_WEST && g.player.x == 1 &&
        g.enemy_count == MAX_ENEMIES && g.enemies[0].active);
    g.enemies[1].active = 0;
    g.player.x = CROWNROAD_W - 2;
    action_resolve_player(&g, (Action){ACTION_MOVE, CROWNROAD_W - 1, CROWNROAD_Y});
    ASSERT("King Road West reaches Town 4's west entrance",
        g.location == LOCATION_TOWN4 && g.player.x == 1 && g.player.y == 12);
    const int slot = 99015;
    int restored = save_game(&g, slot) && load_game(&loaded, slot);
    ASSERT("both King Road caches survive saving in Town 4",
        restored && loaded.crownroad_cache.valid && loaded.kingroad_west_cache.valid &&
        !loaded.crownroad_cache.enemies[0].active && loaded.crownroad_cache.enemies[1].active &&
        loaded.kingroad_west_cache.enemies[0].active && !loaded.kingroad_west_cache.enemies[1].active);
    if (restored) {
        g = loaded;
    }
    action_resolve_player(&g, (Action){ACTION_MOVE, 0, 12});
    ASSERT("Town 4 returns to King Road West's east end with saved enemies",
        g.location == LOCATION_KING_ROAD_WEST && g.player.x == CROWNROAD_W - 2 &&
        !g.enemies[1].active);
    g.player.x = 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, 0, CROWNROAD_Y});
    ASSERT("King Road West returns to the castle's east gate",
        g.location == LOCATION_CASTLE && g.player.x == TOWN_W - 2);
    restored = save_game(&g, slot) && load_game(&loaded, slot);
    ASSERT("castle grounds and position survive save/load",
        restored && loaded.location == LOCATION_CASTLE &&
        loaded.player.x == TOWN_W - 2 && loaded.map.tiles[10][20] == TILE_TOWN_EXIT);
    g.player.x = 1;
    g.player.y = CASTLE_ROAD_Y;
    action_resolve_player(&g, (Action){ACTION_MOVE, 0, CASTLE_ROAD_Y});
    ASSERT("castle west gate restores King Road East's enemy progress",
        g.location == LOCATION_CROWNROAD && g.player.x == CROWNROAD_W - 2 && !g.enemies[0].active);
    g.player.x = 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, 0, CROWNROAD_Y});
    ASSERT("King Road East returns to Rosemoor's east entrance",
        g.location == LOCATION_TOWN3 && g.player.x == TOWN_W - 2 &&
        g.player.y == TOWN3_KING_GATE_Y);
    remove("saves/savegame_99015.json");
    game_init(&g);
    ASSERT("new game clears King Road West progress", !g.kingroad_west_cache.valid);
}

void test_town3_royal_guards(void) {
    printf("Rosemoor Royal Guard tests:\n");
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    game_enter_swamp(&g);
    game_enter_town3(&g);
    ASSERT("guards flank King Road East without blocking it",
        g.map.tiles[TOWN3_GUARD_NORTH_Y][TOWN3_GUARD_X] == TILE_NPC_ROYAL_GUARD &&
        g.map.tiles[TOWN3_GUARD_SOUTH_Y][TOWN3_GUARD_X] == TILE_NPC_ROYAL_GUARD &&
        g.map.tiles[TOWN3_KING_GATE_Y][TOWN3_GUARD_X] == TILE_TOWN_PATH);
    game_talk_to_royal_guard(&g, TOWN3_GUARD_X, TOWN3_GUARD_NORTH_Y);
    ASSERT("guard warns about King Road East",
        g.dialogue_active && strstr(g.dialogue_text, "King Road East") &&
        g.dialogue_y == TOWN3_GUARD_NORTH_Y);
    game_talk_to_royal_guard(&g, TOWN3_GUARD_X, TOWN3_GUARD_SOUTH_Y);
    ASSERT("second guard recommends preparation", strstr(g.dialogue_text, "grow stronger"));
    g.player.x = 28;
    g.player.y = TOWN3_KING_GATE_Y;
    for (int x = 29; x < TOWN_W; x++) {
        action_resolve_player(&g, (Action){ACTION_MOVE, x, TOWN3_KING_GATE_Y});
    }
    ASSERT("guards allow passage into the road", g.location == LOCATION_CROWNROAD);
    game_leave_crownroad(&g, LOCATION_TOWN3);
    g.map.tiles[TOWN3_GUARD_NORTH_Y][TOWN3_GUARD_X] = TILE_TOWN_FLOOR;
    g.map.tiles[TOWN3_GUARD_SOUTH_Y][TOWN3_GUARD_X] = TILE_TOWN_FLOOR;
    g.player.x = TOWN3_GUARD_X;
    g.player.y = TOWN3_GUARD_SOUTH_Y;
    int restored = save_game(&g, 99016) && load_game(&loaded, 99016);
    ASSERT("loading Rosemoor restores guards without covering the player",
        restored && loaded.map.tiles[TOWN3_GUARD_NORTH_Y][TOWN3_GUARD_X] == TILE_NPC_ROYAL_GUARD &&
        loaded.map.tiles[TOWN3_GUARD_SOUTH_Y][TOWN3_GUARD_X] == TILE_TOWN_FLOOR);
    remove("saves/savegame_99016.json");
}

static void step_into_labyrinth_tile(GameState *g, int x, int y) {
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    for (int side = 0; side < 4; side++) {
        int px = x + dx[side];
        int py = y + dy[side];
        if (map_is_walkable(&g->map, px, py) &&
            g->map.tiles[py][px] == TILE_LABYRINTH_FLOOR) {
            g->player.x = px;
            g->player.y = py;
            action_resolve_player(g, (Action){ACTION_MOVE, x, y});
            return;
        }
    }
}

static void find_labyrinth_tile(const Map *m, TileType tile, int *x, int *y) {
    *x = -1;
    *y = -1;
    for (int row = 0; row < LABYRINTH_H; row++) {
        for (int col = 0; col < LABYRINTH_W; col++) {
            if (m->tiles[row][col] == tile) {
                *x = col;
                *y = row;
                return;
            }
        }
    }
}

static void find_false_labyrinth_stair(const Map *m, int *x, int *y) {
    *x = -1;
    *y = -1;
    for (int row = 0; row < LABYRINTH_H; row++) {
        for (int col = 0; col < LABYRINTH_W; col++) {
            if (m->tiles[row][col] == TILE_LABYRINTH_STAIRS &&
                (col != m->stairs_down_x || row != m->stairs_down_y)) {
                *x = col;
                *y = row;
                return;
            }
        }
    }
}

void test_town_healer(void) {
    printf("Town potion sellers tests:\n");
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_MAGE;
    game_init(&g);

    ASSERT("healer and witch leave the starting town",
        g.map.tiles[TOWN_HEALER_DOOR_Y][TOWN_HEALER_DOOR_X] != TILE_HEALER_DOOR &&
        g.map.tiles[TOWN_WITCH_DOOR_Y][TOWN_WITCH_DOOR_X] != TILE_WITCH_DOOR);
    game_enter_town2(&g);
    ASSERT("healer and witch doors are accessible in the second town",
        g.map.tiles[TOWN_HEALER_DOOR_Y][TOWN_HEALER_DOOR_X] == TILE_HEALER_DOOR &&
        g.map.tiles[TOWN_WITCH_DOOR_Y][TOWN_WITCH_DOOR_X] == TILE_WITCH_DOOR);

    ShopScreen shop;
    shop_init(&shop, SHOP_TYPE_HEALER, 0);
    ASSERT("healer sells only health potions at the usual price",
        shop.item_count == 1 && shop.items[0].type == ITEM_POTION_HEALTH &&
        shop_buy_price(&shop.items[0]) == 20 &&
        shop_accepts_item(SHOP_TYPE_HEALER, &shop.items[0]));
    ASSERT("healer purchase uses the ordinary shop controls",
        shop_handle_key(&shop, SDL_SCANCODE_RETURN) == SHOP_BUY &&
        shop_handle_key(&shop, SDL_SCANCODE_TAB) == SHOP_NONE &&
        shop_handle_key(&shop, SDL_SCANCODE_RETURN) == SHOP_SELL &&
        shop_handle_key(&shop, SDL_SCANCODE_ESCAPE) == SHOP_CLOSED);
    g.gold = 20;
    g.player.hp -= 30;
    g.inventory[g.inventory_count++] = shop.items[0];
    g.gold -= shop_buy_price(&shop.items[0]);
    action_resolve_player(&g, (Action){ACTION_USE_ITEM, g.inventory_count - 1, 0});
    ASSERT("health potion restores HP after purchase and use",
        g.gold == 0 && g.player.hp == g.player.max_hp &&
        g.player.mp == g.player.max_mp);

    shop_init(&shop, SHOP_TYPE_WITCH, 0);
    ASSERT("witch sells only mana potions at the usual price",
        shop.item_count == 1 && shop.items[0].type == ITEM_POTION_MANA &&
        shop_buy_price(&shop.items[0]) == 20 &&
        shop_accepts_item(SHOP_TYPE_WITCH, &shop.items[0]));
    g.gold = 20;
    g.player.mp -= 20;
    g.inventory[g.inventory_count++] = shop.items[0];
    g.gold -= shop_buy_price(&shop.items[0]);
    action_resolve_player(&g, (Action){ACTION_USE_ITEM, g.inventory_count - 1, 0});
    ASSERT("mana potion restores MP after purchase and use",
        g.gold == 0 && g.player.mp == g.player.max_mp &&
        g.player.hp == g.player.max_hp);

    shop_init(&shop, SHOP_TYPE_ALCHEMIST, 0);
    ASSERT("alchemist still sells and buys both potion types",
        shop.item_count >= 2 &&
        shop.items[0].type == ITEM_POTION_HEALTH &&
        shop.items[1].type == ITEM_POTION_MANA &&
        shop_buy_price(&shop.items[0]) == 20 &&
        shop_buy_price(&shop.items[1]) == 20 &&
        shop_accepts_item(SHOP_TYPE_ALCHEMIST, &shop.items[0]) &&
        shop_accepts_item(SHOP_TYPE_ALCHEMIST, &shop.items[1]));
    ASSERT("specialists accept only their own potions",
        !shop_accepts_item(SHOP_TYPE_HEALER, &shop.items[1]) &&
        !shop_accepts_item(SHOP_TYPE_WITCH, &shop.items[0]));

    g.map.tiles[13][TOWN_HEALER_DOOR_X] = TILE_TOWN_FLOOR;
    const int slot = 99012;
    if (save_exists(slot)) {
        ASSERT("potion seller test save slot must be unused", 0);
        return;
    }
    int saved = save_game(&g, slot);
    int restored = saved && load_game(&loaded, slot);
    ASSERT("potion recovery and second-town shops survive save and load",
        restored && loaded.gold == 0 &&
        loaded.location == LOCATION_TOWN2 &&
        loaded.player.hp == loaded.player.max_hp &&
        loaded.player.mp == loaded.player.max_mp &&
        loaded.map.tiles[TOWN_HEALER_DOOR_Y][TOWN_HEALER_DOOR_X] == TILE_HEALER_DOOR &&
        loaded.map.tiles[TOWN_WITCH_DOOR_Y][TOWN_WITCH_DOOR_X] == TILE_WITCH_DOOR &&
        loaded.map.tiles[13][TOWN_HEALER_DOOR_X] == TILE_TOWN_PATH);
    remove("saves/savegame_99012.json");
}

void test_rook_labyrinth(void) {
    printf("Rook labyrinth tests:\n");
    static GameState g;
    static GameState loaded;
    g.player.player_class = CLASS_MAGE;
    game_init(&g);
    ASSERT("the starting town has no labyrinth gate",
        g.map.tiles[TOWN_LABYRINTH_Y][TOWN_LABYRINTH_X] !=
            TILE_LABYRINTH_ENTRANCE);
    game_enter_town2(&g);
    ASSERT("second town center extends one row below the main road",
        g.map.tiles[11][TOWN_HEALER_DOOR_X] == TILE_TOWN_PATH &&
        g.map.tiles[12][20] == TILE_TOWN_PATH &&
        g.map.tiles[13][TOWN_HEALER_DOOR_X] == TILE_TOWN_PATH &&
        g.map.tiles[13][20] == TILE_TOWN_PATH &&
        g.map.tiles[13][TOWN_WITCH_DOOR_X] == TILE_TOWN_PATH &&
        g.map.tiles[13][TOWN_HEALER_DOOR_X - 1] == TILE_TOWN_FLOOR &&
        g.map.tiles[14][TOWN_HEALER_DOOR_X] == TILE_TOWN_FLOOR);
    ASSERT("second town places the labyrinth across from the witch",
        TOWN_LABYRINTH_X == TOWN_WITCH_DOOR_X && TOWN_LABYRINTH_Y > 12 &&
        g.map.tiles[TOWN_LABYRINTH_Y][TOWN_LABYRINTH_X] ==
            TILE_LABYRINTH_ENTRANCE &&
        g.map.tiles[TOWN_LABYRINTH_Y + 1][TOWN_LABYRINTH_X] ==
            TILE_TOWN_PATH);
    ASSERT("the old labyrinth entrance and lane return to grass",
        g.map.tiles[18][13] == TILE_TOWN_FLOOR &&
        g.map.tiles[18][14] == TILE_TOWN_FLOOR &&
        g.map.tiles[18][15] == TILE_TOWN_FLOOR);
    ASSERT("labyrinth stays sealed until Rook assigns its quest",
        !game_labyrinth_is_open(&g));

    game_enter_inn(&g);
    ASSERT("Rook is in the inn",
        g.map.tiles[18][10] == TILE_NPC_ROOK &&
        g.map.tiles[7][10] != TILE_NPC_ELOWEN);
    game_talk_to_rook(&g);
    ASSERT("Rook assigns the retrieval quest on first conversation",
        g.rook_quest_state == 1 && g.rook_labyrinth_switches == 0 &&
        game_labyrinth_is_open(&g));
    ASSERT("Rook speaks in a dialogue bubble and the status bar notes the quest",
        g.dialogue_active && strcmp(g.dialogue_speaker, "Rook") == 0 &&
        g.dialogue_x == 10 && g.dialogue_y == 18 &&
        strstr(g.dialogue_text, "ivory rook") != NULL &&
        strcmp(g.messages[g.message_count - 2], "Assigned: The Ivory Rook.") == 0);
    game_talk_to_rook(&g);
    ASSERT("Rook's reminder stays in the bubble while the bar shows a status",
        g.dialogue_active && strstr(g.dialogue_text, "One rune per floor") != NULL &&
        strcmp(g.messages[g.message_count - 1],
            "Rook is waiting for the ivory rook.") == 0);
    game_leave_inn(&g);
    g.player.x = TOWN_LABYRINTH_X - 2;
    g.player.y = 12;
    for (int y = 13; y <= TOWN_LABYRINTH_Y + 1; y++) {
        action_resolve_player(&g, (Action){ACTION_MOVE, g.player.x, y});
    }
    for (int x = TOWN_LABYRINTH_X - 1; x <= TOWN_LABYRINTH_X; x++) {
        action_resolve_player(&g, (Action){ACTION_MOVE, x, g.player.y});
    }
    ASSERT("the approach lane reaches the south-facing labyrinth entrance",
        g.location == LOCATION_TOWN2 && g.player.x == TOWN_LABYRINTH_X &&
        g.player.y == TOWN_LABYRINTH_Y + 1);
    action_resolve_player(&g, (Action){ACTION_MOVE,
        TOWN_LABYRINTH_X, TOWN_LABYRINTH_Y});
    ASSERT("labyrinth entrance starts a three-level combat expedition",
        g.location == LOCATION_LABYRINTH && g.level == 1 &&
        g.enemy_count >= 4 && g.floor_item_count == 0);

    int switches = 0;
    int stairs = 0;
    for (int y = 0; y < LABYRINTH_H; y++) {
        for (int x = 0; x < LABYRINTH_W; x++) {
            TileType tile = g.map.tiles[y][x];
            switches += tile == TILE_LABYRINTH_SWITCH_OFF;
            stairs += tile == TILE_LABYRINTH_STAIRS;
        }
    }
    ASSERT("first floor has one rune and two indistinguishable descents",
        switches == 1 && stairs == 2);
    int rune_x;
    int rune_y;
    find_labyrinth_tile(&g.map, TILE_LABYRINTH_SWITCH_OFF,
        &rune_x, &rune_y);
    g.player.x = rune_x;
    g.player.y = rune_y;
    game_interact_labyrinth(&g);
    ASSERT("first rune remains active during the expedition",
        g.rook_labyrinth_switches == 1);

    int false_x;
    int false_y;
    find_false_labyrinth_stair(&g.map, &false_x, &false_y);
    g.enemies[0].active = 0;
    step_into_labyrinth_tile(&g, false_x, false_y);
    ASSERT("false descent reaches an isolated corridor on floor two",
        g.level == 2 && g.player.x == LABYRINTH_FALSE_EXIT_X + 1 &&
        g.player.y == LABYRINTH_FALSE_EXIT_Y &&
        g.map.tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X] ==
            TILE_LABYRINTH_EXIT &&
        g.map.tiles[LABYRINTH_FALSE_EXIT_Y][LABYRINTH_FALSE_EXIT_X + 2] ==
            TILE_TRAP_REVEALED);
    action_resolve_player(&g, (Action){ACTION_MOVE,
        LABYRINTH_FALSE_EXIT_X, LABYRINTH_FALSE_EXIT_Y});
    ASSERT("false corridor returns to its original stair and keeps enemy state",
        g.level == 1 && g.player.x == false_x &&
        g.player.y == false_y && !g.enemies[0].active);

    step_into_labyrinth_tile(&g, g.map.stairs_down_x, g.map.stairs_down_y);
    ASSERT("main stair reaches the second floor without clearing enemies",
        g.level == 2 && g.player.x == 2 && g.enemy_count >= 5);
    ASSERT("labyrinth floor test save slot is unused", !save_exists(99015));
    int mid_saved = save_game(&g, 99015);
    int mid_loaded = mid_saved && load_game(&loaded, 99015);
    ASSERT("labyrinth floor, enemies, and prior floor cache survive save/load",
        mid_loaded && loaded.location == LOCATION_LABYRINTH &&
        loaded.level == 2 && loaded.enemy_count == g.enemy_count &&
        loaded.labyrinth_cache[0].valid &&
        !loaded.labyrinth_cache[0].enemies[0].active &&
        loaded.rook_labyrinth_switches == 1);
    remove("saves/savegame_99015.json");
    find_labyrinth_tile(&g.map, TILE_LABYRINTH_SWITCH_OFF,
        &rune_x, &rune_y);
    g.player.x = rune_x;
    g.player.y = rune_y;
    game_interact_labyrinth(&g);
    ASSERT("second floor records its own rune", g.rook_labyrinth_switches == 3);

    find_false_labyrinth_stair(&g.map, &false_x, &false_y);
    step_into_labyrinth_tile(&g, false_x, false_y);
    ASSERT("second false descent reaches a dead end on floor three",
        g.level == 3 && g.player.x == LABYRINTH_FALSE_EXIT_X + 1 &&
        g.player.y == LABYRINTH_FALSE_EXIT_Y);
    action_resolve_player(&g, (Action){ACTION_MOVE,
        LABYRINTH_FALSE_EXIT_X, LABYRINTH_FALSE_EXIT_Y});
    ASSERT("third-floor dead end returns to floor two", g.level == 2);

    step_into_labyrinth_tile(&g, g.map.stairs_down_x, g.map.stairs_down_y);
    ASSERT("deepest floor has a guarded relic vault",
        g.level == LABYRINTH_DEPTH && g.enemy_count >= 7 &&
        g.map.tiles[g.map.stairs_up_y][35] == TILE_LABYRINTH_GATE);
    find_labyrinth_tile(&g.map, TILE_LABYRINTH_SWITCH_OFF,
        &rune_x, &rune_y);
    g.player.x = rune_x;
    g.player.y = rune_y;
    game_interact_labyrinth(&g);
    ASSERT("all three runes open the final vault",
        g.rook_labyrinth_switches == 7 &&
        g.map.tiles[g.map.stairs_up_y][35] == TILE_LABYRINTH_FLOOR);
    g.player.x = 40;
    g.player.y = g.map.stairs_up_y;
    game_interact_labyrinth(&g);
    ASSERT("Maze Warden must fall before taking Rook's relic",
        g.rook_quest_state == 1);
    int boss_index = -1;
    for (int i = 0; i < g.enemy_count; i++) {
        if (g.enemies[i].is_boss) {
            boss_index = i;
        }
    }
    ASSERT("Maze Warden spawns as the labyrinth boss", boss_index >= 0);
    if (boss_index >= 0) {
        g.enemies[boss_index].hp = 1;
        g.player.x = 38;
        g.player.y = g.map.stairs_up_y;
        action_resolve_player(&g, (Action){ACTION_MOVE, 39,
            g.map.stairs_up_y});
    }
    ASSERT("defeating the Warden leaves normal boss drops",
        g.defeated_bosses & (1 << LOCATION_LABYRINTH) &&
        g.floor_item_count >= 2);
    g.player.x = 40;
    g.player.y = g.map.stairs_up_y;
    game_interact_labyrinth(&g);
    ASSERT("the player recovers Rook's ivory rook after the fight",
        g.rook_quest_state == 2);

    step_into_labyrinth_tile(&g, 1, g.map.stairs_up_y);
    ASSERT("third-floor exit returns to the second floor", g.level == 2);
    step_into_labyrinth_tile(&g, 1, g.map.stairs_up_y);
    ASSERT("second-floor exit returns to the first floor", g.level == 1);
    step_into_labyrinth_tile(&g, 1, g.map.stairs_up_y);
    ASSERT("the labyrinth exit returns beside its town entrance",
        g.location == LOCATION_TOWN2 &&
        g.player.x == TOWN_LABYRINTH_X &&
        g.player.y == TOWN_LABYRINTH_Y + 1 &&
        g.map.tiles[g.player.y][g.player.x] == TILE_TOWN_PATH);
    game_enter_inn(&g);
    int gold_before = g.gold;
    game_talk_to_rook(&g);
    ASSERT("returning the relic pays Rook's one-time reward",
        g.rook_quest_state == 3 &&
        g.gold == gold_before + ROOK_QUEST_REWARD &&
        g.rook_quest_completions == 1);
    ASSERT("Rook thanks the player in the bubble and the bar records the reward",
        g.dialogue_active && strstr(g.dialogue_text, "My ivory rook!") != NULL &&
        strcmp(g.messages[g.message_count - 1],
            "Completed: The Ivory Rook. 40 gold awarded.") == 0);

    const int slot = 99014;
    game_talk_to_rook(&g);
    ASSERT("Rook does not award the same quest twice",
        g.gold == gold_before + ROOK_QUEST_REWARD &&
        g.rook_quest_state == 3 && g.rook_quest_completions == 1);
    ASSERT("a finished quest keeps Rook's thanks in the bubble",
        g.dialogue_active &&
        strcmp(g.dialogue_text, "Thank you for recovering my ivory rook.") == 0 &&
        strcmp(g.messages[g.message_count - 1],
            "Rook's quest is already complete.") == 0);
    game_leave_inn(&g);
    ASSERT("labyrinth remains visibly open after Rook rewards the quest",
        game_labyrinth_is_open(&g));
    g.player.x = TOWN_LABYRINTH_X;
    g.player.y = TOWN_LABYRINTH_Y + 1;
    action_resolve_player(&g, (Action){ACTION_MOVE,
        TOWN_LABYRINTH_X, TOWN_LABYRINTH_Y});
    ASSERT("completed Rook quest still permits a new labyrinth expedition",
        g.location == LOCATION_LABYRINTH && g.enemy_count >= 4);
    step_into_labyrinth_tile(&g, 1, g.map.stairs_up_y);
    int saved = save_game(&g, slot);
    int restored = saved && load_game(&loaded, slot);
    ASSERT("Rook quest progress and the relocated town entrance survive save and load",
        restored && loaded.rook_quest_state == 3 &&
        loaded.rook_labyrinth_switches == 7 &&
        loaded.rook_quest_completions == 1 && game_labyrinth_is_open(&loaded) &&
        loaded.location == LOCATION_TOWN2 &&
        loaded.map.tiles[TOWN_LABYRINTH_Y][TOWN_LABYRINTH_X] ==
            TILE_LABYRINTH_ENTRANCE &&
        loaded.map.tiles[TOWN_LABYRINTH_Y + 1][TOWN_LABYRINTH_X] ==
            TILE_TOWN_PATH && loaded.map.tiles[18][15] == TILE_TOWN_FLOOR);
    remove("saves/savegame_99014.json");

    loaded.defeated_bosses |= 1 << LOCATION_FOREST;
    loaded.player.x = TOWN_W - 2;
    loaded.player.y = 12;
    action_resolve_player(&loaded, (Action){ACTION_MOVE, TOWN_W - 1, 12});
    ASSERT("Town 2's east gate enters an enemy-free forest road",
        loaded.location == LOCATION_FOREST_ROAD && loaded.player.x == 1 &&
        loaded.player.y == FOREST_ROAD_Y && loaded.enemy_count == 0 &&
        loaded.map.tiles[FOREST_ROAD_Y][2] == TILE_FOREST_FLOOR &&
        loaded.map.tiles[FOREST_ROAD_Y - 1][2] == TILE_FOREST_WALL &&
        loaded.map.tiles[FOREST_ROAD_Y + 1][2] == TILE_FOREST_WALL);
    saved = save_game(&loaded, slot);
    restored = saved && load_game(&g, slot);
    ASSERT("forest road position survives save and load",
        restored && g.location == LOCATION_FOREST_ROAD &&
        g.player.x == 1 && g.player.y == FOREST_ROAD_Y &&
        g.map.tiles[FOREST_ROAD_Y][FOREST_ROAD_W - 1] == TILE_FOREST_EXIT);
    remove("saves/savegame_99014.json");
    for (int x = 2; x < FOREST_ROAD_W; x++) {
        action_resolve_player(&g, (Action){ACTION_MOVE, x, FOREST_ROAD_Y});
    }
    ASSERT("walking east across the road reaches Town 1's west gate",
        g.location == LOCATION_TOWN && g.player.x == 1 &&
        g.player.y == TOWN_ROAD_EXIT_Y);
    g.map.tiles[19][0] = TILE_TOWN_EXIT;
    g.map.tiles[19][1] = TILE_TOWN_PATH;
    saved = save_game(&g, slot);
    restored = saved && load_game(&loaded, slot);
    ASSERT("loading Town 1 moves the gate above the forest and closes the old gate",
        restored && loaded.location == LOCATION_TOWN &&
        loaded.map.tiles[TOWN_ROAD_EXIT_Y][0] == TILE_TOWN_EXIT &&
        loaded.map.tiles[TOWN_ROAD_EXIT_Y][4] == TILE_TOWN_PATH &&
        loaded.map.tiles[19][0] == TILE_WALL &&
        loaded.map.tiles[19][1] == TILE_TOWN_FLOOR);
    remove("saves/savegame_99014.json");
    action_resolve_player(&g, (Action){ACTION_MOVE, 0, TOWN_ROAD_EXIT_Y});
    ASSERT("Town 1's northern west gate enters the road at its east end",
        g.location == LOCATION_FOREST_ROAD &&
        g.player.x == FOREST_ROAD_W - 2 && g.player.y == FOREST_ROAD_Y);
    for (int x = FOREST_ROAD_W - 3; x >= 0; x--) {
        action_resolve_player(&g, (Action){ACTION_MOVE, x, FOREST_ROAD_Y});
    }
    ASSERT("walking west across the road reaches Town 2's east gate",
        g.location == LOCATION_TOWN2 && g.player.x == TOWN_W - 2 &&
        g.player.y == 12);
    action_resolve_player(&g, (Action){ACTION_MOVE, TOWN_W - 3, 12});
    ASSERT("continuing west enters Town 2 rather than returning to the road",
        g.location == LOCATION_TOWN2 && g.player.x == TOWN_W - 3);
    ASSERT("Inn stands east of the Healer with its door on the square",
        TOWN_INN_X >= TOWN_HEALER_X + TOWN_HEALER_W &&
        TOWN_INN_X + TOWN_INN_W <= 20 &&
        g.map.tiles[TOWN_INN_Y][TOWN_INN_X] == TILE_TAVERN &&
        g.map.tiles[TOWN_INN_DOOR_Y][TOWN_INN_DOOR_X] == TILE_TAVERN_DOOR &&
        g.map.tiles[TOWN_INN_DOOR_Y + 1][TOWN_INN_DOOR_X] == TILE_TOWN_PATH);
    ASSERT("the Inn's former lot and lane are open green",
        g.map.tiles[16][5] == TILE_TOWN_FLOOR && g.map.tiles[20][8] == TILE_TOWN_FLOOR &&
        g.map.tiles[18][12] == TILE_TOWN_FLOOR && g.map.tiles[21][10] == TILE_TOWN_FLOOR);
    g.player.x = TOWN_INN_DOOR_X;
    g.player.y = TOWN_INN_DOOR_Y + 1;
    action_resolve_player(&g, (Action){ACTION_MOVE, TOWN_INN_DOOR_X, TOWN_INN_DOOR_Y});
    ASSERT("walking through the Inn door enters Rook's room",
        g.location == LOCATION_INN &&
        g.map.tiles[18][10] == TILE_NPC_ROOK);
    g.player.x = 20;
    g.player.y = 21;
    action_resolve_player(&g, (Action){ACTION_MOVE, 20, 22});
    ASSERT("leaving the Inn returns to Town 2",
        g.location == LOCATION_TOWN2 &&
        g.player.x == TOWN_INN_DOOR_X && g.player.y == TOWN_INN_DOOR_Y + 1);
}

static int forest_path_exists_around(const Map *m, int blocked_room) {
    unsigned char visited[MAP_H][MAP_W] = {{0}};
    int qx[MAP_W * MAP_H];
    int qy[MAP_W * MAP_H];
    int head = 0, tail = 0;
    qx[tail] = m->stairs_up_x;
    qy[tail++] = m->stairs_up_y;
    visited[m->stairs_up_y][m->stairs_up_x] = 1;
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};
    const Room *blocked = &m->rooms[blocked_room];
    while (head < tail) {
        int x = qx[head], y = qy[head++];
        if (x == m->stairs_down_x && y == m->stairs_down_y) return 1;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i], ny = y + dy[i];
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H ||
                visited[ny][nx] || !map_is_walkable(m, nx, ny)) continue;
            if (nx >= blocked->x && nx < blocked->x + blocked->w &&
                ny >= blocked->y && ny < blocked->y + blocked->h) continue;
            visited[ny][nx] = 1;
            qx[tail] = nx;
            qy[tail++] = ny;
        }
    }
    return 0;
}

static Action outdoor_exit_action(const Map *m) {
    Action action = {ACTION_MOVE, m->stairs_down_x, m->stairs_down_y};
    if (m->stairs_down_x == MAP_W - 2) {
        action.target_x++;
    } else if (m->stairs_down_y == 1) {
        action.target_y--;
    } else {
        action.target_y++;
    }
    return action;
}

static Action outdoor_entrance_action(const Map *m) {
    Action action = {ACTION_MOVE, m->stairs_up_x, m->stairs_up_y};
    if (m->stairs_up_x == 1) {
        action.target_x--;
    } else if (m->stairs_up_y == 1) {
        action.target_y--;
    } else {
        action.target_y++;
    }
    return action;
}

static int outdoor_exit_is_on_side(const Map *m, TileType exit_tile, int side) {
    if (side == 0) {
        return m->tiles[m->stairs_down_y][MAP_W - 1] == exit_tile;
    }
    if (side == 1) {
        return m->tiles[0][m->stairs_down_x] == exit_tile;
    }
    return m->tiles[MAP_H - 1][m->stairs_down_x] == exit_tile;
}

static int outdoor_entrance_is_on_side(const Map *m, TileType entrance_tile, int side) {
    if (side == 0) {
        return m->tiles[m->stairs_up_y][0] == entrance_tile;
    }
    if (side == 1) {
        return m->tiles[0][m->stairs_up_x] == entrance_tile;
    }
    return m->tiles[MAP_H - 1][m->stairs_up_x] == entrance_tile;
}

static void reveal_forest_exit(GameState *g) {
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g->map.tiles[y][x] == TILE_FOREST_LANDMARK) {
                g->player.x = x;
                g->player.y = y;
                Action discover = {ACTION_MOVE, x, y};
                action_resolve_player(g, discover);
                return;
            }
        }
    }
}

void test_town_tiles(void) {
    printf("Town tile tests:\n");

    // Tile types are distinct
    ASSERT("TILE_TOWN_FLOOR is distinct",    TILE_TOWN_FLOOR    != TILE_WALL);
    ASSERT("TILE_TOWN_PATH is distinct",     TILE_TOWN_PATH     != TILE_TOWN_FLOOR);
    ASSERT("TILE_TOWN_EXIT is distinct",     TILE_TOWN_EXIT     != TILE_TOWN_PATH);
    ASSERT("TILE_SHOP_BLACKSMITH distinct",  TILE_SHOP_BLACKSMITH != TILE_TOWN_EXIT);
    ASSERT("TILE_SHOP_ALCHEMIST distinct",   TILE_SHOP_ALCHEMIST  != TILE_SHOP_BLACKSMITH);
    ASSERT("TILE_TAVERN is distinct", TILE_TAVERN != TILE_SHOP_ALCHEMIST);
    ASSERT("TILE_WATCHTOWER is distinct", TILE_WATCHTOWER != TILE_TAVERN);

    // Constants are defined
    ASSERT("TOWN_W is 44", TOWN_W == 44);
    ASSERT("TOWN_H is 25", TOWN_H == 25);
}

void test_town_map(void) {
    printf("Town map tests:\n");

    Map m;
    int spawn_x, spawn_y;
    map_generate_town(&m, &spawn_x, &spawn_y);

    // Spawn is walkable
    ASSERT("spawn tile is walkable",
        map_is_walkable(&m, spawn_x, spawn_y));
    ASSERT("town spawn is at the central crossroads",
        spawn_x == 20 && spawn_y == 12);

    // Exit tiles at north edge
    ASSERT("exit tile at north center",
        m.tiles[0][20] == TILE_TOWN_EXIT);
    ASSERT("forest exit at west crossroad",
        m.tiles[12][0] == TILE_TOWN_EXIT);
    ASSERT("the old Tavern loop is gone before Town 2 is discovered",
        m.tiles[TOWN_ROAD_EXIT_Y][0] == TILE_WALL &&
        m.tiles[TOWN_ROAD_EXIT_Y][1] == TILE_TOWN_FLOOR &&
        m.tiles[TOWN_ROAD_EXIT_Y][4] == TILE_TOWN_FLOOR &&
        m.tiles[13][4] == TILE_TOWN_FLOOR &&
        m.tiles[21][4] == TILE_TOWN_FLOOR &&
        m.tiles[21][8] == TILE_TOWN_FLOOR);
    map_set_town2_road(&m, 1);
    ASSERT("Town 2 gate opens north of the forest on a one-tile path",
        TOWN_ROAD_EXIT_Y < 10 && TOWN_ROAD_GATE_Y < 10 &&
        m.tiles[TOWN_ROAD_EXIT_Y][0] == TILE_TOWN_EXIT &&
        m.tiles[TOWN_ROAD_EXIT_Y][1] == TILE_TOWN_PATH &&
        m.tiles[14][0] == TILE_WALL &&
        m.tiles[TOWN_ROAD_EXIT_Y][4] == TILE_TOWN_PATH &&
        m.tiles[11][4] == TILE_TOWN_PATH &&
        m.tiles[12][4] == TILE_TOWN_PATH &&
        m.tiles[19][0] == TILE_WALL &&
        m.tiles[19][1] == TILE_TOWN_FLOOR &&
        m.tiles[TOWN_ROAD_EXIT_Y - 1][0] == TILE_WALL &&
        m.tiles[TOWN_ROAD_EXIT_Y + 1][0] == TILE_WALL &&
        m.tiles[TOWN_ROAD_EXIT_Y - 1][2] == TILE_TOWN_FLOOR &&
        m.tiles[TOWN_ROAD_EXIT_Y + 1][2] == TILE_TOWN_FLOOR);
    ASSERT("opening the Town 2 spur does not rebuild the old Tavern loop",
        m.tiles[20][4] == TILE_TOWN_FLOOR &&
        m.tiles[21][4] == TILE_TOWN_FLOOR &&
        m.tiles[21][5] == TILE_TOWN_FLOOR &&
        m.tiles[21][7] == TILE_TOWN_FLOOR &&
        m.tiles[21][8] == TILE_TOWN_FLOOR &&
        m.tiles[20][5] == TILE_TOWN_FLOOR);
    map_set_town2_road(&m, 0);
    ASSERT("closing the Town 2 spur leaves open green behind",
        m.tiles[TOWN_ROAD_EXIT_Y][0] == TILE_WALL &&
        m.tiles[TOWN_ROAD_EXIT_Y][2] == TILE_TOWN_FLOOR &&
        m.tiles[TOWN_ROAD_EXIT_Y][4] == TILE_TOWN_FLOOR &&
        m.tiles[21][4] == TILE_TOWN_FLOOR);
    map_set_town2_road(&m, 1);
    ASSERT("dungeon exit at east crossroad",
        m.tiles[12][TOWN_W - 1] == TILE_TOWN_EXIT);
    ASSERT("mountain exit at north crossroad",
        m.tiles[0][20] == TILE_TOWN_EXIT);

    // Shop tiles in correct positions
    ASSERT("blacksmith remains in the starting town",
        TOWN_BLACKSMITH_X > 7 &&
        m.tiles[TOWN_BLACKSMITH_Y][TOWN_BLACKSMITH_X] == TILE_SHOP_BLACKSMITH);
    ASSERT("alchemist shifts east",
        m.tiles[TOWN_ALCHEMIST_Y][TOWN_ALCHEMIST_X] ==
            TILE_SHOP_ALCHEMIST);
    ASSERT("blacksmith has a walk-in doorway",
        m.tiles[TOWN_BLACKSMITH_Y + 3][TOWN_BLACKSMITH_X + 2] == TILE_BLACKSMITH_DOOR &&
        map_is_walkable(&m, TOWN_BLACKSMITH_X + 2, TOWN_BLACKSMITH_Y + 3));
    ASSERT("alchemist has a walk-in doorway",
        m.tiles[TOWN_ALCHEMIST_Y + 3][TOWN_ALCHEMIST_X + 2] ==
            TILE_ALCHEMIST_DOOR &&
        map_is_walkable(&m, TOWN_ALCHEMIST_X + 2,
            TOWN_ALCHEMIST_Y + 3));
    ASSERT("town square spans the Blacksmith and Alchemist",
        m.tiles[11][TOWN_BLACKSMITH_X] == TILE_TOWN_PATH &&
        m.tiles[14][TOWN_BLACKSMITH_X] == TILE_TOWN_PATH &&
        m.tiles[11][TOWN_ALCHEMIST_X + 4] == TILE_TOWN_PATH &&
        m.tiles[14][TOWN_ALCHEMIST_X + 4] == TILE_TOWN_PATH &&
        m.tiles[13][TOWN_BLACKSMITH_X - 1] == TILE_TOWN_FLOOR &&
        m.tiles[13][TOWN_ALCHEMIST_X + 5] == TILE_TOWN_FLOOR);
    ASSERT("cobblestone lanes reach the starting-town shops",
        m.tiles[TOWN_BLACKSMITH_Y + 4][TOWN_BLACKSMITH_X + 2] == TILE_TOWN_PATH &&
        m.tiles[TOWN_ALCHEMIST_Y + 4][TOWN_ALCHEMIST_X + 2] ==
            TILE_TOWN_PATH);
    ASSERT("shop facades remain solid away from their doors",
        !map_is_walkable(&m, TOWN_BLACKSMITH_X, TOWN_BLACKSMITH_Y) &&
        !map_is_walkable(&m, TOWN_ALCHEMIST_X, TOWN_ALCHEMIST_Y));
    ASSERT("tavern stands east of the blacksmith, next to the central road",
        TOWN_TAVERN_X > TOWN_BLACKSMITH_X + 4 && TOWN_TAVERN_X == 21 &&
        TOWN_TAVERN_X + TOWN_TAVERN_W <= TOWN_ALCHEMIST_X &&
        m.tiles[TOWN_TAVERN_Y][TOWN_TAVERN_X] == TILE_TAVERN &&
        m.tiles[TOWN_TAVERN_DOOR_Y][TOWN_TAVERN_DOOR_X] == TILE_TAVERN_DOOR &&
        m.tiles[TOWN_TAVERN_DOOR_Y][TOWN_TAVERN_X + TOWN_TAVERN_W - 1] == TILE_TAVERN);
    ASSERT("tavern facade remains solid away from its door",
        !map_is_walkable(&m, TOWN_TAVERN_X, TOWN_TAVERN_Y));
    ASSERT("tavern doorway is walkable",
        map_is_walkable(&m, TOWN_TAVERN_DOOR_X, TOWN_TAVERN_DOOR_Y));
    ASSERT("tavern door opens onto the town square",
        m.tiles[TOWN_TAVERN_DOOR_Y + 1][TOWN_TAVERN_DOOR_X] == TILE_TOWN_PATH);
    ASSERT("tavern's former south-west lot is open green",
        m.tiles[16][5] == TILE_TOWN_FLOOR && m.tiles[20][8] == TILE_TOWN_FLOOR);
    ASSERT("harbor reaches the southeast corner of the town green",
        m.tiles[TOWN_HARBOR_Y][TOWN_HARBOR_X] == TILE_WATCHTOWER &&
        m.tiles[TOWN_H - 2][TOWN_W - 2] == TILE_WATCHTOWER);
    ASSERT("harbor remains closed and solid",
        !map_is_walkable(&m, TOWN_HARBOR_X, TOWN_HARBOR_Y) &&
        !map_is_walkable(&m, TOWN_HARBOR_ENTRANCE_X,
            TOWN_HARBOR_ENTRANCE_Y));
    ASSERT("former watchtower lot is walkable green",
        m.tiles[16][28] == TILE_TOWN_FLOOR && map_is_walkable(&m, 32, 21));

    // Path tiles exist
    ASSERT("vertical path at center",
        m.tiles[10][20] == TILE_TOWN_PATH);
    ASSERT("horizontal path at mid row",
        m.tiles[12][10] == TILE_TOWN_PATH);

    ASSERT("coast exit at south crossroad",
        m.tiles[TOWN_H - 1][20] == TILE_TOWN_EXIT);
}

static void test_necromancer_retaliates_to_arrows(void) {
    static GameState g;
    Item bows[] = {item_make_bow(), item_make_longbow(), item_make_magic_longbow()};
    for (int i = 0; i < 3; i++) {
        g.player.player_class = CLASS_ROGUE;
        game_init(&g);
        g.location = LOCATION_FOREST;
        g.level = FOREST_DEPTH;
        map_generate_forest(&g.map, g.level);
        enemies_spawn(&g);
        ASSERT("Necromancer is available for ranged encounter",
            g.enemy_count > 0 && g.enemies[0].type == ENEMY_FOREST_NECROMANCER);
        if (g.enemy_count == 0 || g.enemies[0].type != ENEMY_FOREST_NECROMANCER) {
            return;
        }
        g.enemy_count = 1;
        Room *grove = &g.map.rooms[g.map.room_count - 1];
        Enemy *boss = &g.enemies[0];
        boss->x = grove->x + 4;
        boss->y = grove->y + grove->h / 2;
        g.player.x = boss->x - bows[i].range;
        g.player.y = boss->y;
        g.player.last_dx = 1;
        g.player.last_dy = 0;
        g.player.hp = 300;
        g.player.max_hp = 300;
        g.inventory_count = 1;
        g.inventory[0] = bows[i];
        g.equipped_main_hand = 0;
        for (int x = g.player.x; x <= boss->x; x++) {
            g.map.tiles[g.player.y][x] = TILE_FOREST_FLOOR;
        }
        action_resolve_enemies(&g);
        ASSERT("unprovoked Necromancer remains dormant outside the grove",
            boss->move_timer == 0 && g.player.hp == 300);
        action_resolve_player(&g, (Action){ACTION_RANGED_ATTACK, 0, 0});
        ASSERT("each bow can hit the Necromancer from outside the grove",
            boss->hp < boss->max_hp);
        action_resolve_enemies(&g);
        ASSERT("a ranged hit triggers the Necromancer's warning turn",
            boss->move_timer == 1 && g.player.hp == 300 &&
            strstr(g.messages[g.message_count - 1], "invokes the forest"));
        action_resolve_enemies(&g);
        ASSERT("Necromancer retaliates at every bow's maximum range",
            boss->move_timer == 2 && g.player.hp < 300);
        int hp = g.player.hp;
        g.player.x = boss->x - 13;
        action_resolve_enemies(&g);
        ASSERT("Necromancer does not attack a distant retreating player",
            boss->move_timer == 2 && g.player.hp == hp);
        g.player.x = boss->x - bows[i].range;
        action_resolve_enemies(&g);
        action_resolve_enemies(&g);
        ASSERT("Necromancer resumes fighting when the player returns",
            boss->move_timer == 4 && g.player.hp < hp);
    }
}

static void test_necromancer_opens_exit(void) {
    static GameState g;
    Action attacks[] = {
        {ACTION_MOVE, 12, 10},
        {ACTION_RANGED_ATTACK, 0, 0},
        {ACTION_CAST_SPELL, 0, 0}
    };
    for (int i = 0; i < 3; i++) {
        g.player.player_class = CLASS_WARRIOR;
        game_init(&g);
        g.location = LOCATION_FOREST;
        g.level = FOREST_DEPTH;
        map_generate_forest(&g.map, g.level);
        g.enemy_count = 2;
        g.enemies[0] = (Enemy){
            .x = 12, .y = 10, .active = 1, .hp = 1, .max_hp = 1,
            .type = ENEMY_FOREST_NECROMANCER, .is_boss = 1
        };
        g.enemies[1] = (Enemy){
            .x = 20, .y = 20, .active = 1, .hp = 10,
            .type = ENEMY_DARK_ELF
        };
        g.player.x = i == 0 ? 11 : 10;
        g.player.y = 10;
        g.player.last_dx = 1;
        g.player.last_dy = 0;
        g.inventory_count = 1;
        g.inventory[0] = i == 0 ? item_make_dagger() : item_make_bow();
        g.equipped_main_hand = 0;
        g.player.known_spell_count = 1;
        g.player.known_spells[0] = spell_make_magic_arrow();
        g.player.equipped_spell = 0;
        g.player.mp = 100;
        for (int x = 10; x <= 12; x++) {
            g.map.tiles[10][x] = TILE_FOREST_FLOOR;
        }
        Action exit = outdoor_exit_action(&g.map);
        ASSERT("final forest exit starts hidden without finding the landmark",
            g.map.tiles[exit.target_y][exit.target_x] == TILE_FOREST_WALL);
        action_resolve_player(&g, attacks[i]);
        ASSERT("melee, arrows and spells record the Necromancer defeat",
            !g.enemies[0].active && (g.defeated_bosses & (1 << LOCATION_FOREST)));
        ASSERT("boss defeat reveals the exit without clearing regular enemies",
            g.map.tiles[exit.target_y][exit.target_x] == TILE_FOREST_EXIT &&
            g.enemies[1].active && !g.level_cleared);
        g.player.x = g.map.stairs_down_x;
        g.player.y = g.map.stairs_down_y;
        action_resolve_player(&g, exit);
        ASSERT("player leaves the final forest with a living enemy behind",
            g.location == LOCATION_TOWN2 &&
            g.forest_cache[FOREST_DEPTH - 1].enemies[1].active &&
            !g.forest_cache[FOREST_DEPTH - 1].level_cleared);

        g.location = LOCATION_FOREST;
        g.level = FOREST_DEPTH - 1;
        g.forest_cache[FOREST_DEPTH - 1].valid = 0;
        game_descend(&g);
        exit = outdoor_exit_action(&g.map);
        ASSERT("regenerated final forest keeps the defeated boss's exit open",
            g.map.tiles[exit.target_y][exit.target_x] == TILE_FOREST_EXIT &&
            g.enemy_count > 0 && !g.level_cleared);
    }
}

void test_forest(void) {
    printf("Forest adventure tests:\n");
    test_necromancer_retaliates_to_arrows();
    test_necromancer_opens_exit();
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);

    g.player.x = 1;
    g.player.y = 12;
    Action west = {ACTION_MOVE, 0, 12};
    action_resolve_player(&g, west);
    ASSERT("west town exit enters forest", g.location == LOCATION_FOREST);
    ASSERT("forest begins on level one", g.level == 1);
    ASSERT("forest begins just inside west edge", g.player.x == 1);
    ASSERT("forest has west entrance",
        g.map.tiles[g.map.stairs_up_y][0] == TILE_FOREST_ENTRANCE);
    ASSERT("forest exit begins hidden",
        g.map.tiles[g.map.stairs_down_y][MAP_W - 1] == TILE_FOREST_WALL);
    ASSERT("forest generation includes branching clearings",
        g.map.room_count == 7);
    ASSERT("lower route reaches exit when upper route is blocked",
        forest_path_exists_around(&g.map, 3));
    ASSERT("upper route reaches exit when lower route is blocked",
        forest_path_exists_around(&g.map, 4));
    int forest_terrain = 0;
    for (int y = 0; y < MAP_H && !forest_terrain; y++)
        for (int x = 0; x < MAP_W; x++)
            if (g.map.tiles[y][x] == TILE_FOREST_FLOOR) {
                forest_terrain = 1;
                break;
            }
    ASSERT("forest contains forest floor tiles", forest_terrain);
    int hidden_trails = 0;
    int hidden_x = -1;
    int hidden_y = -1;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL) {
                hidden_trails++;
                hidden_x = x;
                hidden_y = y;
            }
        }
    }
    ASSERT("forest contains a concealed shortcut", hidden_trails > 0);
    ASSERT("concealed shortcut initially behaves like dense woods",
        hidden_x >= 0 && !map_is_walkable(&g.map, hidden_x, hidden_y));
    reveal_forest_exit(&g);
    ASSERT("forest landmark reveals east stage exit",
        g.map.tiles[g.map.stairs_down_y][MAP_W - 1] == TILE_FOREST_EXIT);
    int concealed_after_landmark = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            concealed_after_landmark +=
                g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL;
        }
    }
    ASSERT("forest landmark reveals the concealed shortcut",
        hidden_x >= 0 && hidden_y >= 0 && concealed_after_landmark == 0 &&
        g.map.tiles[hidden_y][hidden_x] == TILE_FOREST_FLOOR);

    int pickup_x = g.player.x;
    int pickup_y = g.player.y;
    g.floor_item_count = 1;
    g.floor_items[0] = (FloorItem){
        .active = 1,
        .x = pickup_x,
        .y = pickup_y,
        .underlying_tile = TILE_FOREST_FLOOR,
        .item = item_make_health_potion()
    };
    g.map.tiles[pickup_y][pickup_x] = TILE_ITEM;
    Action pickup = {ACTION_PICK_UP, 0, 0};
    action_resolve_player(&g, pickup);
    ASSERT("forest pickup restores forest floor",
        g.map.tiles[pickup_y][pickup_x] == TILE_FOREST_FLOOR);

    g.player.x = MAP_W - 2;
    g.player.y = g.map.stairs_down_y;
    Action east = {ACTION_MOVE, MAP_W - 1, g.player.y};
    action_resolve_player(&g, east);
    ASSERT("forest exit advances without clearing enemies", g.level == 2);
    ASSERT("next forest stage starts at south edge",
        g.player.y == MAP_H - 2);
    Action back = outdoor_entrance_action(&g.map);
    action_resolve_player(&g, back);
    ASSERT("forest entrance returns to previous stage", g.level == 1);
    ASSERT("backtracking arrives inside east edge", g.player.x == MAP_W - 2);

    for (int level = 1; level <= FOREST_DEPTH; level++) {
        g.location = LOCATION_FOREST;
        g.level = level;
        map_generate_forest(&g.map, level);
        enemies_spawn(&g);
        int bosses = 0;
        int invalid_enemy = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            EnemyType type = g.enemies[i].type;
            if (g.enemies[i].is_boss) bosses++;
            if (type < ENEMY_PIXIE || type > ENEMY_FOREST_NECROMANCER)
                invalid_enemy = 1;
        }
        ASSERT("forest floors use only forest roster", !invalid_enemy);
        if (level < FOREST_DEPTH)
            ASSERT("forest stages 1-7 have no boss", bosses == 0);
        else
            ASSERT("forest stage 8 has Necromancer boss",
                bosses == 1 && g.enemies[0].type == ENEMY_FOREST_NECROMANCER);
    }

    static const int expected_rooms[FOREST_DEPTH] = {7, 9, 9, 8, 10, 10, 10, 10};
    static const int expected_entrances[FOREST_DEPTH] = {0, 2, 1, 2, 0, 1, 2, 0};
    static const int expected_exits[FOREST_DEPTH] = {0, 1, 2, 1, 0, 2, 1, 0};
    for (int level = 1; level <= FOREST_DEPTH; level++) {
        map_generate_forest(&g.map, level);
        ASSERT("forest level uses its distinct topology size",
            g.map.room_count == expected_rooms[level - 1]);
        ASSERT("forest levels vary their entrance edge",
            outdoor_entrance_is_on_side(&g.map, TILE_FOREST_ENTRANCE,
                expected_entrances[level - 1]));
        reveal_forest_exit(&g);
        ASSERT("forest levels vary their exit edge",
            outdoor_exit_is_on_side(&g.map, TILE_FOREST_EXIT,
                expected_exits[level - 1]));
    }
    map_generate_forest(&g.map, 2);
    g.location = LOCATION_FOREST;
    g.enemy_count = 0;
    int false_marker_x = -1;
    int false_marker_y = -1;
    int hidden_before_false_marker = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            if (g.map.tiles[y][x] == TILE_FOREST_FALSE_MARKER) {
                false_marker_x = x;
                false_marker_y = y;
            } else if (g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL) {
                hidden_before_false_marker++;
            }
        }
    }
    ASSERT("later forest stages contain a misleading trail marker",
        false_marker_x >= 0 && false_marker_y >= 0);
    if (false_marker_x < 0 || false_marker_y < 0) {
        return;
    }
    g.player.x = false_marker_x;
    g.player.y = false_marker_y;
    Action inspect_false_marker = {
        ACTION_MOVE, false_marker_x, false_marker_y
    };
    action_resolve_player(&g, inspect_false_marker);
    int hidden_after_false_marker = 0;
    int revealed_exit = 0;
    for (int y = 0; y < MAP_H; y++) {
        for (int x = 0; x < MAP_W; x++) {
            hidden_after_false_marker +=
                g.map.tiles[y][x] == TILE_FOREST_HIDDEN_TRAIL;
            revealed_exit += g.map.tiles[y][x] == TILE_FOREST_EXIT;
        }
    }
    ASSERT("misleading marker becomes ordinary forest floor",
        g.map.tiles[false_marker_y][false_marker_x] == TILE_FOREST_FLOOR);
    ASSERT("misleading marker reveals neither shortcut nor exit",
        hidden_after_false_marker == hidden_before_false_marker &&
        revealed_exit == 0);

    int l2y3, l2y4, l2y5, unused;
    map_room_center(&g.map.rooms[3], &unused, &l2y3);
    map_room_center(&g.map.rooms[4], &unused, &l2y4);
    map_room_center(&g.map.rooms[5], &unused, &l2y5);
    ASSERT("forest level 2 has three vertically distinct routes",
        l2y3 < l2y4 && l2y4 < l2y5);
    map_generate_forest(&g.map, 4);
    int hub_y, upper_y, lower_y;
    map_room_center(&g.map.rooms[1], &unused, &hub_y);
    map_room_center(&g.map.rooms[2], &unused, &upper_y);
    map_room_center(&g.map.rooms[4], &unused, &lower_y);
    ASSERT("forest level 4 uses a central branching hub",
        upper_y < hub_y && hub_y < lower_y);

    g.location = LOCATION_FOREST;
    g.level = 1;
    map_generate_forest(&g.map, 1);
    enemies_spawn(&g);
    g.level_cleared = 1;
    game_descend(&g);
    ASSERT("forest progress uses forest cache", g.forest_cache[0].valid);
    ASSERT("forest progress does not overwrite dungeon cache",
        !g.level_cache[0].valid);

    int portal_x = g.player.x;
    int portal_y = g.player.y;
    game_open_town_portal(&g);
    ASSERT("return spell works from forest", g.location == LOCATION_TOWN);
    ASSERT("portal remembers forest location",
        g.portal_location == LOCATION_FOREST);
    ASSERT("forest portal appears beside the forest entrance",
        g.map.tiles[13][2] == TILE_PORTAL);
    Action use_portal = {ACTION_MOVE, 2, 13};
    action_resolve_player(&g, use_portal);
    ASSERT("town portal returns to forest", g.location == LOCATION_FOREST);
    ASSERT("forest portal restores exact tile",
        g.player.x == portal_x && g.player.y == portal_y);

    g.level = FOREST_DEPTH;
    g.level_cleared = 0;
    map_generate_forest(&g.map, g.level);
    enemies_spawn(&g);
    reveal_forest_exit(&g);
    g.player.x = MAP_W - 2;
    g.player.y = g.map.stairs_down_y;
    east = (Action){ACTION_MOVE, MAP_W - 1, g.player.y};
    action_resolve_player(&g, east);
    ASSERT("living Necromancer blocks final forest exit",
        g.location == LOCATION_FOREST);
    for (int i = 0; i < g.enemy_count; i++)
        if (g.enemies[i].type == ENEMY_FOREST_NECROMANCER)
            g.enemies[i].active = 0;
    action_resolve_player(&g, east);
    ASSERT("final east forest exit reaches the second town",
        g.location == LOCATION_TOWN2);
    ASSERT("forest completion arrives at the west road",
        g.player.x == 1 && g.player.y == 12);
}

void test_cain_gift(void) {
    printf("Cain gift tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    ASSERT("Cain stands beside the crossroads within talking range of spawn",
        g.map.tiles[TOWN_CAIN_Y][TOWN_CAIN_X] == TILE_NPC_CAIN &&
        abs(g.player.x - TOWN_CAIN_X) <= 1 &&
        abs(g.player.y - TOWN_CAIN_Y) <= 1);
    ASSERT("Cain is solid and leaves both roads open",
        !map_is_walkable(&g.map, TOWN_CAIN_X, TOWN_CAIN_Y) &&
        g.map.tiles[12][19] == TILE_TOWN_PATH &&
        g.map.tiles[11][20] == TILE_TOWN_PATH);
    ASSERT("Cain's gift starts unclaimed", !g.cain_scroll_given);

    int count = g.inventory_count;
    game_talk_to_cain(&g);
    ASSERT("Cain warns about dangers outside town in his dialogue bubble",
        g.dialogue_active && strcmp(g.dialogue_speaker, "Cain") == 0 &&
        strstr(g.dialogue_text, "Goblins lurk north") &&
        strstr(g.dialogue_text, "undead east") &&
        g.dialogue_x == TOWN_CAIN_X && g.dialogue_y == TOWN_CAIN_Y);
    ASSERT("Cain gives one Return to Town scroll",
        g.cain_scroll_given && g.inventory_count == count + 1 &&
        g.inventory[count].type == ITEM_SCROLL &&
        g.inventory[count].spell_id == SPELL_RETURN_TO_TOWN);
    game_talk_to_cain(&g);
    ASSERT("talking again does not duplicate the gift",
        g.inventory_count == count + 1);
    Action read = {ACTION_USE_ITEM, count, 0};
    action_resolve_player(&g, read);
    ASSERT("Cain's scroll teaches Return to Town",
        g.inventory_count == count && g.player.known_spell_count == 1 &&
        g.player.known_spells[0].id == SPELL_RETURN_TO_TOWN);
    game_enter_tavern(&g);
    game_leave_tavern(&g);
    game_talk_to_cain(&g);
    ASSERT("returning to town after reading the scroll does not renew the gift",
        g.inventory_count == count && g.cain_scroll_given &&
        g.map.tiles[TOWN_CAIN_Y][TOWN_CAIN_X] == TILE_NPC_CAIN);

    game_init(&g);
    while (g.inventory_count < MAX_INVENTORY) {
        g.inventory[g.inventory_count++] = item_make_health_potion();
    }
    game_talk_to_cain(&g);
    ASSERT("a full pack leaves the gift available for later",
        g.inventory_count == MAX_INVENTORY && !g.cain_scroll_given &&
        strstr(g.dialogue_text, "Make room"));
    game_remove_inventory_item(&g, MAX_INVENTORY - 1);
    game_talk_to_cain(&g);
    ASSERT("Cain gives the scroll after the player makes room",
        g.inventory_count == MAX_INVENTORY && g.cain_scroll_given &&
        g.inventory[MAX_INVENTORY - 1].spell_id == SPELL_RETURN_TO_TOWN);
}

void test_harbor_road(void) {
    printf("Harbor road tests:\n");
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    int road_y = TOWN_HARBOR_Y + 1;
    ASSERT("new games have no harbor road",
        !game_harbor_unlocked(&g) && g.map.tiles[road_y][21] == TILE_TOWN_FLOOR);
    int inventory_count = g.inventory_count;
    game_talk_to_rowan(&g);
    ASSERT("Rowan initially gives neither an item nor a quest",
        g.inventory_count == inventory_count && !game_has_treasure_map(&g) &&
        g.elowen_quest_state == 0 && g.dain_quest_state == 0 &&
        g.alder_quest_state == 0 && g.mara_quest_state == 0);
    ASSERT("Rowan explains that the Drowned Queen blocks sailing",
        strstr(g.dialogue_text, "Drowned Queen") != NULL);
    g.defeated_bosses = (1 << LOCATION_DUNGEON) |
        (1 << LOCATION_FOREST) | (1 << LOCATION_MOUNTAINS);
    game_enter_tavern(&g);
    game_leave_tavern(&g);
    ASSERT("other bosses do not open the harbor without the Drowned Queen",
        !game_harbor_unlocked(&g) &&
        g.map.tiles[road_y][21] == TILE_TOWN_FLOOR);
    game_talk_to_rowan(&g);
    ASSERT("Rowan withholds the map until the coast boss is defeated",
        !game_has_treasure_map(&g));
    g.defeated_bosses = 1 << LOCATION_COAST;
    game_enter_tavern(&g);
    game_leave_tavern(&g);
    ASSERT("coast victory opens the harbor without other bosses or quests",
        game_harbor_unlocked(&g) &&
        g.elowen_quest_state == 0 && g.dain_quest_state == 0 &&
        g.alder_quest_state == 0 && g.mara_quest_state == 0);
    for (int x = 20; x < TOWN_HARBOR_X; x++) {
        ASSERT("coast victory connects the south road to the harbor",
            g.map.tiles[road_y][x] == TILE_TOWN_PATH &&
            map_is_walkable(&g.map, x, road_y));
    }
    ASSERT("harbor road bends through a landing onto the dock",
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_X - 1] == TILE_TOWN_PATH &&
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_X] == TILE_TOWN_PATH &&
        g.map.tiles[TOWN_HARBOR_ENTRANCE_Y][TOWN_HARBOR_ENTRANCE_X] == TILE_TOWN_PATH);
    g.player.x = TOWN_HARBOR_X - 1;
    g.player.y = road_y;
    game_move_player(&g, 0, 1);
    game_move_player(&g, 1, 0);
    game_move_player(&g, 1, 0);
    game_move_player(&g, 1, 0);
    ASSERT("player can walk from the road into the harbor dock",
        g.player.x == TOWN_HARBOR_ENTRANCE_X &&
        g.player.y == TOWN_HARBOR_ENTRANCE_Y);
    HarborScreen harbor;
    harbor_init(&harbor);
    ASSERT("dock scene blocks boarding without Rowan's treasure map",
        harbor.selected == 0 &&
        harbor_handle_key(&harbor, SDL_SCANCODE_RETURN, 0, 0) == HARBOR_MAP_REQUIRED);
    harbor_handle_key(&harbor, SDL_SCANCODE_DOWN, 0, 0);
    ASSERT("dock scene can return to town by keyboard",
        harbor.selected == 1 &&
        harbor_handle_key(&harbor, SDL_SCANCODE_KP_ENTER, 0, 0) == HARBOR_CLOSED &&
        harbor_handle_key(&harbor, SDL_SCANCODE_ESCAPE, 0, 0) == HARBOR_CLOSED);
    ASSERT("road leaves Rowan beside the route",
        g.map.tiles[TOWN_ROWAN_Y][TOWN_ROWAN_X] == TILE_NPC_ROWAN);
    while (g.inventory_count < MAX_INVENTORY) {
        g.inventory[g.inventory_count++] = item_make_health_potion();
    }
    game_talk_to_rowan(&g);
    ASSERT("a full pack leaves the map available for later",
        !game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        strstr(g.dialogue_text, "Make room"));
    game_remove_inventory_item(&g, MAX_INVENTORY - 1);
    game_talk_to_rowan(&g);
    ASSERT("Rowan gives the treasure map when space is available",
        game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        g.inventory[MAX_INVENTORY - 1].type == ITEM_TREASURE_MAP);
    ASSERT("Rowan recommends training before the temple without blocking sailing",
        strstr(g.dialogue_text, "dungeon or mountains") != NULL);
    harbor_init(&harbor);
    ASSERT("treasure map enables the dock's boarding option",
        harbor_handle_key(&harbor, SDL_SCANCODE_RETURN,
            game_has_treasure_map(&g), 0) == HARBOR_BOARD);
    Action read_map = {ACTION_USE_ITEM, MAX_INVENTORY - 1, 0};
    action_resolve_player(&g, read_map);
    ASSERT("reading the map describes the island without consuming it",
        game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        strstr(g.messages[g.message_count - 1], "ruined temple"));
    Action drop_map = {ACTION_DROP_ITEM, MAX_INVENTORY - 1, 0};
    action_resolve_player(&g, drop_map);
    ASSERT("the map cannot be dropped and lost",
        game_has_treasure_map(&g) && g.inventory_count == MAX_INVENTORY &&
        g.floor_item_count == 0);
    game_remove_inventory_item(&g, MAX_INVENTORY - 2);
    g.defeated_bosses |= 1 << LOCATION_DUNGEON;
    game_talk_to_rowan(&g);
    ASSERT("talking again with room in the pack does not duplicate the gift",
        g.inventory_count == MAX_INVENTORY - 1 && game_has_treasure_map(&g));
    ASSERT("Rowan recognizes dungeon experience",
        strstr(g.dialogue_text, "dungeon or mountains") == NULL);
    int before_voyage = g.inventory_count;
    game_enter_island(&g);
    ASSERT("first voyage consumes the map and permanently unlocks island travel",
        g.island_travel_unlocked && !game_has_treasure_map(&g) &&
        g.inventory_count == before_voyage - 1);
    game_leave_island(&g);
    game_talk_to_rowan(&g);
    ASSERT("Rowan does not replace a map after the route is unlocked",
        g.inventory_count == before_voyage - 1 &&
        !game_has_treasure_map(&g) &&
        strstr(g.dialogue_text, "route is charted") != NULL);
    harbor_init(&harbor);
    ASSERT("later voyages need no inventory map",
        harbor_handle_key(&harbor, SDL_SCANCODE_RETURN,
            game_can_sail_to_island(&g), 0) == HARBOR_BOARD);

    game_enter_coast(&g);
    game_open_town_portal(&g);
    ASSERT("returning from an expedition keeps the road and Coast portal",
        g.map.tiles[road_y][TOWN_HARBOR_X - 1] == TILE_TOWN_PATH &&
        g.map.tiles[TOWN_H - 3][21] == TILE_PORTAL);
}

static void test_oakhaven_portal_save(void) {
    static GameState g;
    static GameState loaded;
    const int slot = 99030;
    ASSERT("OakHaven test save slot is unused", !save_exists(slot));
    if (save_exists(slot)) {
        return;
    }
    Location regions[] = {LOCATION_DUNGEON, LOCATION_MOUNTAINS};
    for (int i = 0; i < 2; i++) {
        g.player.player_class = CLASS_WARRIOR;
        game_init(&g);
        if (regions[i] == LOCATION_DUNGEON) {
            game_enter_dungeon(&g);
        } else {
            game_enter_mountains(&g);
        }
        int origin_x = g.player.x;
        int origin_y = g.player.y;
        game_open_town_portal(&g);
        int x = regions[i] == LOCATION_DUNGEON ? TOWN_W - 3 : 21;
        int y = regions[i] == LOCATION_DUNGEON ? 13 : 2;
        int restored = save_game(&g, slot) && load_game(&loaded, slot);
        ASSERT("OakHaven portal and destination survive save/load",
            restored && loaded.location == LOCATION_TOWN &&
            loaded.map.tiles[y][x] == TILE_PORTAL &&
            loaded.portal_location == regions[i]);
        if (restored) {
            action_resolve_player(&loaded, (Action){ACTION_MOVE, x, y});
            ASSERT("saved OakHaven portal returns to its original region",
                loaded.location == regions[i] && !loaded.portal_active &&
                loaded.player.x == origin_x && loaded.player.y == origin_y);
        }
        remove("saves/savegame_99030.json");
    }
}

void test_town_spawn(void) {
    test_oakhaven_portal_save();
    printf("Town spawn tests:\n");

    GameState g;
    game_init(&g);

    ASSERT("new game starts in town",     g.location == LOCATION_TOWN);
    ASSERT("player spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("player not on exit tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_TOWN_EXIT);
    ASSERT("player not on shop tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_SHOP_BLACKSMITH &&
        g.map.tiles[g.player.y][g.player.x] != TILE_SHOP_ALCHEMIST);
    g.player.x = 1;
    g.player.y = TOWN_ROAD_EXIT_Y;
    action_resolve_player(&g, (Action){ACTION_MOVE, 0, TOWN_ROAD_EXIT_Y});
    ASSERT("Town 2 road gate stays blocked before the forest boss falls",
        g.location == LOCATION_TOWN && g.player.x == 1 &&
        g.player.y == TOWN_ROAD_EXIT_Y);
}

static void test_goblin_king_retaliation(void) {
    static GameState g;
    Item bows[] = {item_make_bow(), item_make_longbow(), item_make_magic_longbow()};
    for (int scenario = 0; scenario < 5; scenario++) {
        g.player.player_class = CLASS_ROGUE;
        game_init(&g);
        g.location = LOCATION_MOUNTAINS;
        g.level = MOUNTAIN_DEPTH;
        map_generate_mountains(&g.map, g.level);
        enemies_spawn(&g);
        ASSERT("Goblin King is available for retaliation test",
            g.enemy_count > 0 && g.enemies[0].type == ENEMY_MOUNTAIN_GOBLIN_KING);
        if (g.enemy_count == 0 || g.enemies[0].type != ENEMY_MOUNTAIN_GOBLIN_KING) {
            return;
        }
        g.enemy_count = 1;
        Enemy *boss = &g.enemies[0];
        // A fixed arena keeps every shot outside its boundary and inside map bounds.
        g.map.rooms[g.map.room_count - 1] = (Room){30, 30, 12, 12};
        boss->x = 32;
        boss->y = 35;
        g.inventory_count = 1;
        g.inventory[0] = bows[scenario < 3 ? scenario : 2];
        g.equipped_main_hand = 0;
        g.player.known_spell_count = 1;
        g.player.known_spells[0] = spell_make_magic_arrow();
        g.player.equipped_spell = 0;
        g.player.mp = 100;
        g.player.hp = 300;
        g.player.max_hp = 300;
        int range = scenario == 4 ? g.player.known_spells[0].range : g.inventory[0].range;
        int diagonal = scenario == 3;
        g.player.x = boss->x - range;
        g.player.y = boss->y - (diagonal ? range : 0);
        g.player.last_dx = 1;
        g.player.last_dy = diagonal;
        int start_x = g.player.x;
        int start_y = g.player.y;
        for (int step = 0; step <= range; step++) {
            int path_x = start_x + step;
            int path_y = start_y + step * diagonal;
            g.map.tiles[path_y][path_x] = TILE_MOUNTAIN_FLOOR;
            if (diagonal && step > 0) {
                g.map.tiles[path_y - 1][path_x] = TILE_MOUNTAIN_FLOOR;
                g.map.tiles[path_y][path_x - 1] = TILE_MOUNTAIN_FLOOR;
            }
        }
        EnemyProjectiles shots = {0};
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("unprovoked Goblin King waits inside his fortress",
            boss->move_timer == 0 && shots.count == 0 && g.player.hp == 300);
        Action attack = {scenario == 4 ? ACTION_CAST_SPELL : ACTION_RANGED_ATTACK, 0, 0};
        action_resolve_player(&g, attack);
        ASSERT("bows and magic can provoke the Goblin King outside his fortress",
            boss->hp < boss->max_hp && boss->active);
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("a ranged hit triggers the Goblin King's axe warning",
            boss->move_timer == 1 && shots.count == 0 &&
            strstr(g.messages[g.message_count - 1], "raises his axe"));
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("Goblin King retaliates with damage and a visible axe projectile",
            boss->move_timer == 2 && g.player.hp < 300 && shots.count == 1 &&
            shots.shots[0].type == ENEMY_MOUNTAIN_GOBLIN_KING &&
            shots.shots[0].target_x == start_x && shots.shots[0].target_y == start_y);
        int hp = g.player.hp;
        g.player.x = boss->x - 13;
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("Goblin King stops attacking a distant retreating player",
            boss->move_timer == 2 && g.player.hp == hp && shots.count == 0);
        g.player.x = start_x;
        action_resolve_enemies_with_projectiles(&g, &shots);
        action_resolve_enemies_with_projectiles(&g, &shots);
        ASSERT("Goblin King resumes retaliation when the player returns",
            boss->move_timer == 4 && g.player.hp < hp && shots.count == 1);
    }
}

void test_mountains(void) {
    test_goblin_king_retaliation();
    printf("Goblin Mountains tests:\n");
    static const int expected_rooms[MOUNTAIN_DEPTH] = {7, 8, 8, 9, 9, 10, 10, 10};
    static const int expected_entrances[MOUNTAIN_DEPTH] = {0, 2, 1, 2, 0, 1, 2, 0};
    static const int expected_exits[MOUNTAIN_DEPTH] = {1, 0, 2, 1, 2, 0, 1, 2};
    GameState g;
    g.player.player_class = CLASS_WARRIOR;
    game_init(&g);
    g.player.x = 20;
    g.player.y = 1;
    Action enter = {ACTION_MOVE, 20, 0};
    action_resolve_player(&g, enter);
    ASSERT("north OakHaven exit enters mountains", g.location == LOCATION_MOUNTAINS);
    ASSERT("mountains begin on level one", g.level == 1);
    ASSERT("mountains use red-black terrain",
        g.map.tiles[g.player.y][g.player.x] == TILE_MOUNTAIN_FLOOR);

    int origin_x = g.player.x;
    int origin_y = g.player.y;
    game_open_town_portal(&g);
    ASSERT("mountain portal opens beside north OakHaven entrance",
        g.location == LOCATION_TOWN && g.portal_active &&
        g.map.tiles[2][21] == TILE_PORTAL &&
        g.player.x == 20 && g.player.y == 1);
    action_resolve_player(&g, (Action){ACTION_MOVE, 21, 2});
    ASSERT("north portal restores the mountain position and closes",
        g.location == LOCATION_MOUNTAINS && !g.portal_active &&
        g.player.x == origin_x && g.player.y == origin_y);

    for (int level = 1; level <= MOUNTAIN_DEPTH; level++) {
        g.level = level;
        map_generate_mountains(&g.map, level);
        ASSERT("mountain stage uses its authored template size",
            g.map.room_count == expected_rooms[level - 1]);
        ASSERT("mountain levels vary their entrance edge",
            outdoor_entrance_is_on_side(&g.map, TILE_MOUNTAIN_ENTRANCE,
                expected_entrances[level - 1]));
        ASSERT("mountain levels vary their exit edge",
            outdoor_exit_is_on_side(&g.map, TILE_MOUNTAIN_EXIT,
                expected_exits[level - 1]));
        int bridges = 0;
        int caves = 0;
        int fortress = 0;
        for (int y = 0; y < MAP_H; y++) {
            for (int x = 0; x < MAP_W; x++) {
                bridges += g.map.tiles[y][x] == TILE_MOUNTAIN_BRIDGE;
                caves += g.map.tiles[y][x] == TILE_MOUNTAIN_CAVE_FLOOR;
                fortress +=
                    g.map.tiles[y][x] == TILE_MOUNTAIN_FORTRESS_FLOOR;
            }
        }
        if (level == 2 || level == 7) {
            ASSERT("bridge template contains timber crossings", bridges > 0);
        } else if (level == 3 || level == 6) {
            ASSERT("cave template contains underground terrain", caves > 0);
        } else if (level == 4 || level == 5 || level == 8) {
            ASSERT("fortress template contains paved strongholds",
                fortress > 0);
        } else {
            ASSERT("pass template remains exposed basalt",
                bridges == 0 && caves == 0 && fortress == 0);
        }
        enemies_spawn(&g);
        int bosses = 0, invalid = 0;
        for (int i = 0; i < g.enemy_count; i++) {
            EnemyType type = g.enemies[i].type;
            if (g.enemies[i].is_boss) bosses++;
            if (type < ENEMY_GOBLIN_SCOUT ||
                type > ENEMY_MOUNTAIN_GOBLIN_KING) invalid = 1;
        }
        ASSERT("mountains use only mountain enemy roster", !invalid);
        if (level < MOUNTAIN_DEPTH)
            ASSERT("mountain stages 1-7 have no boss", bosses == 0);
        else
            ASSERT("mountain stage 8 has Goblin King",
                bosses == 1 &&
                g.enemies[0].type == ENEMY_MOUNTAIN_GOBLIN_KING);
    }

    g.level = 1;
    map_generate_mountains(&g.map, 1);
    enemies_spawn(&g);
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    Action exit = outdoor_exit_action(&g.map);
    action_resolve_player(&g, exit);
    ASSERT("mountain stage advances without full clear", g.level == 2);
    ASSERT("mountain progress uses independent cache",
        g.mountain_cache[0].valid && !g.level_cache[0].valid &&
        !g.forest_cache[0].valid);

    g.level = MOUNTAIN_DEPTH;
    map_generate_mountains(&g.map, g.level);
    enemies_spawn(&g);
    g.player.x = g.map.stairs_down_x;
    g.player.y = g.map.stairs_down_y;
    exit = outdoor_exit_action(&g.map);
    action_resolve_player(&g, exit);
    ASSERT("Goblin King blocks final mountain exit",
        g.location == LOCATION_MOUNTAINS);
    for (int i = 0; i < g.enemy_count; i++)
        if (g.enemies[i].type == ENEMY_MOUNTAIN_GOBLIN_KING)
            g.enemies[i].active = 0;
    action_resolve_player(&g, exit);
    ASSERT("defeating Goblin King reaches Town 4",
        g.location == LOCATION_TOWN4);
    ASSERT("mountain completion arrives at Town 4 south road",
        g.player.x == 20 && g.player.y == TOWN_H - 2);
}

void test_return_to_town(void) {
    printf("Return to town tests:\n");

    GameState g;
    game_init(&g);

    // Set up dungeon state
    g.location = LOCATION_DUNGEON;
    g.level = 3;
    map_generate(&g.map, g.level);
    enemies_spawn(&g);
    g.level_cleared = 1;
    g.player.x = g.map.stairs_up_x;
    g.player.y = g.map.stairs_up_y;
    g.player.poison_turns = 3;

    int enemies_before = g.enemy_count;
    ASSERT("enemies exist before return", enemies_before > 0);

    game_return_to_town(&g);

    ASSERT("location is town after return",
        g.location == LOCATION_TOWN);
    ASSERT("enemy count is zero after return",
        g.enemy_count == 0);
    ASSERT("player spawn is walkable",
        map_is_walkable(&g.map, g.player.x, g.player.y));
    ASSERT("player not on wall tile",
        g.map.tiles[g.player.y][g.player.x] != TILE_WALL);
    ASSERT("level 3 cached after return",
        g.level_cache[2].valid == 1);
    ASSERT("level 3 cleared state cached",
        g.level_cache[2].level_cleared == 1);
    ASSERT("floor items cleared",
        g.floor_item_count == 0);
    ASSERT("returning to town clears poison",
        g.player.poison_turns == 0);

    g.player.hp = 2;
    g.player.poison_turns = 1;
    action_resolve_player(&g, (Action){ACTION_NONE, 0, 0});
    ASSERT("town actions clear stale poison without dealing damage",
        g.player.poison_turns == 0 && g.player.hp == 2);
}
